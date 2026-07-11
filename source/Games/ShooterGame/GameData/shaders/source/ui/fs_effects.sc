$input v_texcoord0

#include <../bgfx_shader.sh>

SAMPLER2D(u_Texture, 0);

uniform vec4 u_Color;         // base tint: rgb = color, a = opacity

uniform vec4 u_TextureSize;   // xy = texture width, height in pixels (set from script)

uniform vec4 u_ShadowColor;   // rgb = shadow color, a = shadow opacity
uniform vec4 u_ShadowParams;  // x,y = offset in texels, z = spread (shadow radius/thickness) in texels, w = enable (0/1)
uniform vec4 u_ShadowParams2; // x = smoothness (blur radius) in texels, y,z,w = reserved

uniform vec4 u_OutlineColor;  // rgb = outline color, a = outline opacity
uniform vec4 u_OutlineParams; // x = width in texels, y = enable (0/1)

uniform vec4 u_GlowColor;     // rgb = glow color, a = glow opacity
uniform vec4 u_GlowParams;    // x = radius in texels, y = intensity, z = enable (0/1)

#define PI2 6.28318530718
#define OUTLINE_SAMPLES 8
#define GLOW_RINGS 2
#define GLOW_RING_SAMPLES 8

// Shadow quality knobs. Total texture fetches for the shadow are roughly
// (SHADOW_BLUR_RINGS * SHADOW_BLUR_SAMPLES + 1) * (SHADOW_SPREAD_SAMPLES + 1)
// when spread > 0, so lower these first if the effect gets too expensive
// (e.g. many shadowed sprites on screen, or low-end/mobile targets).
#define SHADOW_SPREAD_SAMPLES 8
#define SHADOW_BLUR_RINGS 3
#define SHADOW_BLUR_SAMPLES 8

// Premultiplied "over" compositing: src drawn on top of dst
vec4 over(vec4 src, vec4 dst)
{
    return vec4(src.rgb + dst.rgb * (1.0 - src.a), src.a + dst.a * (1.0 - src.a));
}

// Every effect sample goes through here instead of calling texture2D
// directly. Outside [0,1] returns fully transparent rather than whatever the
// sampler's wrap mode would give (clamp = repeated edge pixels, repeat =
// actually tiling) — that's what lets the C++ side hand this shader padded
// geometry/UVs (see UiRenderer::DrawTexturedRectRegion/DrawTexturedRect9Slice)
// for ANY texture and have shadow/outline/glow correctly fade to nothing in
// the padding, without the source texture needing any blank border baked in.
// The font atlas happens to already have real blank texels there (so this
// is a no-op for text), but a plain image, video frame, or 9-slice edge does
// not, and would otherwise smear/repeat its own edge pixels into the halo.
vec4 sampleTexClamped(vec2 uv)
{
    if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0)
        return vec4_splat(0.0);
    return texture2D(u_Texture, uv);
}

// Max alpha found in a ring around uv - dilates the shape, used for the outline
float ringMaxAlpha(vec2 uv, float radiusTexels)
{
    vec2 texelSize = 1.0 / u_TextureSize.xy;
    float result = 0.0;
    for (int i = 0; i < OUTLINE_SAMPLES; i++)
    {
        float angle = PI2 * float(i) / float(OUTLINE_SAMPLES);
        vec2 offset = vec2(cos(angle), sin(angle)) * texelSize * radiusTexels;
        result = max(result, sampleTexClamped(uv + offset).a);
    }
    return result;
}

// Softly averaged alpha from concentric rings - used for the glow halo
float glowAlpha(vec2 uv, float radiusTexels)
{
    vec2 texelSize = 1.0 / u_TextureSize.xy;
    float total = 0.0;
    float weightSum = 0.0;
    for (int r = 1; r <= GLOW_RINGS; r++)
    {
        float ringRadius = radiusTexels * (float(r) / float(GLOW_RINGS));
        float weight = 1.0 - (float(r) / float(GLOW_RINGS)) * 0.6; // inner rings count more
        for (int i = 0; i < GLOW_RING_SAMPLES; i++)
        {
            float angle = PI2 * float(i) / float(GLOW_RING_SAMPLES);
            vec2 offset = vec2(cos(angle), sin(angle)) * texelSize * ringRadius;
            total += sampleTexClamped(uv + offset).a * weight;
            weightSum += weight;
        }
    }
    return weightSum > 0.0 ? total / weightSum : 0.0;
}

