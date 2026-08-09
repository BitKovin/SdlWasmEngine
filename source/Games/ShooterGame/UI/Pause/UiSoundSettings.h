#pragma once

#include <UI/UiButton.hpp>
#include <UI/UiVerticalBox.hpp>
#include <UI/UiHorizontalBox.hpp>
#include <UI/UiText.hpp>
#include <UI/UiImage.hpp>
#include <UI/UiCanvas.hpp>
#include <UI/UiSlider.hpp>
#include <UI/UiTextBox.hpp>

#include <PauseGameManager.hpp>

// SoundManager isn't part of this handoff, so this include is a guess based
// on the PauseGameManager.hpp convention (manager classes living at the
// include root, not under Settings/ like VideoSettings/GameSettings). Point
// this at wherever SoundManager actually lives if it differs.
#include <SoundSystem/SoundManager.hpp>

#include "UiSettingsStyle.hpp"

#include <sstream>
#include <iomanip>
#include <algorithm>
#include <functional>

// ---------------------------------------------------------------------------
// UiSoundSettings
//
// Same shape as UiVideoSettings/UiInputSettings (UiCanvas, FocusTrap, scrim,
// title, card panel, backButton wired to onClick + OnNavCancel).
//
// Unlike Input (GameSettings::Instance().Input) or Video (a VideoSettingsModel
// read once at construction), there's no settings-model indirection here --
// per the given API this screen reads and writes SoundManager::MasterVolume
// / SfxVolume / MusicVolume directly, the same way UiPauseMenu calls
// PauseGameManager::SetGamePaused directly rather than going through a
// settings object. Writing the static field *is* the live state, so there's
// no separate ApplyToEngine step to call here, and (per the same reasoning)
// no SaveToFile call either -- add one at the writeValue callback in
// BuildVolumeRow if SoundManager turns out to need an explicit persist step.
//
// Each row is a slider + numeric textbox kept in sync, same idiom as
// UiInputSettings::BuildSensitivitySection (mouseSensSlider/mouseSensText),
// just parameterized once (BuildVolumeRow) instead of written out per-row,
// since all three rows are identical in shape.
// ---------------------------------------------------------------------------

class UiSoundSettings : public UiCanvas
{
public:

    std::shared_ptr<UiElement> parentMenu;

    UiSoundSettings(std::shared_ptr<UiElement> parentMenu_)
    {
        FocusTrap = true;
        this->parentMenu = parentMenu_;

        background = std::make_shared<UiImage>();
        background->color = SettingsStyle::Scrim;
        AddChild(background);

        rootBox = std::make_shared<UiVerticalBox>();
        rootBox->origin = vec2(0.5f);
        rootBox->pivot = vec2(0.5f);
        rootBox->ContentDistance = 20.f;

        titleText = std::make_shared<UiText>();
        titleText->text = "${SETTINGS_SOUND_TITLE}";
        titleText->fontSize = SettingsStyle::TitleSize;
        titleText->pivot = vec2(0.5f, 0.f);
        titleText->origin = vec2(0.5f, 0.f);

        BuildVolumeSection();

        buttonsRow = std::make_shared<UiHorizontalBox>();
        buttonsRow->origin = vec2(0.5f, 0);
        buttonsRow->pivot = vec2(0.5f, 0);

        backButton = MakeButton("${SETTINGS_BACK}");
        buttonsRow->AddChild(backButton);

        rootBox->AddChild(titleText);
        rootBox->AddChild(volumePanel);
        rootBox->AddChild(MakeDivider(SettingsStyle::ContentWidth * 0.6f, vec4(1.f, 1.f, 1.f, 0.10f)));
        rootBox->AddChild(buttonsRow);

        AddChild(rootBox);

        backButton->onClick = [this]()
        {
            parentMenu->visible = true;
            RemoveFromParent();
        };

        UpdateChildrenOffsetRecursive();
        UpdateChildrenOffsetRecursive();
    }

    void OnNavCancel() override { backButton->onClick(); }

    void FinalizeChildren() override
    {
        background->size = GetSize();
        UiCanvas::FinalizeChildren();
    }

private:

    std::shared_ptr<UiImage> background;
    std::shared_ptr<UiVerticalBox> rootBox;
    std::shared_ptr<UiText> titleText;

    // ── Volume ───────────────────────────────────────────────────────────────
    std::shared_ptr<UiElement> volumePanel;
    std::shared_ptr<UiSlider> masterSlider;
    std::shared_ptr<UiTextBox> masterText;
    std::shared_ptr<UiSlider> sfxSlider;
    std::shared_ptr<UiTextBox> sfxText;
    std::shared_ptr<UiSlider> musicSlider;
    std::shared_ptr<UiTextBox> musicText;

