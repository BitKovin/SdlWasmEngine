#pragma once

#include "UiElement.h"
#include "UiRenderer.h"
#include <string>
#include "../glm.h"
#include "../AssetRegistry.h"

const float StaticFontSize = 200;

class UiText : public UiElement
{
public:
    UiRenderer::FontHandle font = 0;
    std::string text;
    float       fontSize  = 48;
    glm::vec4   textColor = glm::vec4(1.f);

    UiText()
    {
        font = UiRenderer::LoadFont("GameData/fonts/Kingthings_Calligraphica_2.ttf", StaticFontSize);
    }

    virtual ~UiText() = default;

    virtual void Update() override
    {
        size = GetSize();
        UiElement::Update();
    }

    virtual glm::vec2 GetSize() override
    {
        if (font == UiRenderer::INVALID_FONT) return glm::vec2(0.f);
        glm::vec2 atlasSize = UiRenderer::MeasureText(text, font);
        if (atlasSize.x == 0.f) return glm::vec2(0.f);
        return atlasSize * (fontSize / StaticFontSize);
    }

    virtual void Draw() override
    {
        // finalizedMatrix positions the text origin (top-left of the text bounding
        // box) in screen space, fully accounting for any ancestor rotation.
        // scale drives the atlas-pixel → screen-pixel mapping independently.
        const glm::vec2 scale(fontSize / StaticFontSize);
        UiRenderer::DrawText(text, font, finalizedMatrix, textColor * GetFinalColor(), scale);

        UiElement::Draw();
    }
};
