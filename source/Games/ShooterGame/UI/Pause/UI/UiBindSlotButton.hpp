#pragma once

#include <UI/UiElement.h>
#include <UI/UiButton.hpp>
#include <UI/UiImage.hpp>
#include <UI/UiText.hpp>

#include <functional>
#include <string>

#include "InputIconLibrary.h"

// ---------------------------------------------------------------------------
// UiBindSlotButton
//
// One rebindable slot inside a keybind row: an icon (with a small text
// caption as a readable fallback for icons that haven't been art-directed
// yet).
//
// The main slot button is an ordinary HitCheck=true UiButton, so
// UiNavigation picks it up automatically — no custom nav code needed here,
// same as every other control in this UI.
//
//   Click / Confirm on the slot -> onActivate() (caller opens the capture
//                                   modal for this slot). Unbinding no
//                                   longer lives here as a separate "x"
//                                   button glued to the corner — it's now a
//                                   deliberate "Unbind" action inside that
//                                   same capture modal (see
//                                   UiRebindCaptureModal), so there's no
//                                   destructive control sitting on screen
//                                   at all times.
// ---------------------------------------------------------------------------

class UiBindSlotButton : public UiElement
{
public:
    vec2 SlotSize = vec2(85.f, 85.f);

    std::function<void()> onActivate = nullptr;

    UiBindSlotButton()
    {
        HitCheck = false; // this container itself isn't a nav target — its child is

        mainButton = std::make_shared<UiButton>();
        mainButton->origin = vec2(0.f);
        mainButton->pivot = vec2(0.f);
        mainButton->Color = vec4(0.16f, 0.16f, 0.16f, 1.f);
        mainButton->HoverColor = vec4(0.28f, 0.28f, 0.34f, 1.f);
        mainButton->ImagePath = "GameData/textures/generic/white.png";
        mainButton->onClick = [this]() { if (onActivate) onActivate(); };
        UiElement::AddChild(mainButton);

        icon = std::make_shared<UiImage>();
        icon->origin = vec2(0.5f);
        icon->pivot = vec2(0.5f);
        icon->size = vec2(100);
        mainButton->AddChild(icon);

        captionBg = std::make_shared<UiImage>();
        captionBg->origin = vec2(0.5f, 1.f);
        captionBg->pivot = vec2(0.5f, 1.f);
        captionBg->color = vec4(0.f, 0.f, 0.f, 0.55f);
        //mainButton->AddChild(captionBg);

        caption = std::make_shared<UiText>();
        caption->origin = vec2(0.0f, 0.f);
        caption->pivot = vec2(0.0f, 0.f);
        caption->fontSize = 15.f;
        caption->textColor = vec4(1.f);
        //mainButton->AddChild(caption);
    }

    // Call after resolving the icon path + fallback label for the current
    // effective binding (see UiInputSettings::BuildActionRow).
    void SetContent(const std::string& iconPath, const std::string& labelText, bool bound, bool listening)
    {
        // BUG FIX: rebinding an action left the old icon (or a blank/white
        // image) on screen until the whole menu was torn down and rebuilt.
        // Mutating ImagePath on the *existing* UiImage instance doesn't pick
        // up the change — whatever texture cache UiImage keeps internally
        // only seems to get populated on construction. Recreating the icon
        // element each time sidesteps that: a brand-new UiImage always loads
        // fresh from iconPath, the same way it already does correctly the
        // first time this row is ever built.
        icon->RemoveFromParent();
        icon = std::make_shared<UiImage>();
        icon->origin = vec2(0.5f);
        icon->pivot = vec2(0.5f);
        icon->ImagePath = iconPath;
        icon->size = SlotSize * 0.72f;
        icon->color = bound ? vec4(1.f) : vec4(1.f, 1.f, 1.f, 0.35f);
        mainButton->AddChild(icon);

        caption->text = bound ? labelText : "Unbound";
        caption->position = vec2(SlotSize.x * 0.5f, SlotSize.y - 2.f);
        captionBg->size = vec2(SlotSize.x, 18.f);

        mainButton->Color = listening
            ? vec4(0.30f, 0.45f, 0.65f, 1.f)
            : (bound ? vec4(0.16f, 0.16f, 0.16f, 1.f) : vec4(0.10f, 0.10f, 0.10f, 1.f));
    }

    void Update() override
    {
        mainButton->size = SlotSize;
        UiElement::Update();
    }

    glm::vec2 GetSize() override { return SlotSize; }

private:
    std::shared_ptr<UiButton> mainButton;
    std::shared_ptr<UiImage> icon;
    std::shared_ptr<UiImage> captionBg;
    std::shared_ptr<UiText> caption;
};
