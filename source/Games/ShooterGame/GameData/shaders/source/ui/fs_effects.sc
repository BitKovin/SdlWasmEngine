$input v_texcoord0

#include <../bgfx_shader.sh>

SAMPLER2D(u_Texture, 0);

uniform vec4 u_Color;         // base tint: rgb = color, a = opacity

// ── Viewport-space effect plumbing ──────────────────────────────────────────
//
// Every size/distance/offset used by shadow/outline/glow below (u_ShadowParams,
// u_ShadowParams2, u_OutlineParams, u_GlowParams) is authored in VIEWPORT
// PIXELS — the same "1 UI unit == 1 output pixel" space every UiElement's own
// size/position already lives in (see the note at the top of UiRenderer.h).
//
// Converting "N viewport pixels" into a UV-space sampling delta requires
// knowing how many actual rasterized fragments this specific draw covers per
// UI unit — which is NOT safe to assume is exactly 1:1 with the declared
// element size (e.g. a logical-vs-physical/backbuffer resolution mismatch,
// common with SDL2 on HiDPI displays, or any other gap between "declared UI
// units" and "actual rendered fragments" that the vertex-side transform
// doesn't care about but a precise per-pixel effect radius does). Rather
// than have the C++ side guess at that ratio per draw call, this shader
// measures it directly with screen-space derivatives of v_texcoord0 — exact
// for this renderer's affine (rotate + translate, never perspective, never
// non-uniform runtime scale) transforms, and correct regardless of the
// actual rasterization resolution, DPI, or any customViewport quirk, since
// it's derived from the real neighboring-fragment UV values rather than an
// assumed pixel-to-UV ratio. See pixelsToUV() below.
uniform vec4 u_ViewportSize;  // xy = active render-target size, viewport pixels
                               // (the real screen, or UiRenderer::customViewportSize
                               // when rendering onto a 3D billboard). Not consumed
                               // below — pixelsToUV() doesn't need it — but set on
                               // every draw (see UiRenderer::SetShaderProjection) so
                               // a future effect can reason in absolute
                               // viewport-pixel terms without new C++ plumbing.
uniform vec4 u_ElementSize;   // xy = this draw's own on-screen size, viewport pixels
                               // (finalizedSize for images/9-slice; the rendered
                               // text block size for DrawText). Same availability
                               // note as u_ViewportSize above.

// The valid sampling sub-rect for THIS draw, in [0,1] texture-UV space:
// xy = min corner, zw = max corner. Defaults to (0,0,1,1) — the whole
// texture — but text sets this per-glyph to that glyph's own (padded) UV
// box, since many glyphs share one atlas texture and a shadow/glow/outline
// on one glyph must not bleed into the pixels of the glyph packed next to
// it. See sampleTexClamped below.
uniform vec4 u_ClampRect;

uniform vec4 u_ShadowColor;   // rgb = shadow color, a = shadow opacity
uniform vec4 u_ShadowParams;  // x,y = offset in viewport pixels, z = spread (shadow radius/thickness) in viewport pixels, w = enable (0/1)
uniform vec4 u_ShadowParams2; // x = smoothness (blur radius) in viewport pixels, y,z,w = reserved

uniform vec4 u_OutlineColor;  // rgb = outline color, a = outline opacity
uniform vec4 u_OutlineParams; // x = width in viewport pixels, y = enable (0/1)

uniform vec4 u_GlowColor;     // rgb = glow color, a = glow opacity
uniform vec4 u_GlowParams;    // x = radius in viewport pixels, y = intensity, z = enable (0/1)

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
// directly. Outside u_ClampRect returns fully transparent rather than
// whatever the sampler's wrap mode would give (clamp = repeated edge pixels,
// repeat = actually tiling) — that's what lets the C++ side hand this shader
// padded geometry/UVs (see UiRenderer::DrawTexturedRectRegion,
// DrawTexturedRect9Slice, DrawText) for ANY texture and have shadow/outline/
// glow correctly fade to nothing in the padding, without the source texture
// needing any blank border baked in.
//
// For a plain image/video/9-slice, u_ClampRect is (0,0,1,1) — the padding
// bleeds past the requested rect but never past the texture's own edges.
// For text, it's set per glyph to that glyph's own padded UV box, because
// many glyphs share one atlas: without this, a large glow/shadow radius
// could sample past a tightly-packed glyph's own cell and pick up pixels
// from whatever's packed next to it in the atlas.
vec4 sampleTexClamped(vec2 uv)
{
    if (uv.x < u_ClampRect.x || uv.x > u_ClampRect.z || uv.y < u_ClampRect.y || uv.y > u_ClampRect.w)
        return vec4_splat(0.0);
    return texture2D(u_Texture, uv);
}

