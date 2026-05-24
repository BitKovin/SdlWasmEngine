// vs_shadowvoledge.sc — shadow volume edge shader with full per-vertex skinning
//
// VERTEX RECORD LAYOUT:
//   a_position    — bind-pose position for THIS endpoint (p0 or p1)
//   a_normal      — p0 bind-pose position   (SAME for all 4 records)
//   a_tangent     — p1 bind-pose position   (SAME for all 4 records)
//   a_bitangent   — adj0 bind-pose position (SAME for all 4 records)
//   a_indices     — THIS endpoint's own bone indices (for correct output)
//   a_weight      — THIS endpoint's own bone weights (for correct output)
//   a_color0      — [dom_bone_p0, dom_bone_p1, dom_bone_adj0, extrude_flag]
//   a_texcoord0   — [adj1.x, adj1.y]        (SAME for all 4 records)
//   a_texcoord1   — [adj1.z, dom_bone_adj1]  (SAME for all 4 records)
//
// Records per edge:
//   Rec 0: a_position=p0, a_indices=p0_bones, extrude=0   (base    p0)
//   Rec 1: a_position=p1, a_indices=p1_bones, extrude=0   (base    p1)
//   Rec 2: a_position=p0, a_indices=p0_bones, extrude=1   (extruded p0)
//   Rec 3: a_position=p1, a_indices=p1_bones, extrude=1   (extruded p1)
//   Indices: [0,2,1,  1,2,3]
//
// SILHOUETTE TEST (identical for all 4 records):
//   - Skin p0   with dom_bone_p0   (a_color0.r)  → world_p0
//   - Skin p1   with dom_bone_p1   (a_color0.g)  → world_p1
//   - Skin adj0 with dom_bone_adj0 (a_color0.b)  → world_adj0
//   - adj1 = vec3(a_texcoord0.xy, a_texcoord1.x)
//   - Skin adj1 with dom_bone_adj1 (a_texcoord1.y) → world_adj1
//   - lit0 = isFaceLit(world_p0, world_p1, world_adj0)
//   - lit1 = isFaceLit(world_p1, world_p0, world_adj1)
//   - isSilhouette = lit0 && !lit1
//   All 4 records store the same data → same result → consistent collapse. ✓
//
// OUTPUT POSITION (per-vertex correct):
//   - Skin a_position with a_indices/a_weight → thisPos  (correct for p0 or p1)
//   - base vertex:      gl_Position = viewProj * vec4(thisPos, 1.0)
//   - extruded, silhouette: gl_Position = viewProj * vec4(normalize(thisPos-lightPos), 0)
//   - extruded, not silh.: gl_Position = viewProj * vec4(thisPos, 1.0)  → coincides
//                           with base vertex → zero-area triangle. ✓

$input a_position, a_normal, a_tangent, a_bitangent, a_indices, a_weight, a_color0, a_texcoord0, a_texcoord1

#include <../bgfx_shader.sh>

uniform mat4 projection;
uniform mat4 view;
uniform mat4 world;
uniform vec4 u_shadowLightPos;

#define MAX_BONES 128
uniform mat4 finalBonesMatrices[MAX_BONES];

mat4 GetBoneTransform(vec4 indices, vec4 weights)
{
    float sum = weights.x + weights.y + weights.z + weights.w;
    if (sum < 0.001)
        return mat4(1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1);
    float r = 1.0 / sum;
    return finalBonesMatrices[int(indices.x)] * (weights.x * r)
         + finalBonesMatrices[int(indices.y)] * (weights.y * r)
         + finalBonesMatrices[int(indices.z)] * (weights.z * r)
         + finalBonesMatrices[int(indices.w)] * (weights.w * r);
}

vec3 skinSingle(vec3 bindPos, int boneIdx)
{
    return mul(mul(world, finalBonesMatrices[boneIdx]), vec4(bindPos, 1.0)).xyz;
}

// Mirrors isFaceLit from original: OR of dot(n, lightPos-px) > 0 for all 3 verts.
bool isFaceLit(vec3 p0, vec3 p1, vec3 p2, vec3 lightPos)
{
    vec3 n = cross(p1 - p0, p2 - p0);
    return (dot(n, lightPos - p0) > 0.0)
        || (dot(n, lightPos - p1) > 0.0)
        || (dot(n, lightPos - p2) > 0.0);
}

void main()
{
    mat4 viewProj = mul(projection, view);
    vec3 lightPos = u_shadowLightPos.xyz;

    // ── Silhouette test — identical for all 4 records ─────────────────────────
    vec3 world_p0   = skinSingle(a_normal,    int(a_color0.r));
    vec3 world_p1   = skinSingle(a_tangent,   int(a_color0.g));
    vec3 world_adj0 = skinSingle(a_bitangent, int(a_color0.b));
    vec3 adj1_bind  = vec3(a_texcoord0.x, a_texcoord0.y, a_texcoord1.x);
    vec3 world_adj1 = skinSingle(adj1_bind,   int(a_texcoord1.y));

    // lit0: face (p0,p1,adj0) is facing the light
    // lit1: face (p1,p0,adj1) is facing the light
    // Silhouette: one face lit, the other not — lit0 && !lit1 means this
    // entry's direction (v0→v1) is the outward silhouette direction.
    bool lit0 = isFaceLit(world_p0, world_p1, world_adj0, lightPos);
    bool lit1 = isFaceLit(world_p1, world_p0, world_adj1, lightPos);
    bool isSilhouette = lit0 && !lit1;

    // ── Output position — per-vertex correct skinning ─────────────────────────
    vec3 thisPos = mul(mul(world, GetBoneTransform(a_indices, a_weight)),
                       vec4(a_position, 1.0)).xyz;

    bool isExtruded = (a_color0.a > 0.5);

    if (!isExtruded)
    {
        // Base vertex: always output directly, no silhouette test.
        gl_Position = mul(viewProj, vec4(thisPos, 1.0));
        
    }
    else
    {

    

    if (isSilhouette)
    {
        vec3 L = normalize(thisPos - lightPos);
        gl_Position = mul(viewProj, vec4(L, 0.0));
    }
    else
    {
        // Stay at base position → coincides with base vertex → zero-area. ✓
        gl_Position = mul(viewProj, vec4(thisPos, 1.0));
    }
    }
}
