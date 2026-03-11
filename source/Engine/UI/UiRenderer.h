#pragma once

#include "../glm.h"
#include <SDL2/SDL_ttf.h>
#include "../gl.h"
#include <string>
#include <unordered_map>

#include <bgfx/bgfx.h>

using namespace std;

class Texture;

namespace UiRenderer {
    void Init(); // Call once at startup
    void Shutdown(); // Optional
    void DrawTexturedRect(const glm::vec2& pos, const glm::vec2& size, float rotation, vec2 pivot, bgfx::TextureHandle texture, const glm::vec4& color = glm::vec4(1.0f));
    void DrawTexturedRectShader(const glm::vec2& pos, const glm::vec2& size, float rotation, glm::vec2 pivot, bgfx::TextureHandle texture, const glm::vec4& color, const string& shader);
    void DrawTexturedRectShaderParams(const glm::vec2& pos, const glm::vec2& size, float rotation, glm::vec2 pivot, std::unordered_map<std::string, bgfx::TextureHandle>& textures, std::unordered_map<std::string, float>& scalars, std::unordered_map<std::string, vec4>& vec4s, const glm::vec4& color, const string& shader);
    void DrawBorderRect(const glm::vec2& pos, const glm::vec2& size, const glm::vec4& color);
    // Draw text using SDL_TTF. Uses a texture cache to avoid recreating textures.
    void DrawText(std::string text, TTF_Font* font, const glm::vec2& pos, float rotation, vec2 pivot, const glm::vec4& color, const glm::vec2& scale, const string& shader = "");
    // Call at the end of each frame to update time and clean the cache
    void EndFrame();

    inline bool customViewport = false;
    inline ivec2 customViewportSize = ivec2(800, 600);

}