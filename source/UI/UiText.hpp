#pragma once

#include "UiElement.h"   // Assumed to be provided in your project
#include "UiRenderer.h"    // Assumed to be provided in your project
#include <SDL2/SDL_ttf.h>
#include <string>
#include "../glm.h"

#include "../AssetRegisty.h"

class UiText : public UiElement {
public:
    TTF_Font* font = nullptr;   // Assign your default font here or externally
    std::string text;
    float fontSize = 48;
    glm::vec4 baseColor = glm::vec4(1.0f); // White (RGBA)

    UiText() {
        font = AssetRegistry::GetFontFromFile("GameData/Fonts/Kingthings_Calligraphica_2.ttf", 72);
    }

    virtual ~UiText() = default;

    // Update recalculates the size and updates base UiElement properties.
    virtual void Update() override {
        size = GetSize();
        UiElement::Update();
    }

    // GetSize measures the text using SDL_TTF and scales the result.
    virtual glm::vec2 GetSize() override {
        if (!font) return glm::vec2(0.0f);
        int w = 0, h = 0;
        if (TTF_SizeUTF8(font, text.c_str(), &w, &h) != 0) {
            // If there's an error, return zero size.
            return glm::vec2(0.0f);
        }
        // Scale based on fontSize relative to a 72 DPI reference.
        return glm::vec2(w, h) * (fontSize / 72.f);
    }

    // Draw renders the text using the Renderer::DrawText method.
    virtual void Draw() override {
        // Calculate the drawing position by adding the element offset.
        glm::vec2 pos = position + offset;

        // Compute scale factor based on desired font size.
        glm::vec2 scale(fontSize / 72.f);

        // Draw the text.
        UiRenderer::DrawText(text, font, pos, baseColor, scale);

        // Optionally, draw child elements and borders.
        UiElement::Draw();
    }
};
