#pragma once

#include <UI/UiButton.hpp>
#include <UI/UiVerticalBox.hpp>
#include <UI/UiHorizontalBox.hpp>
#include <UI/UiText.hpp>
#include <UI/UiImage.hpp>
#include <UI/UiCanvas.hpp>
#include <UI/UiDropdown.hpp>

#include <PauseGameManager.hpp>

#include <Settings/VideoSettings.h>

#include <Settings/GameSettings.h>

#include "UiSettingsStyle.hpp"

#include <algorithm>

// ---------------------------------------------------------------------------
// UiVideoSettings
//
// Same shape as UiInputSettings: UiCanvas, FocusTrap, a backButton wired to
// both onClick and OnNavCancel, content built from UiSettingsStyle's shared
// card-panel/divider/type-scale language.
//
// Two cards:
//   - Display: Resolution, Window Mode, VSync
//   - Graphics: MSAA, FXAA, Dynamic Shadows
//
// Every control updates its corresponding property in GameSettings::Instance().Video
// and then immediately triggers ApplyToEngine().
// ---------------------------------------------------------------------------

class UiVideoSettings : public UiCanvas
{
public:

    std::shared_ptr<UiElement> parentMenu;

    UiVideoSettings(std::shared_ptr<UiElement> parentMenu_)
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
        titleText->text = "Video Settings";
        titleText->fontSize = SettingsStyle::TitleSize;
        titleText->pivot = vec2(0.5f, 0.f);
        titleText->origin = vec2(0.5f, 0.f);

        VideoSettingsModel model;
        VideoSettings::InitModelData(model);

        BuildDisplaySection(model);
        BuildGraphicsSection(model);

        buttonsRow = std::make_shared<UiHorizontalBox>();
        buttonsRow->origin = vec2(0.5f, 0);
        buttonsRow->pivot = vec2(0.5f, 0);

        backButton = MakeButton("Back");
        buttonsRow->AddChild(backButton);

        rootBox->AddChild(titleText);
        rootBox->AddChild(displayPanel);
        rootBox->AddChild(graphicsPanel);
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

    // ── Display ──────────────────────────────────────────────────────────────
    std::shared_ptr<UiElement> displayPanel;
    std::shared_ptr<UiDropdown> resolutions;
    std::shared_ptr<UiDropdown> windowMode;
    std::shared_ptr<UiButton> vsyncButton;
    std::shared_ptr<UiImage> vsyncToggleBg;
    std::shared_ptr<UiText> vsyncLabel;

    // ── Graphics ─────────────────────────────────────────────────────────────
    std::shared_ptr<UiElement> graphicsPanel;
    std::shared_ptr<UiDropdown> msaaDropdown;
    std::shared_ptr<UiButton> fxaaButton;
    std::shared_ptr<UiImage> fxaaToggleBg;
    std::shared_ptr<UiText> fxaaLabel;
    std::shared_ptr<UiButton> dynamicShadowsButton;
    std::shared_ptr<UiImage> dynamicShadowsToggleBg;
    std::shared_ptr<UiText> dynamicShadowsLabel;

    std::shared_ptr<UiHorizontalBox> buttonsRow;
    std::shared_ptr<UiButton> backButton;

    // ───────────────────────────────────────────────────────────────────────
    // Display section
    // ───────────────────────────────────────────────────────────────────────

