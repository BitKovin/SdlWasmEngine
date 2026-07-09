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

    // ---- finalized effect parameters (snapshot used at render time) ----
    bool        finalizedShadowEnabled = false;
    glm::vec4   finalizedShadowColor = glm::vec4(0.f, 0.f, 0.f, 1.f);
    glm::vec2   finalizedShadowOffset = glm::vec2(2.f, -2.f);
    float       finalizedShadowSoftness = 1.f;
    float       finalizedShadowSpread = 4.f;

    bool        finalizedOutlineEnabled = false;
    glm::vec4   finalizedOutlineColor = glm::vec4(0.f, 0.f, 0.f, 1.f);
    float       finalizedOutlineWidth = 1.f;

    bool        finalizedGlowEnabled = false;
    glm::vec4   finalizedGlowColor = glm::vec4(1.f, 1.f, 1.f, 1.f);
    float       finalizedGlowRadius = 6.f;
    float       finalizedGlowIntensity = 1.f;

public:
    UiRenderer::FontHandle font = 0;
    std::string text;
    float       fontSize = 48;
    glm::vec4   textColor = glm::vec4(1.f);

    // ---- shadow ----
    bool        shadowEnabled = false;
    glm::vec4   shadowColor = glm::vec4(0.f, 0.f, 0.f, 1.f);
    glm::vec2   shadowOffset = glm::vec2(6.f, 6.f);
    float       shadowSoftness = 3.f;
    float       shadowSpread = 3.f;

    // ---- outline ----
    bool        outlineEnabled = false;
    glm::vec4   outlineColor = glm::vec4(0.f, 0.f, 0.f, 1.f);
    float       outlineWidth = 1.f;

    // ---- glow ----
    bool        glowEnabled = false;
    glm::vec4   glowColor = glm::vec4(1.f, 1.f, 1.f, 1.f);
    float       glowRadius = 6.f;
    float       glowIntensity = 1.f;

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

        finalizedShadowEnabled = shadowEnabled;
        finalizedShadowColor = shadowColor;
        finalizedShadowOffset = shadowOffset;
        finalizedShadowSoftness = shadowSoftness;
		finalizedShadowSpread = shadowSpread;

        finalizedOutlineEnabled = outlineEnabled;
        finalizedOutlineColor = outlineColor;
        finalizedOutlineWidth = outlineWidth;

        finalizedGlowEnabled = glowEnabled;
        finalizedGlowColor = glowColor;
        finalizedGlowRadius = glowRadius;
        finalizedGlowIntensity = glowIntensity;
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

        std::string shader = "";
        std::unordered_map<std::string, glm::vec4> uniforms;
        float effectPadding = 0.f;

        if (finalizedShadowEnabled || finalizedGlowEnabled || finalizedOutlineEnabled)
        {
            shader = "ui/fs_effects";

            uniforms["u_ShadowColor"] = finalizedShadowColor;
            uniforms["u_ShadowParams"] = glm::vec4(finalizedShadowOffset.x, finalizedShadowOffset.y,
                finalizedShadowSpread, finalizedShadowEnabled ? 1.f : 0.f);
			uniforms["u_ShadowParams2"] = glm::vec4(finalizedShadowSoftness, 0.f, 0.f, 0.f);

            uniforms["u_OutlineColor"] = finalizedOutlineColor;
            uniforms["u_OutlineParams"] = glm::vec4(finalizedOutlineWidth, finalizedOutlineEnabled ? 1.f : 0.f, 0.f, 0.f);

            uniforms["u_GlowColor"] = finalizedGlowColor;
            uniforms["u_GlowParams"] = glm::vec4(finalizedGlowRadius, finalizedGlowIntensity,
                finalizedGlowEnabled ? 1.f : 0.f, 0.f);

            const float shadowReach = finalizedShadowEnabled ? (glm::length(finalizedShadowOffset) + finalizedShadowSoftness) : 0.f;
            const float outlineReach = finalizedOutlineEnabled ? finalizedOutlineWidth : 0.f;
            const float glowReach = finalizedGlowEnabled ? finalizedGlowRadius : 0.f;
            effectPadding = std::max({ shadowReach, outlineReach, glowReach });
        }

        UiRenderer::DrawText(finalizedText, font, finalizedMatrix, textColor * GetFinalColor(), scale, shader, uniforms, effectPadding);

        UiElement::Draw();
    }
};
