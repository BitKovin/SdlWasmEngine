#pragma once

#include "../glm.h"
#include <SDL2/SDL_ttf.h>
#include "../gl.h"
#include <string>

using namespace std;

class Texture;

namespace UiRenderer {
    void Init(); // Call once at startup
    void Shutdown(); // Optional
    void DrawTexturedRect(const glm::vec2& pos, const glm::vec2& size,float rotation, vec2 pivot, GLuint texture, const glm::vec4& color = glm::vec4(1.0f));
    void DrawTexturedRectShader(const glm::vec2& pos, const glm::vec2& size, float rotation, vec2 pivot, GLuint texture, const glm::vec4& color, const string& shader);
    void DrawBorderRect(const glm::vec2& pos, const glm::vec2& size, const glm::vec4& color);
    // Draw text using SDL_TTF. Uses a texture cache to avoid recreating textures.
    void DrawText(std::string text, TTF_Font* font, const glm::vec2& pos, float rotation, vec2 pivot, const glm::vec4& color, const glm::vec2& scale, const string& shader = "");
    // Call at the end of each frame to update time and clean the cache
    void EndFrame();

	inline bool customViewport = false;
	inline ivec2 customViewportSize = ivec2(800, 600);

}