#pragma once

#include "UiElement.h"
#include "UiRenderer.h"
#include <string>
#include "../glm.h"
#include "../AssetRegistry.h"

const float StaticFontSize = 200;

class UiText : public UiElement
{

private:

    std::string finalizedText = "";


public:
    UiRenderer::FontHandle font = 0;
    std::string text;
    float       fontSize = 48;
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

    void FinalizeChildren() override
    {
        UiElement::FinalizeChildren();
        finalizedText = text;
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
        const glm::vec2 scale(fontSize / StaticFontSize);

        // Calculate the conversion ratio from screen-space to atlas-space
        const float toAtlas = fontSize / StaticFontSize;

        std::string shader = "";
        std::unordered_map<std::string, glm::vec4> uniforms;
        float effectPadding = 0.f;

        if (finalizedShadowEnabled || finalizedGlowEnabled || finalizedOutlineEnabled)
        {
            shader = "ui/fs_effects";

            // Convert all screen-space effect parameters into atlas-space
            glm::vec2 atlasShadowOffset = finalizedShadowOffset;
            float     atlasShadowSpread = finalizedShadowSpread;
            float     atlasShadowSoftness = finalizedShadowSoftness;
            float     atlasOutlineWidth = finalizedOutlineWidth;
            float     atlasGlowRadius = finalizedGlowRadius;

            uniforms["u_ShadowColor"] = finalizedShadowColor;
            uniforms["u_ShadowParams"] = glm::vec4(atlasShadowOffset.x, atlasShadowOffset.y,
                atlasShadowSpread, finalizedShadowEnabled ? 1.f : 0.f);
            uniforms["u_ShadowParams2"] = glm::vec4(atlasShadowSoftness, 0.f, 0.f, 0.f);

            uniforms["u_OutlineColor"] = finalizedOutlineColor;
            uniforms["u_OutlineParams"] = glm::vec4(atlasOutlineWidth, finalizedOutlineEnabled ? 1.f : 0.f, 0.f, 0.f);

            uniforms["u_GlowColor"] = finalizedGlowColor;
            uniforms["u_GlowParams"] = glm::vec4(atlasGlowRadius, finalizedGlowIntensity,
                finalizedGlowEnabled ? 1.f : 0.f, 0.f);

            // Calculate effect padding in ATLAS PIXELS
            const float shadowReach = finalizedShadowEnabled ? (glm::length(atlasShadowOffset) + atlasShadowSoftness) : 0.f;
            const float outlineReach = finalizedOutlineEnabled ? atlasOutlineWidth : 0.f;
            const float glowReach = finalizedGlowEnabled ? atlasGlowRadius : 0.f;
            effectPadding = std::max({ shadowReach, outlineReach, glowReach });
        }

        UiRenderer::DrawText(finalizedText, font, finalizedMatrix, textColor * GetFinalColor(), scale, shader, uniforms, effectPadding);

        UiElement::Draw();
    }
};