// Dilated alpha at a single point: max alpha within `spreadTexels` of p,
// approximated with one ring sample (same trick as the outline above).
// This is what actually grows the shadow's THICKNESS, independent of blur.
float shadowSpreadAlpha(vec2 p, float spreadTexels)
{
    vec2 texelSize = 1.0 / u_TextureSize.xy;
    float result = sampleTexClamped(p).a;
    if (spreadTexels > 0.0)
    {
        for (int i = 0; i < SHADOW_SPREAD_SAMPLES; i++)
        {
            float angle = PI2 * float(i) / float(SHADOW_SPREAD_SAMPLES);
            vec2 offset = vec2(cos(angle), sin(angle)) * texelSize * spreadTexels;
            result = max(result, sampleTexClamped(p + offset).a);
        }
    }
    return result;
}

// Drop shadow alpha, Photoshop-style: the shape is first DILATED by
// `spreadTexels` (grows the solid core, hard edge), then that dilated
// shape is BLURRED by `smoothnessTexels` (softens the edge into a
// gradient). Two independent knobs instead of one shared blur radius.
float shadowAlpha(vec2 uv, vec2 offsetTexels, float spreadTexels, float smoothnessTexels)
{
    vec2 texelSize = 1.0 / u_TextureSize.xy;
    vec2 center = uv - offsetTexels * texelSize;

    if (smoothnessTexels <= 0.0)
        return shadowSpreadAlpha(center, spreadTexels);

    float total = shadowSpreadAlpha(center, spreadTexels);
    float weightSum = 1.0;

    for (int r = 1; r <= SHADOW_BLUR_RINGS; r++)
    {
        float ringRadius = smoothnessTexels * (float(r) / float(SHADOW_BLUR_RINGS));
        float weight = 1.0 - (float(r) / float(SHADOW_BLUR_RINGS)) * 0.6; // inner rings count more
        for (int i = 0; i < SHADOW_BLUR_SAMPLES; i++)
        {
            float angle = PI2 * float(i) / float(SHADOW_BLUR_SAMPLES);
            vec2 offset = vec2(cos(angle), sin(angle)) * texelSize * ringRadius;
            total += shadowSpreadAlpha(center + offset, spreadTexels) * weight;
            weightSum += weight;
        }
    }
    return total / weightSum;
}

void main()
{
    vec4 texColor = sampleTexClamped(v_texcoord0);

    // ---- base sprite (original behaviour) ----
    vec4 baseColor = texColor * u_Color;
    vec4 baseLayer = vec4(baseColor.rgb * baseColor.a, baseColor.a);

    // ---- outline: ring dilation minus the shape itself ----
    vec4 outlineLayer = vec4_splat(0.0);
    if (u_OutlineParams.y > 0.5 && u_OutlineParams.x > 0.0)
    {
        float dilated = ringMaxAlpha(v_texcoord0, u_OutlineParams.x);
        float mask = clamp(dilated - texColor.a, 0.0, 1.0) * u_OutlineColor.a;
        outlineLayer = vec4(u_OutlineColor.rgb * mask, mask);
    }

    // ---- glow: soft halo extending beyond the shape ----
    vec4 glowLayer = vec4_splat(0.0);
    if (u_GlowParams.z > 0.5 && u_GlowParams.x > 0.0)
    {
        float mask = clamp(glowAlpha(v_texcoord0, u_GlowParams.x) * u_GlowParams.y, 0.0, 1.0) * u_GlowColor.a;
        glowLayer = vec4(u_GlowColor.rgb * mask, mask);
    }

    // ---- drop shadow: offset, thickened by spread, softened by smoothness ----
    vec4 shadowLayer = vec4_splat(0.0);
    if (u_ShadowParams.w > 0.5)
    {
        float mask = shadowAlpha(v_texcoord0, u_ShadowParams.xy, u_ShadowParams.z, u_ShadowParams2.x) * u_ShadowColor.a;
        shadowLayer = vec4(u_ShadowColor.rgb * mask, mask);
    }

    // Compose back-to-front: shadow, glow, outline, base sprite on top
    vec4 result = shadowLayer;
    result = over(glowLayer, result);
    result = over(outlineLayer, result);
    result = over(baseLayer, result);

    gl_FragColor = result;
}
