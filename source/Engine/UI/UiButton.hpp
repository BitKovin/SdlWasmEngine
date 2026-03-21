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

    vec4 Color      = vec4(.15f, 0.15f, 0.15f, 1);

    // Shown while cursor is over this element (touch events present) or while
    // IsHovered is forced externally. Visual only — no interaction logic.
    vec4 HoverColor = vec4(.2f, 0.2f, 0.2f, 1);

    // Shown while this element has keyboard/gamepad focus.
    // Lower priority than hover so press feedback always wins.
    vec4 FocusColor = vec4(0.25f, 0.25f, 0.45f, 1);

    // Set by a parent (e.g. UiScrollRegion) to show hover on elements
    // with HitCheck = false that never receive touch events themselves.
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

    // Keyboard/gamepad confirm → same as a click.
    void OnNavConfirm() override
    {
        if (onClick) onClick();
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

        vec4 tint;
        if (!TouchEvents.empty() || IsHovered)
            tint = HoverColor;
        else if (IsFocused)
            tint = FocusColor;
        else
            tint = Color;

        if (PixelShader.empty())
            UiRenderer::DrawTexturedRect(pos, finalizedSize, rotation, pivot, tex->getHandle(), tint * GetFinalColor());
        else
            UiRenderer::DrawTexturedRectShader(pos, finalizedSize, rotation, pivot, tex->getHandle(), tint * GetFinalColor(), PixelShader);

        UiElement::Draw();
    }
};
