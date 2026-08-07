#pragma once

#include "../glm.h"
#include "../gl.h"
#include <string>
#include <unordered_map>

#include <bgfx/bgfx.h>

using namespace std;

class Texture;

// The UI canvas is always exactly the render target's resolution — 1 UI unit
// == 1 output pixel, no separate DPI/UI-scale factor anywhere in this system.
// Worth knowing if you're writing a fragment shader that wants to reason
// about on-screen pixel size: element->size (and finalizedSize) already *is*
// that, no conversion needed. This holds equally whether rendering to the
// real screen or to customViewport (see below) — only the pixel dimensions
// of "the render target" change, the 1:1 mapping doesn't.
//
// This is also why every shadow/outline/glow property on UiElement (offset,
// spread, softness, outline width, glow radius) is authored in the same
// viewport-pixel units, never in element-local [0,1] space or in the bound
// texture's own texel space. The ui/fs_effects shader converts "N viewport
// pixels" to the right UV delta to sample using screen-space derivatives of
// its own UV (dFdx/dFdy) rather than a value computed here in C++ — that's
// deliberate: a value computed from `size` here would have to assume
// declared element size maps 1:1 onto this draw's actual rasterized
// resolution, which isn't safe to assume (DPI/backbuffer scale differences,
// customViewport, etc. can all break that assumption even though the
// vertex-side transform doesn't care). Measuring it on the GPU from real
// neighboring fragments sidesteps needing that assumption at all. Every
// draw function that can carry effects still hands the shader
// u_ViewportSize and u_ElementSize for whatever future effect wants that
// context explicitly.

namespace UiRenderer {

    // Opaque handle returned by LoadFont. Pass to DrawText and UnloadFont.
    using FontHandle = uint32_t;
    static constexpr FontHandle INVALID_FONT = 0;

    void Init();
    void Shutdown();

    // ── Font atlas API ─────────────────────────────────────────────────────────
    FontHandle LoadFont(const char* path, float pixelHeight);
    void       UnloadFont(FontHandle handle);

    // ── Draw calls — legacy (pos + rotation + pivot) ───────────────────────────
    // These remain for any code that hasn't been ported to matrix calls yet.
    void DrawTexturedRect(const glm::vec2& pos, const glm::vec2& size, float rotation, vec2 pivot, bgfx::TextureHandle texture, const glm::vec4& color = glm::vec4(1.0f));
    void DrawTexturedRectShader(const glm::vec2& pos, const glm::vec2& size, float rotation, glm::vec2 pivot, bgfx::TextureHandle texture, const glm::vec4& color, const string& shader);
    void DrawTexturedRectShaderParams(const glm::vec2& pos, const glm::vec2& size, float rotation, glm::vec2 pivot, std::unordered_map<std::string, bgfx::TextureHandle>& textures, std::unordered_map<std::string, vec4>& vec4s, const glm::vec4& color, const string& shader);
    void DrawBorderRect(const glm::vec2& pos, const glm::vec2& size, const glm::vec4& color);

    glm::vec2 MeasureText(const std::string& text, FontHandle font);

    // ── Draw calls — matrix-based ──────────────────────────────────────────────
    //
    // The mat3 transform maps element-local space (origin = top-left of the
    // element, spanning [0, size]) to screen space, fully encoding translation,
    // rotation, and any scale.  The renderer multiplies each of the four quad
    // corners through this matrix before submitting geometry, so parent
    // rotations are propagated automatically.
    //
    // Use these overloads from every UiElement::Draw() implementation.
    // Obtain the matrix from element.finalizedMatrix.

    // Solid / textured rectangle.
    void DrawTexturedRect(const glm::mat3& transform, const glm::vec2& size,
                          bgfx::TextureHandle texture,
                          const glm::vec4& color = glm::vec4(1.f));

    // Textured rectangle with a custom pixel shader (single texture slot).
    void DrawTexturedRectShader(const glm::mat3& transform, const glm::vec2& size,
                                bgfx::TextureHandle texture,
                                const glm::vec4& color,
                                const string& shader, float textureHeight, float textureWidth);

    // Textured rectangle with a custom pixel shader (multiple parameter maps).
    // No separate float/"scalar" map — bgfx uniforms are vec4-sized no matter
    // what you conceptually pass, so a scalar just goes in as vec4(v,0,0,0).
    void DrawTexturedRectShaderParams(const glm::mat3& transform, const glm::vec2& size,
                                      std::unordered_map<std::string, bgfx::TextureHandle>& textures,
                                      std::unordered_map<std::string, vec4>& vec4s,
                                      const glm::vec4& color,
                                      const string& shader);

    // Debug border rectangle.
    void DrawBorderRect(const glm::mat3& transform, const glm::vec2& size,
                        const glm::vec4& color);

