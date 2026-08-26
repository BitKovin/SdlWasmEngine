#pragma once

#include <UI/UiButton.hpp>
#include <UI/UiText.hpp>
#include "UiSettingsStyle.hpp"
#include <functional>
#include <string>

// ---------------------------------------------------------------------------
// UiSaveSlotRow
//
// One row in the Save/Load slot list: a left-aligned slot label and a
// right-aligned status (a saved timestamp, or "Empty"), with the whole row
// clickable as a single UiButton -- the same "the button IS the background"
// idiom UiBindSlotButton uses for its mainButton, just stretched to a
// full-width row instead of a square icon slot.
//
// `interactive=false` (used for the quicksave/autosave rows on the *Save*
// screen -- see UiSaveLoadMenu::RefreshList) disables the hit target
// entirely and dims the row, rather than wiring onActivate to a no-op, so a
// reserved slot can never end up focused or hovered as if it were usable.
// ---------------------------------------------------------------------------
class UiSaveSlotRow : public UiButton
{
public:
    vec2 RowSize = vec2(450.f, SettingsStyle::RowHeight);

    std::function<void()> onActivate = nullptr;

    UiSaveSlotRow()
    {
        // Configure the button itself
        origin = vec2(0.f);
        pivot = vec2(0.f);
        ImagePath = "GameData/textures/generic/white.png";
        onClick = [this]() { if (onActivate) onActivate(); };

        label = std::make_shared<UiText>();
        label->fontSize = SettingsStyle::RowLabelSize;
        label->pivot = vec2(0.f, 0.5f);
        label->origin = vec2(0.f, 0.5f);
        label->position = vec2(SettingsStyle::PanelPadding * 0.6f, 0.f);
        label->fontSize = 45;
        AddChild(label);

        status = std::make_shared<UiText>();
        status->fontSize = SettingsStyle::CaptionSize;
        status->textColor = SettingsStyle::CaptionColor;
        status->pivot = vec2(1.f, 0.5f);
        status->origin = vec2(1.f, 0.5f);
        status->position = vec2(-SettingsStyle::PanelPadding * 0.6f, 0.f);
        status->fontSize = 36;
        AddChild(status);
    }

    // exists      -- whether this slot currently has a save on disk. Dims
    //                the label a little even before the status text is read,
    //                so empty manual slots visually read as "empty" at a
    //                glance.
    // alternate   -- zebra striping, same idea as UiInputSettings' RowAlt/RowBase.
    // interactive -- false for reserved (quicksave/autosave) rows on the Save
    //                screen: disables the hit target and dims things further.
    void SetContent(const std::string& labelText, const std::string& statusText,
        bool exists, bool alternate, bool interactive)
    {
        label->text = labelText;
        status->text = statusText;

        const float shade = alternate ? 0.03f : 0.f;

        if (!interactive)
        {
            Color = vec4(0.09f + shade, 0.09f + shade, 0.10f + shade, 1.f);
            HoverColor = Color; // no hover feedback -- it isn't clickable
            label->textColor = vec4(1.f, 1.f, 1.f, 0.45f);
        }
        else if (exists)
        {
            Color = vec4(0.16f + shade, 0.16f + shade, 0.18f + shade, 1.f);
            HoverColor = vec4(0.28f, 0.32f, 0.42f, 1.f);
            label->textColor = vec4(1.f);
        }
        else
        {
            Color = vec4(0.11f + shade, 0.11f + shade, 0.12f + shade, 1.f);
            HoverColor = vec4(0.24f, 0.28f, 0.36f, 1.f);
            label->textColor = vec4(1.f, 1.f, 1.f, 0.7f);
        }

        HitCheck = interactive;
        DisableFocus = !interactive;
    }

    void Update() override
    {
        size = RowSize;
        UiButton::Update();
    }

    glm::vec2 GetSize() override { return RowSize; }

private:
    std::shared_ptr<UiText> label;
    std::shared_ptr<UiText> status;
};