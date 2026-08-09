$input v_texcoord0, v_color0, v_normal, v_world, v_texcoord1, v_tangent, v_bitangent

#include <bgfx_shader.sh>

SAMPLER2D(u_texture, 0);

// IMPORTANT: this MUST be bound with point/nearest filtering and clamp addressing
// (BGFX_SAMPLER_POINT | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP) on the C++ side.
// A linearly-filtered depth sample taken across a silhouette edge (e.g. a bar in
// front of a wall) blends two unrelated depths into a fake "in-between" surface
// point. That fake point can land inside DECAL_RANGE purely by chance, which is
// what paints the stray lines behind/around your decals.
SAMPLER2D(depthTexture, 1);

uniform vec4 modelColor;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 viewProjectionInv;

// ---------------------------------------------------------------------------
// Tunables
// ---------------------------------------------------------------------------

#define DECAL_RANGE 0.1   // max allowed distance (world units, ~2 cm) between the
                            // decal's plane and the real surface, measured along v_normal.

#define DECAL_EDGE_SOFTEN 0.01   // world units. Width of a smooth fade band ending at
                                  // DECAL_RANGE. Turns the old binary discard into a soft
                                  // ramp so the range boundary can't alias into a visible line.

#define DECAL_SILHOUETTE_REJECT 1               // 1 = discard fragments that sit on a depth
                                                 // discontinuity instead of a real continuous surface.
#define DECAL_SILHOUETTE_THRESHOLD (DECAL_RANGE * 6.0) // world units/pixel of allowed surfacePos
                                                         // change before we assume we're straddling
                                                         // two unrelated surfaces, not one continuous one.
                                                         // Raise this if decals flicker away at glancing
                                                         // angles or long view distances.

#define DECAL_DEBUG_RANGE 0 // set to 1: shows planeDist as color instead of the
                             // decal texture. green = within DECAL_RANGE,
                             // red = surface is further away along +v_normal,
                             // blue = further away along -v_normal, brighter = worse.

void main()
{
    // --- 1. Find this fragment's position on screen using the SAME view/projection
    //         the depth prepass was rendered with. ---
    vec4 clipPos = mul(projection, mul(view, vec4(v_world.xyz, 1.0)));
    vec2 ndc     = clipPos.xy / clipPos.w;

    // NDC keeps +Y up, textures are sampled with +V down -> flip Y.
    vec2 screenUv = ndc * vec2(0.5, -0.5) + 0.5;

    // Defensive: reject if the reprojection lands outside the screen (near-plane
    // clipping, degenerate w, etc.) instead of silently sampling a clamped edge texel.
    if (screenUv.x < 0.0 || screenUv.x > 1.0 || screenUv.y < 0.0 || screenUv.y > 1.0)
        discard;

    // --- 2. Sample the depth prepass at that pixel. ---
    float deviceDepth = texture2D(depthTexture, screenUv).r;

    // Cleared/background depth -> nothing rendered there, decal touches nothing.
    if (deviceDepth >= 0.99999)
        discard;

    // --- 3. Unproject that depth sample back into world space. ---
#if BGFX_SHADER_LANGUAGE_GLSL
    float ndcDepth = deviceDepth * 2.0 - 1.0; // GL clip space z: -1..1
#else
    float ndcDepth = deviceDepth;             // D3D/Metal/Vulkan clip space z: 0..1
#endif

    vec4 worldPos4  = mul(viewProjectionInv, vec4(ndc, ndcDepth, 1.0));
    vec3 surfacePos = worldPos4.xyz / worldPos4.w;

#if DECAL_SILHOUETTE_REJECT
    // --- 3b. Bail out on depth-buffer silhouettes. ---
    // A continuous surface moves only a little from one pixel to its neighbour.
    // A big jump means this pixel and its neighbour actually reconstructed two
    // unrelated surfaces (a bar in front of a wall, a corner, a doorframe against
    // the corridor behind it). Trusting that fake point is exactly what produced
    // both bugs you reported: the stray lines, AND the cutoff that looked like it
    // was tracking surface normal (silhouettes are usually also where the normal
    // changes sharply, so normal-correlated noise was really a depth-sampling
    // problem wearing a disguise).
    float posGrad = length(fwidth(surfacePos));
    if (posGrad > DECAL_SILHOUETTE_THRESHOLD)
        discard;
#endif

    // --- 4. Reject if the real surface there is further from the decal's plane
    //         (measured along the decal's own projection axis) than allowed. ---
    float planeDist = dot(surfacePos - v_world.xyz, v_normal);

#if DECAL_DEBUG_RANGE
    float t = clamp(abs(planeDist) / (DECAL_RANGE * 20.0), 0.0, 1.0);
    vec3 dbg = (abs(planeDist) <= DECAL_RANGE)
        ? vec3(0.0, 1.0, 0.0)
        : (planeDist > 0.0 ? vec3(t, 0.0, 0.0) : vec3(0.0, 0.0, t));
    gl_FragColor = vec4(dbg, 1.0);
    return;
#endif

    float absDist = abs(planeDist);
    if (absDist > DECAL_RANGE)
        discard;

    // Soft ramp over the last DECAL_EDGE_SOFTEN units instead of a hard cutoff -
    // this is what removes the single-pixel aliased line right at the range boundary.
    float rangeFade = 1.0 - smoothstep(DECAL_RANGE - DECAL_EDGE_SOFTEN, DECAL_RANGE, absDist);

    // --- 5. Normal decal shading. ---
    vec4 texColor = texture2D(u_texture, v_texcoord0) * v_color0 * modelColor;

    float alpha = texColor.a * rangeFade;
    if (alpha < 0.01)
        discard;

    gl_FragColor = vec4(texColor.rgb, alpha);
}