    // Text — the transform places the text origin in screen space (accounts for
    // all ancestor rotations).  scale is the font-size multiplier applied on top
    // of the atlas pixel size, independent of the transform. effectPadding is
    // in VIEWPORT PIXELS (same as DrawTexturedRectRegion/DrawTexturedRect9Slice)
    // — internally converted to the font atlas's own native pixel space using
    // `scale`, since that's the space the glyph geometry/UV math operates in.
    void DrawText(std::string text, FontHandle font,
                  const glm::mat3& transform,
                  const glm::vec4& color, const glm::vec2& scale,
                  const string& shader = "", std::unordered_map<std::string, vec4> shaderUniforms = {}, float effectPadding = 0.f);

    // ── Draw calls — transient vertex buffer ───────────────────────────────────
    //
    // Everything above submits the same static unit quad (s_quadVB) — cheap,
    // but fixed to exactly [0,size], so it can't draw a partial rect and can't
    // give shadow/outline/glow room to bleed past the element's own edges.
    // These two build a fresh transient VB per call instead, which costs more
    // but can do both. Prefer the plain calls above for the common case
    // (full rect, no effects); reach for these when you actually need a
    // partial rect, effect padding, or 9-slicing.
    //
    // effectPadding is in VIEWPORT PIXELS, same meaning as DrawText's — both
    // position and UV are inflated to match (converted internally to each
    // draw's own local/UV space), so the extra geometry samples real texels
    // just past the requested rect rather than stretching or repeating the
    // edge, and a "6px shadow" reaches 6 actual screen pixels regardless of
    // the bound texture's resolution. Pass 0 for no padding.

    // Sub-rectangle of this element (rectPos/rectSize in local [0,1] space —
    // (0,0)/(1,1) draws the whole element, same as DrawTexturedRect) mapped
    // 1:1 onto the same range of `texture`'s UV space. This is the geometry-
    // masking primitive: only the requested fraction is actually submitted as
    // geometry, so unlike a fragment-shader discard/mix, nothing is drawn (or
    // sampled) outside the requested rect at all.
    // effectPadding: viewport pixels. textureWidth/textureHeight: the real
    // bound texture's pixel dimensions — no longer used for the padding math
    // (see the .cpp; geometry/UV padding here is always 1:1 with the element,
    // independent of texture resolution) but still accepted for symmetry
    // with DrawTexturedRect9Slice and in case a future caller needs it.
    void DrawTexturedRectRegion(const glm::mat3& transform, const glm::vec2& size,
                                const glm::vec2& rectPos, const glm::vec2& rectSize,
                                bgfx::TextureHandle texture, const glm::vec4& color,
                                const string& shader = "", std::unordered_map<std::string, vec4> shaderUniforms = {},
                                float effectPadding = 0.f, float textureWidth = 0.f, float textureHeight = 0.f);

    // Margins in source-texture texels. Corners render at their native texel
    // size in screen pixels regardless of how much the element is stretched;
    // edges stretch along one axis; the center stretches both. Not currently
    // combinable with a partial rect (DrawTexturedRectRegion) in one call —
    // this always draws the full element.
    //
    // effectPadding (viewport pixels, same as DrawTexturedRectRegion/DrawText)
    // extends only the OUTER boundary of the whole 9-slice grid by that many
    // screen pixels on all 4 sides — the corner/edge/center proportions
    // between the margins themselves never move, so shadow/outline/glow gets
    // real geometry (and real texture UV, sampled just past the source
    // texture's own edge) to bleed into without disturbing the 9-slice's own
    // stable footprint. The outward padding amount itself is exact at the
    // corners and at each edge cell's native (unstretched) axis, since a
    // 9-slice's outer boundary is always native (1 texel = 1 pixel) scale by
    // construction. Ring-sampling reach (shadow/outline/glow radius) is exact
    // everywhere, including deep inside a stretched edge/center cell —
    // fs_effects.sc derives the pixel-to-UV conversion per fragment from
    // screen-space derivatives rather than a single value for the whole draw,
    // so it naturally picks up each cell's own texel:pixel ratio.
    struct NineSliceMargins { float left = 0.f, top = 0.f, right = 0.f, bottom = 0.f; };

    void DrawTexturedRect9Slice(const glm::mat3& transform, const glm::vec2& size,
                                const NineSliceMargins& margins,
                                bgfx::TextureHandle texture, const glm::vec4& color,
                                const string& shader = "", std::unordered_map<std::string, vec4> shaderUniforms = {},
                                float effectPadding = 0.f, float textureWidth = 0.f, float textureHeight = 0.f);

    // ── Stencil mask ──────────────────────────────────────────────────────────
    // Legacy overload (kept for compatibility).
    void PushMask(const glm::vec2& pos, const glm::vec2& size,
                  float rotation = 0.f, glm::vec2 pivot = glm::vec2(0.f));

    // Matrix overload — preferred; encodes all ancestor transforms.
    void PushMask(const glm::mat3& transform, const glm::vec2& size);

    void PopMask();
    void ClearStencil();

    // ── Frame boundary ────────────────────────────────────────────────────────
    void EndFrame();

    inline bool  customViewport = false;
    inline ivec2 customViewportSize = ivec2(800, 600);

} // namespace UiRenderer
