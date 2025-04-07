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
    void DrawTexturedRect(const glm::vec2& pos, const glm::vec2& size, GLuint texture, const glm::vec4& color = glm::vec4(1.0f));
    void DrawBorderRect(const glm::vec2& pos, const glm::vec2& size, const glm::vec4& color);
    // Draw text using SDL_TTF. This creates an SDL surface and a temporary OpenGL texture,
    // then draws it as a textured quad.
    void DrawText(const std::string& text, TTF_Font* font, const glm::vec2& pos, const glm::vec4& color, const glm::vec2& scale);
}