    std::shared_ptr<UiHorizontalBox> buttonsRow;
    std::shared_ptr<UiButton> backButton;

    // SoundManager::MasterVolume/SfxVolume/MusicVolume weren't given a
    // documented range -- 0..1 matches the min/max shape every other
    // settings screen in this codebase uses (see
    // InputSensitivitySettings::MinMouseSensitivity etc.). Adjust here if
    // SoundManager actually expects a different range.
    static constexpr float MinVolume = 0.f;
    static constexpr float MaxVolume = 1.f;

    // ───────────────────────────────────────────────────────────────────────
    // Volume section
    // ───────────────────────────────────────────────────────────────────────

    void BuildVolumeSection()
    {
        using namespace SettingsStyle;

        auto content = std::make_shared<UiVerticalBox>();
        content->ContentDistance = 14.f;

        auto header = std::make_shared<UiText>();
        header->text = "${SETTINGS_SOUND_VOLUME_HEADER}";
        header->fontSize = PanelHeaderSize;
        header->pivot = vec2(0.f, 0.0f);
        header->origin = vec2(0.f, 0.0f);
        content->AddChild(header);
        content->AddChild(MakeDivider(ContentWidth - PanelPadding * 2.f));

        content->AddChild(BuildVolumeRow("${SETTINGS_SOUND_MASTER_VOLUME}", SoundManager::GlobalVolume,
            masterSlider, masterText,
            [](float v) { SoundManager::GlobalVolume = v; }));

        content->AddChild(BuildVolumeRow("${SETTINGS_SOUND_SFX_VOLUME}", SoundManager::SfxVolume,
            sfxSlider, sfxText,
            [](float v) { SoundManager::SfxVolume = v; }));

        content->AddChild(BuildVolumeRow("${SETTINGS_SOUND_MUSIC_VOLUME}", SoundManager::MusicVolume,
            musicSlider, musicText,
            [](float v) { SoundManager::MusicVolume = v; }));

        volumePanel = std::make_shared<UiCardPanel>(vec2(ContentWidth, 280.f), content);
    }

    std::shared_ptr<UiHorizontalBox> BuildVolumeRow(const std::string& label, float initialValue,
        std::shared_ptr<UiSlider>& sliderOut, std::shared_ptr<UiTextBox>& textOut,
        std::function<void(float)> writeValue)
    {
        auto labelCell = std::make_shared<UiElement>();
        labelCell->size = vec2(320.f, 44.f);

        auto txt = std::make_shared<UiText>();
        txt->text = label;
        txt->fontSize = SettingsStyle::RowLabelSize;
        txt->pivot = vec2(0.f, 0.5f);
        txt->origin = vec2(0.f, 0.5f);
        labelCell->AddChild(txt);

        auto slider = std::make_shared<UiSlider>();
        slider->MinValue = MinVolume;
        slider->MaxValue = MaxVolume;
        slider->Value = initialValue;
        slider->Step = 0.01f;
        slider->size = vec2(360.f, 28.f);
        sliderOut = slider;

        auto textBox = std::make_shared<UiTextBox>();
        textBox->NumericOnly = true;
        textBox->SetText(FormatFloat(initialValue));
        textBox->size = vec2(90.f, 40.f);
        textOut = textBox;

        slider->onValueChanged = [textBox, writeValue](float v)
        {
            writeValue(v);
            textBox->SetText(FormatFloat(v));
        };

        std::shared_ptr<UiSlider> sliderCapture = slider;
        textBox->onSubmit = [sliderCapture, textBox, writeValue](const std::string& text)
        {
            float v = ParseFloatClamped(text, MinVolume, MaxVolume);
            writeValue(v);
            sliderCapture->Value = v;
            textBox->SetText(FormatFloat(v));
        };

        auto row = std::make_shared<UiHorizontalBox>();
        row->ContentDistance = 14.f;
        row->AddChild(labelCell);
        row->AddChild(slider);
        row->AddChild(textBox);

        return row;
    }

    std::shared_ptr<UiButton> MakeButton(std::string text)
    {
        std::shared_ptr<UiButton> btn = std::make_shared<UiButton>();
        std::shared_ptr<UiText> txt = std::make_shared<UiText>();

        btn->size = vec2(400, 70);
        txt->text = text;
        txt->fontSize = SettingsStyle::ButtonLabelSize;
        txt->pivot = vec2(0.5f);
        txt->origin = vec2(0.5f);
        btn->AddChild(txt);

        return btn;
    }

    static std::string FormatFloat(float v)
    {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(2) << v;
        return ss.str();
    }

    static float ParseFloatClamped(const std::string& text, float minV, float maxV)
    {
        try { return std::clamp(std::stof(text), minV, maxV); }
        catch (...) { return minV; }
    }
};
