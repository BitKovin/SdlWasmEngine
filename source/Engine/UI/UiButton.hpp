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
    vec4 HoverColor = vec4(.2f, 0.2f, 0.2f, 1);

    // Set externally to force hover visual (e.g. by a scroll region parent).
    bool IsHovered = false;

    std::function<void()> onClick = nullptr;

    // Optional keyboard confirm override. If null, defaults to onClick.
    // Assign when confirm should differ from click (e.g. UiDropdown header
    // opens the panel on confirm rather than toggling it).
    std::function<void()> onNavConfirm = nullptr;

    bool OnlyTouch = false;
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

    void OnNavConfirm() override
    {
        if (onNavConfirm) onNavConfirm();
        else if (onClick) onClick();
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

        const bool hovered = IsHovered || !TouchEvents.empty() || IsFocused;
        const vec4 tint = hovered ? HoverColor : Color;

        if (PixelShader.empty())
            UiRenderer::DrawTexturedRect(pos, finalizedSize, rotation, pivot, tex->getHandle(), tint * GetFinalColor());
        else
            UiRenderer::DrawTexturedRectShader(pos, finalizedSize, rotation, pivot, tex->getHandle(), tint * GetFinalColor(), PixelShader);

        UiElement::Draw();
    }
};