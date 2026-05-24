// shadow_volume.cpp

#include "shadow_volume.hpp"
#include "skinned_model.hpp"
#include "model.hpp"

#include <unordered_map>
#include <cstring>
#include <cassert>
#include <cmath>

namespace roj
{

// ─────────────────────────────────────────────────────────────────────────────
// VERTEX LAYOUTS
//
// The previous design stored the precomputed bind-pose face normal and rotated
// it with v0's dominant bone for the lit test. This broke at joints: v1/v2
// positions are moved by their own bones, so the actual deformed face normal
// is completely different from the bind-pose normal rotated by just one bone.
//
// New design: store ALL relevant positions in EVERY record. The VS skins each
// position with its own dominant bone and computes the face normal from the
// actual deformed positions. The dominant-bone approximation per vertex is far
// better than a single shared bone for the whole triangle.
//
// ── CAP vertex ───────────────────────────────────────────────────────────────
//   a_position    (3f) — bind-pose position for THIS vertex output (p0/p1/p2)
//   a_normal      (3f) — p0 bind-pose position   (SAME for all 6 records)
//   a_tangent     (3f) — p1 bind-pose position   (SAME for all 6 records)
//   a_bitangent   (3f) — p2 bind-pose position   (SAME for all 6 records)
//   a_indices     (4f) — THIS vertex's own bone indices (for output)
//   a_weight      (4f) — THIS vertex's own bone weights (for output)
//   a_color0      (4f) — [dom_bone_p0, dom_bone_p1, dom_bone_p2, extrude_flag]
//   a_texcoord0   (2f) — unused
//   a_texcoord1   (2f) — unused
//   a_texcoord2   (2f) — unused
//
//   Per triangle, 6 records:
//     Rec 0: a_position=p0, own_bones=v0_bones, extrude=0    (front p0)
//     Rec 1: a_position=p0, own_bones=v0_bones, extrude=1    (back  p0)
//     Rec 2: a_position=p1, own_bones=v1_bones, extrude=0    (front p1)
//     Rec 3: a_position=p1, own_bones=v1_bones, extrude=1    (back  p1)
//     Rec 4: a_position=p2, own_bones=v2_bones, extrude=0    (front p2)
//     Rec 5: a_position=p2, own_bones=v2_bones, extrude=1    (back  p2)
//
//   Index buffer: front=[0,2,4], back=[5,3,1]
//
// ── EDGE vertex ───────────────────────────────────────────────────────────────
//   a_position    (3f) — bind-pose position for THIS endpoint (p0 or p1)
//   a_normal      (3f) — p0 bind-pose position   (SAME for all 4 records)
//   a_tangent     (3f) — p1 bind-pose position   (SAME for all 4 records)
//   a_bitangent   (3f) — adj0 bind-pose position (SAME for all 4 records)
//   a_indices     (4f) — THIS endpoint's own bone indices (for output)
//   a_weight      (4f) — THIS endpoint's own bone weights (for output)
//   a_color0      (4f) — [dom_bone_p0, dom_bone_p1, dom_bone_adj0, extrude_flag]
//   a_texcoord0   (2f) — adj1.x, adj1.y          (SAME for all 4 records)
//   a_texcoord1   (2f) — adj1.z, dom_bone_adj1   (SAME for all 4 records)
//   a_texcoord2   (2f) — unused
//
//   Per edge entry, 4 records:
//     Rec 0: a_position=p0, own_bones=p0_bones, extrude=0    (base    p0)
//     Rec 1: a_position=p1, own_bones=p1_bones, extrude=0    (base    p1)
//     Rec 2: a_position=p0, own_bones=p0_bones, extrude=1    (extruded p0)
//     Rec 3: a_position=p1, own_bones=p1_bones, extrude=1    (extruded p1)
//
//   Index buffer: [0,2,1,  1,2,3]
//
// WHY THIS WORKS AT MULTI-BONE JOINTS
//   Every record in a primitive stores all positions of that primitive.
//   The VS skins each position with its own dominant bone → approximates the
//   actual deformed position. Face normal computed from deformed positions →
//   correct lit/silhouette classification even at highly deformed joints.
//   All records in a primitive store the SAME set of positions and dom bones
//   → SAME face normal computation → SAME lit/silhouette result → consistent
//   collapse/render decision for the whole primitive → no tearing. ✓
// ─────────────────────────────────────────────────────────────────────────────

namespace {

// ── Position deduplication ────────────────────────────────────────────────────
inline float roundP(float v) { return std::round(v * 1e7f) / 1e7f; }

struct Vec3Key {
    float x, y, z;
    bool operator==(const Vec3Key& o) const { return x==o.x && y==o.y && z==o.z; }
};
struct Vec3KeyHash {
    size_t operator()(const Vec3Key& k) const {
        size_t h = 2166136261u;
        auto mix = [&](float f){ uint32_t u; memcpy(&u,&f,4); h=(h^u)*16777619u; };
        mix(k.x); mix(k.y); mix(k.z); return h;
    }
};

struct CleanVert { glm::vec3 pos; const VertexData* src; };

static void deduplicateVerts(
    const std::vector<VertexData>& srcVerts,
    const std::vector<uint32_t>&   srcIndices,
    std::vector<CleanVert>&        cv,
    std::vector<uint32_t>&         tri)
{
    tri.resize(srcIndices.size());
    std::unordered_map<Vec3Key, uint32_t, Vec3KeyHash> seen;
    seen.reserve(srcIndices.size());
    for (size_t i = 0; i < srcIndices.size(); ++i) {
        const VertexData& sv = srcVerts[srcIndices[i]];
        Vec3Key key{ roundP(sv.Position.x), roundP(sv.Position.y), roundP(sv.Position.z) };
        auto [it, ins] = seen.emplace(key, (uint32_t)cv.size());
        if (ins) cv.push_back({ sv.Position, &sv });
        tri[i] = it->second;
    }
}

// Returns dominant bone index (highest weight) for a vertex
static float domBone(const VertexData& v) {
    int best = 0; float bestW = v.BlendWeights[0];
    for (int k = 1; k < 4; ++k) {
        if (v.BlendWeights[k] > bestW) { bestW = v.BlendWeights[k]; best = k; }
    }
    return (float)(int)v.BlendIndices[best];
}

// ── Edge adjacency ────────────────────────────────────────────────────────────
struct EdgeEntry { uint32_t v0, v1, adj0, adj1; };

static std::vector<EdgeEntry> buildEdges(const std::vector<uint32_t>& tri)
{
    struct HalfEdge { uint32_t i0,i1; std::vector<uint32_t> opp0,opp1; };
    std::unordered_map<uint64_t,HalfEdge> em;
    em.reserve(tri.size());
    for (size_t t = 0; t < tri.size()/3; ++t) {
        auto addE = [&](uint32_t a, uint32_t b, uint32_t opp) {
            uint32_t lo=a,hi=b; bool c=(lo<hi); if(!c)std::swap(lo,hi);
            uint64_t k=((uint64_t)lo<<32)|hi;
            auto [it,ins]=em.emplace(k,HalfEdge{lo,hi,{},{}});
            (c?it->second.opp0:it->second.opp1).push_back(opp);
        };
        addE(tri[t*3+0],tri[t*3+1],tri[t*3+2]);
        addE(tri[t*3+1],tri[t*3+2],tri[t*3+0]);
        addE(tri[t*3+2],tri[t*3+0],tri[t*3+1]);
    }
    std::vector<EdgeEntry> out; out.reserve(em.size()*2);
    for (auto&[key,e]:em) {
        if (e.opp0.size()==1&&e.opp1.size()==1) {
            out.push_back({e.i0,e.i1,e.opp0[0],e.opp1[0]});
            out.push_back({e.i1,e.i0,e.opp1[0],e.opp0[0]});
        } else {
            for(uint32_t a:e.opp0) out.push_back({e.i0,e.i1,a,a});
            for(uint32_t a:e.opp1) out.push_back({e.i1,e.i0,a,a});
        }
    }
    return out;
}

static void copyBones(VertexData* dst, const VertexData& src) {
    for (int k = 0; k < 4; ++k) dst->BlendIndices[k] = src.BlendIndices[k];
    dst->BlendWeights = src.BlendWeights;
}

// ── Cap vertex write ──────────────────────────────────────────────────────────
static void writeCapVert(
    const glm::vec3& thisPos,           // a_position: position for this vertex's output
    const glm::vec3& p0,                // a_normal:   same for all 6 records
    const glm::vec3& p1,                // a_tangent:  same for all 6 records
    const glm::vec3& p2,                // a_bitangent:same for all 6 records
    float db0, float db1, float db2,    // dominant bones of p0,p1,p2 — same for all 6
    const VertexData& ownBones,         // per-record: own bone data for correct output
    float extrude,                      // 0=front, 1=back
    VertexData* dst)
{
    std::memset(dst, 0, sizeof(VertexData));
    dst->Position  = thisPos;
    dst->Normal    = p0;    
    dst->Tangent   = p1;
    dst->BiTangent = p2;
    // Color: [dom_bone_p0, dom_bone_p1, dom_bone_p2, extrude_flag]
    dst->Color = glm::vec4(db0, db1, db2, extrude);
    copyBones(dst, ownBones);
}

// ── Edge vertex write ─────────────────────────────────────────────────────────
static void writeEdgeVert(
    const glm::vec3& thisPos,           // a_position: this endpoint for output
    const glm::vec3& p0,                // a_normal:   same for all 4 records
    const glm::vec3& p1,                // a_tangent:  same for all 4 records
    const glm::vec3& adj0,              // a_bitangent:same for all 4 records
    const glm::vec3& adj1,              // texcoord0.xy + texcoord1.x: same for all 4
    float db0, float db1,               // dom bones of p0, p1
    float dbAdj0, float dbAdj1,         // dom bones of adj0, adj1
    const VertexData& ownBones,         // per-record: own bone data for correct output
    float extrude,                      // 0=base, 1=extruded
    VertexData* dst)
{
    std::memset(dst, 0, sizeof(VertexData));
    dst->Position  = thisPos;
    dst->Normal    = p0;
    dst->Tangent   = p1;
    dst->BiTangent = adj0;
    // Color: [dom_bone_p0, dom_bone_p1, dom_bone_adj0, extrude_flag]
    dst->Color = glm::vec4(db0, db1, dbAdj0, extrude);
    // adj1.xyz split across texcoord0 and texcoord1
    dst->TextureCoordinate  = glm::vec2(adj1.x, adj1.y);       // a_texcoord0
    dst->TextureCoordinate2 = glm::vec2(adj1.z, dbAdj1);       // a_texcoord1
    copyBones(dst, ownBones);
}

} // anonymous namespace

// ─────────────────────────────────────────────────────────────────────────────
ShadowVolumePrecomp BuildShadowVolumePrecomp(const SkinnedMesh& mesh)
{
    ShadowVolumePrecomp result;
    if (mesh.indices.size() < 3) return result;

    const bgfx::VertexLayout layout = VertexData::Declaration();
    const uint32_t stride = layout.getStride();

    // ── 1. Positional deduplication ───────────────────────────────────────────
    std::vector<CleanVert> cv;
    std::vector<uint32_t>  tri;
    deduplicateVerts(mesh.vertices, mesh.indices, cv, tri);

    // Remove degenerate triangles
    {
        std::vector<uint32_t> f; f.reserve(tri.size());
        for (size_t t = 0; t < tri.size()/3; ++t) {
            const glm::vec3& p0=cv[tri[t*3+0]].pos, &p1=cv[tri[t*3+1]].pos, &p2=cv[tri[t*3+2]].pos;
            glm::vec3 c=glm::cross(p1-p0,p2-p0);
            if (std::sqrt(glm::dot(c,c))*0.5f >= 1e-5f)
                { f.push_back(tri[t*3+0]); f.push_back(tri[t*3+1]); f.push_back(tri[t*3+2]); }
        }
        tri=std::move(f);
    }
    if (tri.size()<3) return result;

    const uint32_t triCount=(uint32_t)tri.size()/3;

    // ── 2. Edge adjacency ─────────────────────────────────────────────────────
    std::vector<EdgeEntry> edges=buildEdges(tri);

    // ── 3. Cap VB + IB ────────────────────────────────────────────────────────
    {
        const uint32_t nv=triCount*6, ni=triCount*6;
        std::vector<uint8_t>  buf(nv*stride,0);
        std::vector<uint32_t> idx; idx.reserve(ni);

        auto vAt=[&](uint32_t i)->VertexData*{
            return reinterpret_cast<VertexData*>(buf.data()+i*stride); };

        for (uint32_t t=0;t<triCount;++t) {
            const uint32_t i0=tri[t*3+0], i1=tri[t*3+1], i2=tri[t*3+2];
            const glm::vec3& p0=cv[i0].pos, &p1=cv[i1].pos, &p2=cv[i2].pos;
            float db0=domBone(*cv[i0].src), db1=domBone(*cv[i1].src), db2=domBone(*cv[i2].src);
            const uint32_t base=t*6;

            // 6 records: pairs for p0/p1/p2, each pair has front(extrude=0) and back(extrude=1)
            // All 6 store the SAME p0,p1,p2 and dom bones → consistent lit test
            // Each uses ITS OWN bone data for the output position
            writeCapVert(p0,p0,p1,p2,db0,db1,db2,*cv[i0].src,0.f,vAt(base+0)); // front p0
            writeCapVert(p0,p0,p1,p2,db0,db1,db2,*cv[i0].src,1.f,vAt(base+1)); // back  p0
            writeCapVert(p1,p0,p1,p2,db0,db1,db2,*cv[i1].src,0.f,vAt(base+2)); // front p1
            writeCapVert(p1,p0,p1,p2,db0,db1,db2,*cv[i1].src,1.f,vAt(base+3)); // back  p1
            writeCapVert(p2,p0,p1,p2,db0,db1,db2,*cv[i2].src,0.f,vAt(base+4)); // front p2
            writeCapVert(p2,p0,p1,p2,db0,db1,db2,*cv[i2].src,1.f,vAt(base+5)); // back  p2

            // Front: original winding p0,p1,p2
            idx.push_back(base+0); idx.push_back(base+2); idx.push_back(base+4);
            // Back:  reversed winding p2,p1,p0
            idx.push_back(base+5); idx.push_back(base+3); idx.push_back(base+1);
        }
        assert(idx.size()==ni);
        result.capVbh       =bgfx::createVertexBuffer(bgfx::copy(buf.data(),nv*stride),layout);
        result.capIbh       =bgfx::createIndexBuffer(bgfx::copy(idx.data(),ni*4),BGFX_BUFFER_INDEX32);
        result.capIndexCount=ni;
    }

    // ── 4. Edge VB + IB ───────────────────────────────────────────────────────
    {
        const uint32_t ne=(uint32_t)edges.size(), nv=ne*4, ni=ne*6;
        std::vector<uint8_t>  buf(nv*stride,0);
        std::vector<uint32_t> idx; idx.reserve(ni);

        auto vAt=[&](uint32_t i)->VertexData*{
            return reinterpret_cast<VertexData*>(buf.data()+i*stride); };

        for (uint32_t e=0;e<ne;++e) {
            const EdgeEntry& ee=edges[e];
            const uint32_t base=e*4;

            const glm::vec3& p0  =cv[ee.v0].pos,  &p1  =cv[ee.v1].pos;
            const glm::vec3& adj0=cv[ee.adj0].pos, &adj1=cv[ee.adj1].pos;

            float db0   =domBone(*cv[ee.v0].src),  db1   =domBone(*cv[ee.v1].src);
            float dbAdj0=domBone(*cv[ee.adj0].src), dbAdj1=domBone(*cv[ee.adj1].src);

            // All 4 records carry identical (p0,p1,adj0,adj1,dom bones) → consistent test
            // Each uses ITS OWN endpoint bone data for output
            writeEdgeVert(p0,p0,p1,adj0,adj1,db0,db1,dbAdj0,dbAdj1,*cv[ee.v0].src,0.f,vAt(base+0)); // base    p0
            writeEdgeVert(p1,p0,p1,adj0,adj1,db0,db1,dbAdj0,dbAdj1,*cv[ee.v1].src,0.f,vAt(base+1)); // base    p1
            writeEdgeVert(p0,p0,p1,adj0,adj1,db0,db1,dbAdj0,dbAdj1,*cv[ee.v0].src,1.f,vAt(base+2)); // extruded p0
            writeEdgeVert(p1,p0,p1,adj0,adj1,db0,db1,dbAdj0,dbAdj1,*cv[ee.v1].src,1.f,vAt(base+3)); // extruded p1

            idx.push_back(base+0); idx.push_back(base+2); idx.push_back(base+1);
            idx.push_back(base+1); idx.push_back(base+2); idx.push_back(base+3);
        }
        assert(idx.size()==ni);
        result.edgeVbh        =bgfx::createVertexBuffer(bgfx::copy(buf.data(),nv*stride),layout);
        result.edgeIbh        =bgfx::createIndexBuffer(bgfx::copy(idx.data(),ni*4),BGFX_BUFFER_INDEX32);
        result.edgeIndexCount =ni;
    }

    return result;
}

} // namespace roj
