// vs_shadowvolcap.sc — shadow volume cap shader with full per-vertex skinning
//
// VERTEX RECORD LAYOUT:
//   a_position    — bind-pose position for THIS vertex output (p0, p1, or p2)
//   a_normal      — p0 bind-pose position   (SAME for all 6 records of a triangle)
//   a_tangent     — p1 bind-pose position   (SAME for all 6 records)
//   a_bitangent   — p2 bind-pose position   (SAME for all 6 records)
//   a_indices     — THIS vertex's own bone indices (for correct output position)
//   a_weight      — THIS vertex's own bone weights (for correct output position)
//   a_color0      — [dom_bone_p0, dom_bone_p1, dom_bone_p2, extrude_flag]
//                   dom_bone_px = index of highest-weight bone for vertex x
//                   extrude_flag: 0.0 = front-cap, 1.0 = back-cap
//
// LIT TEST (identical for all 6 records):
//   - Skin p0 with finalBonesMatrices[int(a_color0.r)]  → world_p0
//   - Skin p1 with finalBonesMatrices[int(a_color0.g)]  → world_p1
//   - Skin p2 with finalBonesMatrices[int(a_color0.b)]  → world_p2
//   - faceNormal = cross(world_p1 - world_p0, world_p2 - world_p0)
//   - lit = OR of dot(faceNormal, lightPos - world_px) > 0 for x=0,1,2
//   All 6 records store the same p0/p1/p2 and same dom bones → same lit. ✓
//
// OUTPUT POSITION (per-vertex correct):
//   - Skin a_position with THIS vertex's a_indices/a_weight → thisPos
//   - front: gl_Position = viewProj * vec4(thisPos, 1.0)
//   - back:  gl_Position = viewProj * vec4(normalize(thisPos - lightPos), 0.0)
//
// COLLAPSE: if !lit → gl_Position = vec4(0,0,0,0)
//   All 6 records collapse consistently → zero-area triangle → no stencil. ✓

$input a_position, a_normal, a_tangent, a_bitangent, a_indices, a_weight, a_color0

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

// Skin a bind-pose position using a single bone (dominant-bone approximation).
// Accurate for rigid parts; approximate at smooth joints, but consistent across
// all records of the same primitive (same dom bone index stored in all 6). ✓
vec3 skinSingle(vec3 bindPos, int boneIdx)
{
    return mul(mul(world, finalBonesMatrices[boneIdx]), vec4(bindPos, 1.0)).xyz;
}

void main()
{
    mat4 viewProj = mul(projection, view);
    vec3 lightPos = u_shadowLightPos.xyz;

    // ── Lit test — identical for all 6 records ────────────────────────────────
    // Skin each triangle vertex with its own dominant bone.
    // All 6 records store the same p0/p1/p2 in a_normal/a_tangent/a_bitangent
    // and the same dom bone indices in a_color0.rgb → identical result. ✓
    vec3 wp0 = skinSingle(a_normal,    int(a_color0.r));
    vec3 wp1 = skinSingle(a_tangent,   int(a_color0.g));
    vec3 wp2 = skinSingle(a_bitangent, int(a_color0.b));

    vec3 faceN = cross(wp1 - wp0, wp2 - wp0);

    // OR across all three vertices — matches isFaceLit from original exactly.
    // For a very distant directional light these are nearly identical, but
    // keeping the OR ensures correctness for any light distance.
    bool lit = (dot(faceN, lightPos - wp0) > 0.0)
            || (dot(faceN, lightPos - wp1) > 0.0)
            || (dot(faceN, lightPos - wp2) > 0.0);

    if (!lit)
    {
        gl_Position = vec4(0.0, 0.0, 0.0, 0.0);

    }
    else
    {

    // ── Output position — per-vertex correct skinning ─────────────────────────
    // a_position = this vertex's bind-pose position (p0, p1, or p2)
    // a_indices/a_weight = this vertex's own bone data → correct deformed pos
    vec3 thisPos = mul(mul(world, GetBoneTransform(a_indices, a_weight)),
                       vec4(a_position, 1.0)).xyz;

    bool isBackCap = (a_color0.a > 0.5);

    if (isBackCap)
    {
        vec3 L = normalize(thisPos - lightPos);
        gl_Position = mul(viewProj, vec4(L, 0.0));
    }
    else
    {
        gl_Position = mul(viewProj, vec4(thisPos, 1.0));
    }
    }
}
