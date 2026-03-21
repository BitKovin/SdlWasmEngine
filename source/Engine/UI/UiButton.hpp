#pragma once

#include "UiElement.h"
#include "UiRenderer.h"
#include "../Texture.hpp"
#include "../AssetRegistry.h"
#include "../EngineMain.h"
#include "../Input.h"

class UiButton : public UiElement
{
private:
    Texture* tex = nullptr;

public:
    std::string ImagePath = "GameData/texture/ui/white.png";

    vec4 Color = vec4(.15f, 0.15f, 0.15f, 1);

    // Visual-only hover tint. Shown while this element has active touch events
    // (pressed or held) or while IsHovered is set externally.
    // Has no effect on interaction logic.
    vec4 HoverColor = vec4(.2f, 0.2f, 0.2f, 1);

    // Driven externally by a parent that intercepts touches (e.g. UiScrollRegion)
    // so children with HitCheck=false still get hover feedback. Visual only.
    bool IsHovered = false;

    std::function<void()> onClick = nullptr;

    bool OnlyTouch     = false;
    bool OnlyNotPaused = false;

    UiButton() { HitCheck = true; }
    ~UiButton() {}

    void Update() override
    {
        UiElement::Update();

        if (EngineMain::MainInstance->Paused && OnlyNotPaused) return;

        for (const auto& touch : TouchEvents)
        {
            if (touch.id < 10 && OnlyTouch) continue;

            if (touch.released)
            {
                if (onClick) onClick();
            }
        }
    }


    void Draw() override
    {
        if (Level::ChangingLevel)
        {
            tex = AssetRegistry::GetTextureFromFile(ImagePath);
        }
        else
        {
            if (tex == nullptr)
            {
                tex = AssetRegistry::GetTextureFromFile(ImagePath);
                if (!tex->valid)
                    tex = AssetRegistry::GetTextureFromFile("GameData/textures/generic/white.png");
            }
        }

        vec2 pos = finalizedPosition + finalizedOffset;

        const bool hovered = IsHovered || !TouchEvents.empty();
        const vec4 tint    = hovered ? HoverColor : Color;

        if (PixelShader.empty())
            UiRenderer::DrawTexturedRect(pos, finalizedSize, rotation, pivot, tex->getHandle(), tint * GetFinalColor());
        else
            UiRenderer::DrawTexturedRectShader(pos, finalizedSize, rotation, pivot, tex->getHandle(), tint * GetFinalColor(), PixelShader);

        UiElement::Draw();
    }
};
