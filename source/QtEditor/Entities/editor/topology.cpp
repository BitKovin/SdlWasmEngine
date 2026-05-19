// editor/topology.cpp
#include "topology.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace editor {

static bool posNear(glm::vec3 a, glm::vec3 b, float eps = kEpsilon * 10.f) noexcept {
    return glm::length(a - b) < eps;
}

static uint32_t internVertex(BrushTopology& topo, glm::vec3 pos, uint32_t& nextId)
{
    for (uint32_t i = 0; i < static_cast<uint32_t>(topo.vertices.size()); ++i)
        if (posNear(topo.vertices[i].position, pos)) return i;
    TopoVertex v; v.id = nextId++; v.position = pos;
    topo.vertices.push_back(v);
    return static_cast<uint32_t>(topo.vertices.size() - 1);
}

static uint32_t internEdge(BrushTopology& topo,
                             uint32_t vA, uint32_t vB,
                             uint32_t faceIdx, uint32_t& nextId)
{
    if (vA > vB) std::swap(vA, vB);
    for (uint32_t i = 0; i < static_cast<uint32_t>(topo.edges.size()); ++i) {
        auto& e = topo.edges[i];
        uint32_t a = std::min(e.vertA, e.vertB);
        uint32_t b = std::max(e.vertA, e.vertB);
        if (a == vA && b == vB) { e.faceB = faceIdx; return i; }
    }
    TopoEdge e; e.id = nextId++; e.vertA = vA; e.vertB = vB;
    e.faceA = e.faceB = faceIdx;
    topo.edges.push_back(e);
    return static_cast<uint32_t>(topo.edges.size() - 1);
}

static void normalizeFaceLoop(std::vector<glm::vec3>& verts)
{
    if (verts.size() < 3) return;

    std::vector<glm::vec3> out;
    out.reserve(verts.size());
    for (const auto& p : verts) {
        if (out.empty() || !posNear(out.back(), p)) out.push_back(p);
    }
    if (out.size() >= 2 && posNear(out.front(), out.back())) out.pop_back();
    verts.swap(out);
}

// ─────────────────────────────────────────────────────────────────────────────
// buildTopology  — reads from renderData per-face meshes
// ─────────────────────────────────────────────────────────────────────────────

void buildTopology(Brush& brush)
{
    brush.topology.clear();
    if (!brush.renderData.valid) return;

    BrushTopology& topo = brush.topology;
    uint32_t nextVid = 1, nextEid = 1;
    auto nFaces = static_cast<uint32_t>(brush.faces.size());

    topo.faces.resize(nFaces);
    for (uint32_t fi = 0; fi < nFaces; ++fi) topo.faces[fi].faceIndex = fi;

    for (uint32_t fi = 0; fi < nFaces; ++fi) {
        const FaceMesh& fm = brush.renderData.faces[fi];
        if (!fm.valid || fm.vertices.size() < 3) continue;

        FaceTopo& ft = topo.faces[fi];
        ft.vertIndices.clear();
        ft.edgeIndices.clear();

        std::vector<glm::vec3> loop;
        loop.reserve(fm.vertices.size());
        for (const auto& bv : fm.vertices) loop.push_back(bv.position);
        normalizeFaceLoop(loop);
        if (loop.size() < 3) continue;

        for (const auto& pos : loop) {
            uint32_t vi = internVertex(topo, pos, nextVid);
            ft.vertIndices.push_back(vi);
        }
        for (size_t pi = 0; pi < ft.vertIndices.size(); ++pi) {
            uint32_t vA = ft.vertIndices[pi];
            uint32_t vB = ft.vertIndices[(pi + 1) % ft.vertIndices.size()];
            if (vA == vB) continue;
            uint32_t ei = internEdge(topo, vA, vB, fi, nextEid);
            ft.edgeIndices.push_back(ei);
        }
    }

    // Back-fill vertex → face/edge references
    for (uint32_t ei = 0; ei < static_cast<uint32_t>(topo.edges.size()); ++ei) {
        auto addUnique = [](std::vector<uint32_t>& v, uint32_t x) {
            if (std::find(v.begin(), v.end(), x) == v.end()) v.push_back(x);
        };
        addUnique(topo.vertices[topo.edges[ei].vertA].edgeIndices, ei);
        addUnique(topo.vertices[topo.edges[ei].vertB].edgeIndices, ei);
    }
    for (uint32_t fi = 0; fi < nFaces; ++fi) {
        for (uint32_t vi : topo.faces[fi].vertIndices) {
            auto& verts = topo.vertices[vi].faceIndices;
            if (std::find(verts.begin(), verts.end(), fi) == verts.end())
                verts.push_back(fi);
        }
    }

    topo.valid = true;
}

