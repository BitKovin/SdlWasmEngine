#pragma once

#include "UiElement.h"   // Assumed to be provided in your project
#include "UiRenderer.h"    // Assumed to be provided in your project
#include <SDL2/SDL_ttf.h>
#include <string>
#include "../glm.h"

#include "../AssetRegistry.h"

const float StaticFontSize = 200;

class UiText : public UiElement {
public:
    TTF_Font* font = nullptr;   // Assign your default font here or externally
    std::string text;
    float fontSize = 48;
    glm::vec4 textColor = glm::vec4(1.0f); // White (RGBA)

    UiText() {
        font = AssetRegistry::GetFontFromFile("GameData/fonts/Kingthings_Calligraphica_2.ttf", StaticFontSize);
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

        int totalWidth = 0;
        int totalHeight = 0;
        int lineHeight = TTF_FontLineSkip(font);

        std::stringstream ss(text);
        std::string line;
        int lineCount = 0;

        while (std::getline(ss, line, '\n')) {
            int w = 0, h = 0;
            // Empty lines still contribute height but have no measurable width
            const std::string& toMeasure = line.empty() ? " " : line;
            if (TTF_SizeUTF8(font, toMeasure.c_str(), &w, &h) != 0)
                return glm::vec2(0.0f);

            totalWidth = std::max(totalWidth, w);
            lineCount++;
        }

        // Use lineSkip for consistent spacing (same logic as RenderMultilineText)
        totalHeight = lineCount > 0 ? lineHeight * lineCount : 0;

        return glm::vec2(totalWidth, totalHeight) * (fontSize / StaticFontSize);
    }

    // Draw renders the text using the Renderer::DrawText method.
    virtual void Draw() override {
        // Calculate the drawing position by adding the element offset.
        glm::vec2 pos = position + offset;

        // Compute scale factor based on desired font size.
        glm::vec2 scale(fontSize / StaticFontSize);

        // Draw the text.
        UiRenderer::DrawText(text, font, pos, rotation, pivot, textColor * GetFinalColor(), scale);

        // Optionally, draw child elements and borders.
        UiElement::Draw();
    }
};
