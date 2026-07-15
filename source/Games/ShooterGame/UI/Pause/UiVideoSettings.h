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
// card-panel/divider/type-scale language. Two cards now instead of one --
// Display (Resolution, Window Mode) and Graphics (MSAA, FXAA) -- mirroring
// how UiInputSettings splits Sensitivity from Key Bindings rather than
// cramming everything into a single panel. See UiSettingsStyle.hpp for the
// shared constants/panel classes.
//
// Resolution + Window Mode keep their previous behavior (applied immediately
// via UpdateResolution/UpdateWindowMode, unchanged from before).
//
// MSAA + FXAA are real now, sourced from the actual Settings/VideoSettings.h
// this time (VideoSettingsModel::msaa / fxaa_enabled, already populated by
// the same VideoSettings::InitModelData(model) call this file already makes
// for resolutions) rather than the earlier "Off/FXAA/MSAA" single-dropdown
// guess, which conflated two independent settings into one and didn't match
// how the engine actually models them. They're two separate controls,
// matching VideoSettingsModel's shape:
//   - MSAA is an integer sample count (0/2/4/8), read from and written to
//     EngineMain::MainInstance->MainRenderer->MultiSampleCount -- a
//     dropdown, same as Resolution/Window Mode, options fixed to the four
//     values the real onChange("msaa", ...) handler parses.
//   - FXAA is a bool, read from and written to
//     EngineMain::MainInstance->MainRenderer->FXAAEnabled -- there's no
//     checkbox widget anywhere else in this codebase to model against, so
//     it uses the same On/Off toggle-button treatment UiInputSettings uses
//     for Invert Y (invertYButton/invertYToggleBg/invertYLabel there,
//     fxaaButton/fxaaToggleBg/fxaaLabel here). Functionally a checkbox
//     either way -- just this codebase's established look for one.
// Both apply immediately on change, same as every other control on this
// screen, and are also written into GameSettings::Instance().Video (MSAA,
// FXAA) alongside Resolution and Window Mode, so all four round-trip through
// Serialize()/Deserialize() the next time the game saves/loads settings.
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

    // ── Graphics ─────────────────────────────────────────────────────────────
    std::shared_ptr<UiElement> graphicsPanel;
    std::shared_ptr<UiDropdown> msaaDropdown;
    std::shared_ptr<UiButton> fxaaButton;
    std::shared_ptr<UiImage> fxaaToggleBg;
    std::shared_ptr<UiText> fxaaLabel;

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

        displayPanel = std::make_shared<UiCardPanel>(vec2(ContentWidth, 250.f), content);
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
        // Fixed to the four sample counts the real onChange("msaa", ...)
        // handler in Settings/VideoSettings.h parses ("0"/"2"/"4"/"8"). If
        // MultiSampleCount is currently something outside that set, it's
        // appended rather than silently snapped to the nearest option --
        // same defensive fallback Resolution already uses above.
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
        RefreshFXAALabel(model.fxaa_enabled);

        fxaaButton->onClick = [this]()
        {
            bool enabled = !EngineMain::MainInstance->MainRenderer->FXAAEnabled;
            EngineMain::MainInstance->MainRenderer->FXAAEnabled = enabled;
            GameSettings::Instance().Video.FXAA = enabled;
            RefreshFXAALabel(enabled);
        };

        content->AddChild(GetSettingRow("FXAA", fxaaButton));

        graphicsPanel = std::make_shared<UiCardPanel>(vec2(ContentWidth, 250.f), content);
    }

    void RefreshFXAALabel(bool on)
    {
        fxaaLabel->text = on ? "On" : "Off";
        fxaaToggleBg->color = on ? SettingsStyle::ToggleOn : SettingsStyle::ToggleOff;
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
        // Parse resolution string, expected format: "WIDTHxHEIGHT" (e.g., "1920x1080")
        size_t xPos = value.find('x');
        if (xPos == std::string::npos) return;

        int width = std::stoi(value.substr(0, xPos));
        int height = std::stoi(value.substr(xPos + 1));

        // Assuming gWindow is your SDL_Window* accessible globally
        SDL_Window* gWindow = EngineMain::MainInstance->Window;

        // For fullscreen modes, we need to change the display mode
        Uint32 flags = SDL_GetWindowFlags(gWindow);
        if (flags & SDL_WINDOW_FULLSCREEN) {
            // In fullscreen, we must change the display mode
            SDL_DisplayMode mode;
            mode.format = SDL_PIXELFORMAT_UNKNOWN; // let SDL choose
            mode.w = width;
            mode.h = height;
            mode.refresh_rate = 0; // use current refresh rate
            mode.driverdata = nullptr;

            SDL_SetWindowDisplayMode(gWindow, &mode);
            // Toggle fullscreen off and on to apply? Usually SetDisplayMode works immediately
            // But we can also just set the mode and it should apply
        }
        else {
            // Windowed mode: just set the size
            SDL_SetWindowSize(gWindow, width, height);
        }

        GameSettings::Instance().Video.Width = width;
        GameSettings::Instance().Video.Height = height;
    }

    static inline void UpdateWindowMode(int index, const std::string& value)
    {
        // Dropdown options are {"windowed"=0, "fullscreen"=1, "borderless"=2} --
        // the string stored here must match that same order, since it's what
        // VideoSettingsData::ApplyToEngine() reads back on next load. Previously
        // index 1/2 stored the opposite label from what the SDL call actually did.

        if (index == 0)
        {
            SDL_SetWindowFullscreen(EngineMain::MainInstance->Window, 0);
            GameSettings::Instance().Video.WindowMode = "windowed";
        }
        else if (index == 1)
        {
            SDL_SetWindowFullscreen(EngineMain::MainInstance->Window, SDL_WINDOW_FULLSCREEN);
            GameSettings::Instance().Video.WindowMode = "fullscreen";
        }
        else // index == 2
        {
            SDL_SetWindowFullscreen(EngineMain::MainInstance->Window, SDL_WINDOW_FULLSCREEN_DESKTOP);
            GameSettings::Instance().Video.WindowMode = "borderless";
        }
    }

    static inline void UpdateMSAA(int index, const std::string& value)
    {
        // Same parse-with-fallback-to-0 behavior as the real
        // onChange("msaa", ...) handler in Settings/VideoSettings.h.
        int msaaLevel = 0;
        try
        {
            msaaLevel = std::stoi(value);
        }
        catch (const std::exception&)
        {
        }

        EngineMain::MainInstance->MainRenderer->MultiSampleCount = msaaLevel;
        GameSettings::Instance().Video.MSAA = msaaLevel;
    }

};