// ─────────────────────────────────────────────────────────────────────────────
// rayPickVertex / rayPickEdge  (unchanged from previous version)
// ─────────────────────────────────────────────────────────────────────────────

static float pointToLineDistance(glm::vec3 point,
                                  glm::vec3 lineOrigin, glm::vec3 lineDir,
                                  float& tOnLine) noexcept
{
    glm::vec3 d = point - lineOrigin;
    tOnLine = glm::dot(d, lineDir);
    return glm::length(point - (lineOrigin + lineDir * tOnLine));
}

uint32_t rayPickVertex(const Ray& ray, const Brush& brush, float radius)
{
    if (!brush.topology.valid) return ~0u;
    float bestDist = std::numeric_limits<float>::max();
    uint32_t bestIdx = ~0u;
    for (uint32_t i = 0; i < static_cast<uint32_t>(brush.topology.vertices.size()); ++i) {
        float t; float perp = pointToLineDistance(
            brush.topology.vertices[i].position, ray.origin, ray.direction, t);
        if (t < 0.f || perp > radius) continue;
        if (t < bestDist) { bestDist = t; bestIdx = i; }
    }
    return bestIdx;
}

static float segSegDist(glm::vec3 p0, glm::vec3 p1, glm::vec3 q0, glm::vec3 q1) noexcept
{
    glm::vec3 d1 = p1-p0, d2 = q1-q0, r = p0-q0;
    float a = glm::dot(d1,d1), e = glm::dot(d2,d2), f = glm::dot(d2,r);
    float s, t;
    constexpr float eps = 1e-6f;
    if (a <= eps && e <= eps) return glm::length(r);
    if (a <= eps) { s = 0.f; t = glm::clamp(f/e,0.f,1.f); }
    else {
        float c = glm::dot(d1,r);
        if (e <= eps) { t = 0.f; s = glm::clamp(-c/a,0.f,1.f); }
        else {
            float b = glm::dot(d1,d2), denom = a*e-b*b;
            s = denom != 0.f ? glm::clamp((b*f-c*e)/denom,0.f,1.f) : 0.f;
            t = (b*s+f)/e;
            if      (t < 0.f) { t = 0.f; s = glm::clamp(-c/a,0.f,1.f); }
            else if (t > 1.f) { t = 1.f; s = glm::clamp((b-c)/a,0.f,1.f); }
        }
    }
    return glm::length(p0+d1*s-(q0+d2*t));
}

uint32_t rayPickEdge(const Ray& ray, const Brush& brush, float radius)
{
    if (!brush.topology.valid) return ~0u;
    constexpr float kFar = 1e6f;
    glm::vec3 rayEnd = ray.origin + ray.direction * kFar;
    float bestDist = std::numeric_limits<float>::max();
    uint32_t bestIdx = ~0u;
    const auto& verts = brush.topology.vertices;
    for (uint32_t i = 0; i < static_cast<uint32_t>(brush.topology.edges.size()); ++i) {
        const auto& e = brush.topology.edges[i];
        float d = segSegDist(ray.origin, rayEnd, verts[e.vertA].position, verts[e.vertB].position);
        if (d < radius && d < bestDist) { bestDist = d; bestIdx = i; }
    }
    return bestIdx;
}

} // namespace editor
