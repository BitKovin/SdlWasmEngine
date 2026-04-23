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

    // Track which touch ID pressed this button.  onClick only fires when the
    // *same* ID releases — preventing spurious clicks when a finger that
    // started a drag elsewhere lifts over this button.
    int m_pressedTouchId = -1;

public:
    std::string ImagePath = "GameData/texture/ui/white.png";

    vec4 Color = vec4(.15f, 0.15f, 0.15f, 1);
    vec4 HoverColor = vec4(.2f, 0.2f, 0.2f, 1);

    bool IsHovered = false;

    std::function<void()> onClick = nullptr;
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

            if (touch.pressed)
            {
                m_pressedTouchId = touch.id;
            }
            else if (touch.released)
            {
                // Only fire if this is the same touch that originally pressed.
                if (touch.id == m_pressedTouchId)
                {
                    m_pressedTouchId = -1;
                    if (onClick) onClick();
                }
            }
        }

        // If TouchEvents is empty and no touch is holding, clear any stale
        // pressed state — but only when there are no events at all this frame,
        // so we never clear on the same frame the press arrived.
        if (m_pressedTouchId != -1 && TouchEvents.empty())
            m_pressedTouchId = -1;
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

        const bool hovered = IsHovered || !TouchEvents.empty() || IsFocused;
        const vec4 tint = hovered ? HoverColor : Color;

        if (PixelShader.empty())
            UiRenderer::DrawTexturedRect(finalizedMatrix, finalizedSize, tex->getHandle(), tint * GetFinalColor());
        else
            UiRenderer::DrawTexturedRectShader(finalizedMatrix, finalizedSize, tex->getHandle(), tint * GetFinalColor(), PixelShader, tex->height, tex->width);

        UiElement::Draw();
    }
};