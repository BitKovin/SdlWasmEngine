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

    void Init();    // Call once at startup
    void Shutdown(); // Optional

    // ── Font atlas API ────────────────────────────────────────────────────────────
    // Load a .ttf file and build a GPU-backed glyph atlas at the given pixel height.
    // Returns INVALID_FONT on failure.
    FontHandle LoadFont(const char* path, float pixelHeight);
    void       UnloadFont(FontHandle handle);

    // ── Draw calls ────────────────────────────────────────────────────────────────
    void DrawTexturedRect(const glm::vec2& pos, const glm::vec2& size, float rotation, vec2 pivot, bgfx::TextureHandle texture, const glm::vec4& color = glm::vec4(1.0f));
    void DrawTexturedRectShader(const glm::vec2& pos, const glm::vec2& size, float rotation, glm::vec2 pivot, bgfx::TextureHandle texture, const glm::vec4& color, const string& shader);
    void DrawTexturedRectShaderParams(const glm::vec2& pos, const glm::vec2& size, float rotation, glm::vec2 pivot, std::unordered_map<std::string, bgfx::TextureHandle>& textures, std::unordered_map<std::string, float>& scalars, std::unordered_map<std::string, vec4>& vec4s, const glm::vec4& color, const string& shader);
    void DrawBorderRect(const glm::vec2& pos, const glm::vec2& size, const glm::vec4& color);

    // Draw text using a font atlas built from a .ttf file (stb_truetype).
    // New Unicode codepoints are rasterized and packed into the atlas on demand.
    // `scale` multiplies the font's natural pixel size. `pivot` and `rotation` work
    // identically to the other Draw* functions.
    void DrawText(std::string text, FontHandle font,
        const glm::vec2& pos, float rotation, vec2 pivot,
        const glm::vec4& color, const glm::vec2& scale,
        const string& shader = "");

    // Measure the bounding box of `text` in atlas pixels (before `scale` is applied).
    // Returns glm::vec2(0) if the font handle is invalid or text is empty.
    glm::vec2 MeasureText(const std::string& text, FontHandle font);

    // Call at the end of each frame to upload any dirty atlas changes to the GPU.
    void EndFrame();

    inline bool  customViewport = false;
    inline ivec2 customViewportSize = ivec2(800, 600);

} // namespace UiRenderer