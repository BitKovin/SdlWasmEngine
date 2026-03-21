#pragma once

#include "UiElement.h"   // Assumed to be provided in your project
#include "UiRenderer.h"    // Assumed to be provided in your project
#include <string>
#include "../glm.h"

#include "../AssetRegistry.h"

const float StaticFontSize = 200;

class UiText : public UiElement {
public:
    UiRenderer::FontHandle font = 0;   // Assign your default font here or externally
    std::string text;
    float fontSize = 48;
    glm::vec4 textColor = glm::vec4(1.0f); // White (RGBA)

    UiText() {
        font = UiRenderer::LoadFont("GameData/fonts/Kingthings_Calligraphica_2.ttf", StaticFontSize);
    }

    virtual ~UiText() = default;

    // Update recalculates the size and updates base UiElement properties.
    virtual void Update() override {
        size = GetSize();
        UiElement::Update();
    }

    virtual glm::vec2 GetSize() override {
        if (font == UiRenderer::INVALID_FONT) return glm::vec2(0.0f);

        glm::vec2 atlasSize = UiRenderer::MeasureText(text, font);
        if (atlasSize.x == 0.f) return glm::vec2(0.0f);

        return atlasSize * (fontSize / StaticFontSize);
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