// Converts a 2D offset in VIEWPORT PIXELS into the matching UV-space delta
// for THIS fragment, using screen-space derivatives of v_texcoord0. duvdx/
// duvdy are "how much UV changes per 1 actual rasterized fragment step in
// screen X / screen Y" — measured directly by the GPU from real neighboring
// fragments, so a `deltaPixels` step (assumed to already be a real,
// on-target fragment count — see u_ViewportSize's comment above) maps to
// the UV delta that actually samples that many fragments away, regardless
// of how this draw's declared size relates to its actual rasterized
// resolution. This is also naturally exact per-cell for a 9-sliced element
// (corners/edges/center each have their own texel:pixel ratio) without the
// shader needing to know which cell a given fragment belongs to.
vec2 pixelsToUV(vec2 deltaPixels, vec2 duvdx, vec2 duvdy)
{
    return duvdx * deltaPixels.x + duvdy * deltaPixels.y;
}

// Max alpha found in a ring around uv - dilates the shape, used for the outline
float ringMaxAlpha(vec2 uv, float radiusPixels, vec2 duvdx, vec2 duvdy)
{
    float result = 0.0;
    for (int i = 0; i < OUTLINE_SAMPLES; i++)
    {
        float angle = PI2 * float(i) / float(OUTLINE_SAMPLES);
        vec2 offset = pixelsToUV(vec2(cos(angle), sin(angle)) * radiusPixels, duvdx, duvdy);
        result = max(result, sampleTexClamped(uv + offset).a);
    }
    return result;
}

// Softly averaged alpha from concentric rings - used for the glow halo
float glowAlpha(vec2 uv, float radiusPixels, vec2 duvdx, vec2 duvdy)
{
    float total = 0.0;
    float weightSum = 0.0;
    for (int r = 1; r <= GLOW_RINGS; r++)
    {
        float ringRadius = radiusPixels * (float(r) / float(GLOW_RINGS));
        float weight = 1.0 - (float(r) / float(GLOW_RINGS)) * 0.6; // inner rings count more
        for (int i = 0; i < GLOW_RING_SAMPLES; i++)
        {
            float angle = PI2 * float(i) / float(GLOW_RING_SAMPLES);
            vec2 offset = pixelsToUV(vec2(cos(angle), sin(angle)) * ringRadius, duvdx, duvdy);
            total += sampleTexClamped(uv + offset).a * weight;
            weightSum += weight;
        }
    }
    return weightSum > 0.0 ? total / weightSum : 0.0;
}

// Dilated alpha at a single point: max alpha within `spreadPixels` of p,
// approximated with one ring sample (same trick as the outline above).
// This is what actually grows the shadow's THICKNESS, independent of blur.
float shadowSpreadAlpha(vec2 p, float spreadPixels, vec2 duvdx, vec2 duvdy)
{
    float result = sampleTexClamped(p).a;
    if (spreadPixels > 0.0)
    {
        for (int i = 0; i < SHADOW_SPREAD_SAMPLES; i++)
        {
            float angle = PI2 * float(i) / float(SHADOW_SPREAD_SAMPLES);
            vec2 offset = pixelsToUV(vec2(cos(angle), sin(angle)) * spreadPixels, duvdx, duvdy);
            result = max(result, sampleTexClamped(p + offset).a);
        }
    }
    return result;
}

