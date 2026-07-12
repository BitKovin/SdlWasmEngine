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
// that, no conversion needed.

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
    // of the atlas pixel size, independent of the transform.
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
    // effectPadding is in source-texture texels, same meaning as DrawText's —
    // both position and UV are inflated by effectPadding/textureWidth (or
    // height), so the extra geometry samples real texels just past the
    // requested rect rather than stretching or repeating the edge. Pass 0 (or
    // textureWidth/Height = 0) for no padding.

    // Sub-rectangle of this element (rectPos/rectSize in local [0,1] space —
    // (0,0)/(1,1) draws the whole element, same as DrawTexturedRect) mapped
    // 1:1 onto the same range of `texture`'s UV space. This is the geometry-
    // masking primitive: only the requested fraction is actually submitted as
    // geometry, so unlike a fragment-shader discard/mix, nothing is drawn (or
    // sampled) outside the requested rect at all.
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
    // Unlike DrawTexturedRectRegion/DrawText, effectPadding does NOT expand
    // the geometry here — total size is always exactly `size`, regardless of
    // effectPadding. Shadow/outline/glow on a 9-sliced element render
    // clipped to its own box instead of bleeding past it; a stable,
    // predictable footprint is the point of 9-slicing in the first place.
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