    void BuildDisplaySection(VideoSettingsModel& model)
    {
        using namespace SettingsStyle;

        auto content = std::make_shared<UiVerticalBox>();
        content->ContentDistance = 14.f;

        auto header = std::make_shared<UiText>();
        header->text = "Display";
        header->fontSize = PanelHeaderSize;
        header->pivot = vec2(0.f, 0.0f);
        header->origin = vec2(0.f, 0.0f);
        content->AddChild(header);
        content->AddChild(MakeDivider(ContentWidth - PanelPadding * 2.f));

        if (std::find(model.resolutions.begin(), model.resolutions.end(), model.selected_resolution) == model.resolutions.end())
        {
            model.resolutions.push_back(model.selected_resolution);
        }

        int selectedIndex = std::distance(model.resolutions.begin(),
            std::find(model.resolutions.begin(), model.resolutions.end(), model.selected_resolution));

        resolutions = std::make_shared<UiDropdown>();
        resolutions->SetOptions(model.resolutions);
        resolutions->SetSelectedIndex(selectedIndex);
        resolutions->size = vec2(360.f, 56.f);
        resolutions->onSelectionChanged = UiVideoSettings::UpdateResolution;
        content->AddChild(GetSettingRow("Resolution", resolutions));

        windowMode = std::make_shared<UiDropdown>();
        windowMode->SetOptions({ "windowed", "fullscreen", "borderless" });

        int windowOption = 0;
        Uint32 flags = SDL_GetWindowFlags(EngineMain::MainInstance->Window);
        if ((flags & SDL_WINDOW_FULLSCREEN_DESKTOP) == SDL_WINDOW_FULLSCREEN_DESKTOP)
            windowOption = 2; // borderless -- requires ALL its bits to match
        else if (flags & SDL_WINDOW_FULLSCREEN)
            windowOption = 1; // true exclusive fullscreen

        windowMode->SetSelectedIndex(windowOption);
        windowMode->size = vec2(360.f, 56.f);
        windowMode->onSelectionChanged = UiVideoSettings::UpdateWindowMode;
        content->AddChild(GetSettingRow("Window Mode", windowMode));

        // ── VSync ─────────────────────────────────────────────────────────────
        vsyncButton = std::make_shared<UiButton>();
        vsyncButton->size = vec2(120.f, 40.f);

        vsyncToggleBg = std::make_shared<UiImage>();
        vsyncToggleBg->size = vsyncButton->size;
        vsyncButton->AddChild(vsyncToggleBg);

        vsyncLabel = std::make_shared<UiText>();
        vsyncLabel->fontSize = 24.f;
        vsyncLabel->pivot = vec2(0.5f);
        vsyncLabel->origin = vec2(0.5f);
        vsyncButton->AddChild(vsyncLabel);

        RefreshVSyncLabel(GameSettings::Instance().Video.VSync);

        vsyncButton->onClick = [this]()
            {
                bool enabled = !GameSettings::Instance().Video.VSync;
                GameSettings::Instance().Video.VSync = enabled;
                GameSettings::Instance().Video.ApplyToEngine();
                RefreshVSyncLabel(enabled);
            };
        content->AddChild(GetSettingRow("VSync", vsyncButton));

        displayPanel = std::make_shared<UiCardPanel>(vec2(ContentWidth, 320.f), content);
    }

    // ───────────────────────────────────────────────────────────────────────
    // Graphics section
    // ───────────────────────────────────────────────────────────────────────

    void BuildGraphicsSection(const VideoSettingsModel& model)
    {
        using namespace SettingsStyle;

        auto content = std::make_shared<UiVerticalBox>();
        content->ContentDistance = 14.f;

        auto header = std::make_shared<UiText>();
        header->text = "Graphics";
        header->fontSize = PanelHeaderSize;
        header->pivot = vec2(0.f, 0.0f);
        header->origin = vec2(0.f, 0.0f);
        content->AddChild(header);
        content->AddChild(MakeDivider(ContentWidth - PanelPadding * 2.f));

        // ── MSAA ─────────────────────────────────────────────────────────────
        std::vector<std::string> msaaOptions = { "0", "2", "4", "8" };
        std::string currentMsaa = std::to_string(model.msaa);
        if (std::find(msaaOptions.begin(), msaaOptions.end(), currentMsaa) == msaaOptions.end())
        {
            msaaOptions.push_back(currentMsaa);
        }
        int msaaIndex = std::distance(msaaOptions.begin(),
            std::find(msaaOptions.begin(), msaaOptions.end(), currentMsaa));

        msaaDropdown = std::make_shared<UiDropdown>();
        msaaDropdown->SetOptions(msaaOptions);
        msaaDropdown->SetSelectedIndex(msaaIndex);
        msaaDropdown->size = vec2(360.f, 56.f);
        msaaDropdown->onSelectionChanged = UiVideoSettings::UpdateMSAA;
        content->AddChild(GetSettingRow("MSAA", msaaDropdown));

        // ── FXAA ─────────────────────────────────────────────────────────────
        fxaaButton = std::make_shared<UiButton>();
        fxaaButton->size = vec2(120.f, 40.f);

        fxaaToggleBg = std::make_shared<UiImage>();
        fxaaToggleBg->size = fxaaButton->size;
        fxaaButton->AddChild(fxaaToggleBg);

        fxaaLabel = std::make_shared<UiText>();
        fxaaLabel->fontSize = 24.f;
        fxaaLabel->pivot = vec2(0.5f);
        fxaaLabel->origin = vec2(0.5f);
        fxaaButton->AddChild(fxaaLabel);
        RefreshFXAALabel(GameSettings::Instance().Video.FXAA);

        fxaaButton->onClick = [this]()
            {
                bool enabled = !GameSettings::Instance().Video.FXAA;
                GameSettings::Instance().Video.FXAA = enabled;
                GameSettings::Instance().Video.ApplyToEngine();
                RefreshFXAALabel(enabled);
            };

        content->AddChild(GetSettingRow("FXAA", fxaaButton));

        // ── Dynamic Shadows ──────────────────────────────────────────────────
        dynamicShadowsButton = std::make_shared<UiButton>();
        dynamicShadowsButton->size = vec2(120.f, 40.f);

        dynamicShadowsToggleBg = std::make_shared<UiImage>();
        dynamicShadowsToggleBg->size = dynamicShadowsButton->size;
        dynamicShadowsButton->AddChild(dynamicShadowsToggleBg);

        dynamicShadowsLabel = std::make_shared<UiText>();
        dynamicShadowsLabel->fontSize = 24.f;
        dynamicShadowsLabel->pivot = vec2(0.5f);
        dynamicShadowsLabel->origin = vec2(0.5f);
        dynamicShadowsButton->AddChild(dynamicShadowsLabel);
        RefreshDynamicShadowsLabel(GameSettings::Instance().Video.DynamicShadows);

        dynamicShadowsButton->onClick = [this]()
            {
                bool enabled = !GameSettings::Instance().Video.DynamicShadows;
                GameSettings::Instance().Video.DynamicShadows = enabled;
                GameSettings::Instance().Video.ApplyToEngine();
                RefreshDynamicShadowsLabel(enabled);
            };

        content->AddChild(GetSettingRow("Dynamic Shadows", dynamicShadowsButton));

        graphicsPanel = std::make_shared<UiCardPanel>(vec2(ContentWidth, 320.f), content);
    }