// Drop shadow alpha, Photoshop-style: the shape is first DILATED by
// `spreadPixels` (grows the solid core, hard edge), then that dilated
// shape is BLURRED by `smoothnessPixels` (softens the edge into a
// gradient). Two independent knobs instead of one shared blur radius.
float shadowAlpha(vec2 uv, vec2 offsetPixels, float spreadPixels, float smoothnessPixels, vec2 duvdx, vec2 duvdy)
{
    vec2 center = uv - pixelsToUV(offsetPixels, duvdx, duvdy);

    if (smoothnessPixels <= 0.0)
        return shadowSpreadAlpha(center, spreadPixels, duvdx, duvdy);

    float total = shadowSpreadAlpha(center, spreadPixels, duvdx, duvdy);
    float weightSum = 1.0;

    for (int r = 1; r <= SHADOW_BLUR_RINGS; r++)
    {
        float ringRadius = smoothnessPixels * (float(r) / float(SHADOW_BLUR_RINGS));
        float weight = 1.0 - (float(r) / float(SHADOW_BLUR_RINGS)) * 0.6; // inner rings count more
        for (int i = 0; i < SHADOW_BLUR_SAMPLES; i++)
        {
            float angle = PI2 * float(i) / float(SHADOW_BLUR_SAMPLES);
            vec2 offset = pixelsToUV(vec2(cos(angle), sin(angle)) * ringRadius, duvdx, duvdy);
            total += shadowSpreadAlpha(center + offset, spreadPixels, duvdx, duvdy) * weight;
            weightSum += weight;
        }
    }
    return total / weightSum;
}

void main()
{
    vec4 texColor = sampleTexClamped(v_texcoord0);

    // Measured once per fragment, shared by every effect below — see
    // pixelsToUV()'s comment for why this replaces a CPU-computed ratio.
    //
    // duvdy is negated: dFdy walks the RASTERIZER's own fragment-coordinate Y
    // axis, whose direction (up- or down-increasing) is a backend/pipeline
    // convention independent of this UI system's own "local/screen +Y = down"
    // convention (see UiRenderer.h/UiElement.h — every position, offset, and
    // size in this codebase is authored assuming +Y is down). u_Projection
    // (see UiRenderer::SetShaderProjection) is deliberately built to make
    // geometry LAND in the right place either way, but that correction
    // doesn't reach a value measured post-rasterization like this one, so it
    // has to be applied here instead. Isotropic effects (outline, glow, the
    // shadow's own spread/blur) sample a full symmetric ring and are
    // unaffected either way; only a directional offset (shadowOffset) would
    // visibly point the wrong way (up instead of down) without this.
    vec2 duvdx = dFdx(v_texcoord0);
    vec2 duvdy = -dFdy(v_texcoord0);

    // ---- base sprite (original behaviour) ----
    vec4 baseColor = texColor * u_Color;
    vec4 baseLayer = vec4(baseColor.rgb * baseColor.a, baseColor.a);

    // ---- outline: ring dilation minus the shape itself ----
    vec4 outlineLayer = vec4_splat(0.0);
    if (u_OutlineParams.y > 0.5 && u_OutlineParams.x > 0.0)
    {
        float dilated = ringMaxAlpha(v_texcoord0, u_OutlineParams.x, duvdx, duvdy);
        float mask = clamp(dilated - texColor.a, 0.0, 1.0) * u_OutlineColor.a;
        outlineLayer = vec4(u_OutlineColor.rgb * mask, mask);
    }

    // ---- glow: soft halo extending beyond the shape ----
    vec4 glowLayer = vec4_splat(0.0);
    if (u_GlowParams.z > 0.5 && u_GlowParams.x > 0.0)
    {
        float mask = clamp(glowAlpha(v_texcoord0, u_GlowParams.x, duvdx, duvdy) * u_GlowParams.y, 0.0, 1.0) * u_GlowColor.a;
        glowLayer = vec4(u_GlowColor.rgb * mask, mask);
    }

    // ---- drop shadow: offset, thickened by spread, softened by smoothness ----
    vec4 shadowLayer = vec4_splat(0.0);
    if (u_ShadowParams.w > 0.5)
    {
        float mask = shadowAlpha(v_texcoord0, u_ShadowParams.xy, u_ShadowParams.z, u_ShadowParams2.x, duvdx, duvdy) * u_ShadowColor.a;
        shadowLayer = vec4(u_ShadowColor.rgb * mask, mask);
    }

    // Compose back-to-front: shadow, glow, outline, base sprite on top
    vec4 result = shadowLayer;
    result = over(glowLayer, result);
    result = over(outlineLayer, result);
    result = over(baseLayer, result);

    gl_FragColor = vec4(result.rgb* result.a, result.a);
}