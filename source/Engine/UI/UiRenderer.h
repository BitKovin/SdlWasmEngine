#pragma once

#include "../glm.h"
#include "../gl.h"
#include <string>
#include <unordered_map>

#include <bgfx/bgfx.h>

using namespace std;

class Texture;

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
    void DrawTexturedRectShaderParams(const glm::vec2& pos, const glm::vec2& size, float rotation, glm::vec2 pivot, std::unordered_map<std::string, bgfx::TextureHandle>& textures, std::unordered_map<std::string, float>& scalars, std::unordered_map<std::string, vec4>& vec4s, const glm::vec4& color, const string& shader);
    void DrawBorderRect(const glm::vec2& pos, const glm::vec2& size, const glm::vec4& color);
    void DrawText(std::string text, FontHandle font,
                  const glm::vec2& pos, float rotation, vec2 pivot,
                  const glm::vec4& color, const glm::vec2& scale,
                  const string& shader = "");
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
    void DrawTexturedRectShaderParams(const glm::mat3& transform, const glm::vec2& size,
                                      std::unordered_map<std::string, bgfx::TextureHandle>& textures,
                                      std::unordered_map<std::string, float>& scalars,
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
                  const string& shader = "");

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