    void RefreshVSyncLabel(bool on)
    {
        vsyncLabel->text = on ? "On" : "Off";
        vsyncToggleBg->color = on ? SettingsStyle::ToggleOn : SettingsStyle::ToggleOff;
    }

    void RefreshFXAALabel(bool on)
    {
        fxaaLabel->text = on ? "On" : "Off";
        fxaaToggleBg->color = on ? SettingsStyle::ToggleOn : SettingsStyle::ToggleOff;
    }

    void RefreshDynamicShadowsLabel(bool on)
    {
        dynamicShadowsLabel->text = on ? "On" : "Off";
        dynamicShadowsToggleBg->color = on ? SettingsStyle::ToggleOn : SettingsStyle::ToggleOff;
    }

    std::shared_ptr<UiHorizontalBox> GetSettingRow(const std::string& label, std::shared_ptr<UiElement> control)
    {
        auto labelCell = std::make_shared<UiElement>();
        labelCell->size = vec2(320.f, 56.f);

        auto txt = std::make_shared<UiText>();
        txt->text = label;
        txt->fontSize = SettingsStyle::RowLabelSize;
        txt->pivot = vec2(0.f, 0.5f);
        txt->origin = vec2(0.f, 0.5f);
        labelCell->AddChild(txt);

        control->pivot = vec2(0.f, 0.5f);
        control->origin = vec2(0.f, 0.5f);

        auto row = std::make_shared<UiHorizontalBox>();
        row->ContentDistance = 14.f;
        row->AddChild(labelCell);
        row->AddChild(control);

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

    static inline void UpdateResolution(int index, const std::string& value)
    {
        size_t xPos = value.find('x');
        if (xPos == std::string::npos) return;

        int width = std::stoi(value.substr(0, xPos));
        int height = std::stoi(value.substr(xPos + 1));

        GameSettings::Instance().Video.Width = width;
        GameSettings::Instance().Video.Height = height;
        GameSettings::Instance().Video.ApplyToEngine();
    }

    static inline void UpdateWindowMode(int index, const std::string& value)
    {
        if (index == 0)
            GameSettings::Instance().Video.WindowMode = "windowed";
        else if (index == 1)
            GameSettings::Instance().Video.WindowMode = "fullscreen";
        else // index == 2
            GameSettings::Instance().Video.WindowMode = "borderless";

        GameSettings::Instance().Video.ApplyToEngine();
    }

    static inline void UpdateMSAA(int index, const std::string& value)
    {
        int msaaLevel = 0;
        try
        {
            msaaLevel = std::stoi(value);
        }
        catch (const std::exception&)
        {
        }

        GameSettings::Instance().Video.MSAA = msaaLevel;
        GameSettings::Instance().Video.ApplyToEngine();
    }

};