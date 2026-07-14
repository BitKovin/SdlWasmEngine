#pragma once

#include "UiElement.h"
#include "UiButton.hpp"
#include "UiImage.hpp"
#include "UiText.hpp"

#include <functional>
#include <string>

#include "InputIconLibrary.h"

// ---------------------------------------------------------------------------
// UiBindSlotButton
//
// One rebindable slot inside a keybind row: an icon (with a small text
// caption as a readable fallback for icons that haven't been art-directed
// yet) plus a tiny "x" clear button in the corner.
//
// Both the main slot button and the clear button are ordinary HitCheck=true
// UiButtons, so UiNavigation picks them up automatically — no custom nav
// code needed here, same as every other control in this UI.
//
//   Click / Confirm on the slot  -> onActivate()  (caller opens the capture
//                                    modal for this slot)
//   Click / Confirm on the "x"   -> onClear()      (only visible/focusable
//                                    once the slot is actually bound)
// ---------------------------------------------------------------------------

class UiBindSlotButton : public UiElement
{
public:
    vec2 SlotSize = vec2(72.f, 72.f);

    std::function<void()> onActivate = nullptr;
    std::function<void()> onClear = nullptr;

    UiBindSlotButton()
    {
        HitCheck = false; // this container itself isn't a nav target — its children are

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

        clearButton = std::make_shared<UiButton>();
        clearButton->origin = vec2(1.f, 0.f);
        clearButton->pivot = vec2(1.f, 0.f);
        clearButton->size = vec2(20.f, 20.f);
        clearButton->Color = vec4(0.45f, 0.10f, 0.10f, 1.f);
        clearButton->HoverColor = vec4(0.65f, 0.15f, 0.15f, 1.f);
        clearButton->ImagePath = "GameData/textures/generic/white.png";
        clearButton->onClick = [this]() { if (onClear) onClear(); };
        UiElement::AddChild(clearButton);

        clearLabel = std::make_shared<UiText>();
        clearLabel->text = "x";
        clearLabel->fontSize = 18.f;
        clearLabel->pivot = vec2(0.5f);
        clearLabel->origin = vec2(0.5f);
        clearButton->AddChild(clearLabel);
    }

    // Call after resolving the icon path + fallback label for the current
    // effective binding (see UiInputSettings::BuildRow).
    void SetContent(const std::string& iconPath, const std::string& labelText, bool bound, bool listening)
    {
        icon->ImagePath = iconPath;
        icon->size = SlotSize * 0.72f;
        icon->color = bound ? vec4(1.f) : vec4(1.f, 1.f, 1.f, 0.35f);

        caption->text = bound ? labelText : "Unbound";
        caption->position = vec2(SlotSize.x * 0.5f, SlotSize.y - 2.f);
        captionBg->size = vec2(SlotSize.x, 18.f);

        clearButton->visible = bound;
        clearButton->HitCheck = bound;

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
    std::shared_ptr<UiButton> clearButton;
    std::shared_ptr<UiText> clearLabel;
};
