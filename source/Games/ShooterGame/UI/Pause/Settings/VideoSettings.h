#pragma once

#include <UI/RmlUi/UiModel.h>

#include <SDL2/SDL.h>

#include <UI/RmlUi/RmlUiEvents.h>

class VideoSettingsModel : public UIModel
{
public:
    std::vector<Rml::String> resolutions;

    Rml::String selected_resolution;
    Rml::String window_mode;

    int msaa = 0;
    bool fxaa_enabled = false;

protected:
    bool Bind(Rml::DataModelConstructor& constructor) override
    {
        UI_BIND_ARRAY(std::vector<Rml::String>);

        UI_BIND(resolutions);
        UI_BIND(selected_resolution);
        UI_BIND(window_mode);
        UI_BIND(msaa);
        UI_BIND(fxaa_enabled);

        return true;
    }
};

namespace VideoSettings
{

    static std::string GetResolutionString(int w, int h)
    {
        return std::to_string(w) + "x" + std::to_string(h);
    }

    inline void InitModelData(VideoSettingsModel& model)
    {

		auto window = EngineMain::MainInstance->Window;

        model.resolutions.clear();

        // --- Get current display ---
        int displayIndex = SDL_GetWindowDisplayIndex(window);
        if (displayIndex < 0)
            displayIndex = 0;

        // --- Enumerate display modes ---
        int modeCount = SDL_GetNumDisplayModes(displayIndex);

        std::set<std::string> uniqueResolutions;

        for (int i = 0; i < modeCount; ++i)
        {
            SDL_DisplayMode mode;
            if (SDL_GetDisplayMode(displayIndex, i, &mode) != 0)
                continue;

            std::string res = GetResolutionString(mode.w, mode.h);

            // Avoid duplicates (very common in SDL)
            if (uniqueResolutions.insert(res).second)
            {
                model.resolutions.push_back(res.c_str());
            }
        }

        std::sort(model.resolutions.begin(), model.resolutions.end(),
            [](const Rml::String& a, const Rml::String& b)
            {
                int aw, ah, bw, bh;
                sscanf(a.c_str(), "%dx%d", &aw, &ah);
                sscanf(b.c_str(), "%dx%d", &bw, &bh);
                return (aw * ah) < (bw * bh);
            });

        // --- Current resolution ---
        int w, h;
        
		w = EngineMain::MainInstance->ScreenSize.x;
		h = EngineMain::MainInstance->ScreenSize.y;

        model.selected_resolution = GetResolutionString(w, h).c_str();

        // --- Window mode ---
        Uint32 flags = SDL_GetWindowFlags(window);

        if (flags & SDL_WINDOW_FULLSCREEN_DESKTOP)
            model.window_mode = "fullscreen";
        else if (flags & SDL_WINDOW_FULLSCREEN)
            model.window_mode = "borderless";
        else
            model.window_mode = "windowed";

        // --- MSAA / FXAA (your engine) ---
        model.msaa = EngineMain::MainInstance->MainRenderer->MultiSampleCount;
        model.fxaa_enabled = EngineMain::MainInstance->MainRenderer->FXAAEnabled;

    }

    inline void BindVideoSettingsCallbacks(Rml::ElementDocument* doc)
    {

        using namespace RmlUiEvents;

        onChange(doc, "resolution", [](const std::string& value) {
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
            });

        // Window mode dropdown
        onChange(doc, "windowMode", [](const std::string& value) {
            SDL_Window* gWindow = EngineMain::MainInstance->Window;

            if (value == "fullscreen") {
                // Switch to true fullscreen (desktop resolution may change)
                SDL_SetWindowFullscreen(gWindow, SDL_WINDOW_FULLSCREEN);
            }
            else if (value == "borderless") {
                // Borderless fullscreen window (desktop resolution stays)
                SDL_SetWindowFullscreen(gWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);
            }
            else if (value == "windowed") {
                // Exit fullscreen
                SDL_SetWindowFullscreen(gWindow, 0);
                // Optionally restore windowed size
                // SDL_SetWindowSize(gWindow, previousWidth, previousHeight);
            }
            });

        // MSAA dropdown – value is string like "0", "2", "4", "8"
        onChange(doc, "msaa", [](const std::string& value) 
            {

                int msaaLevel = 0;

                try
                {
                    msaaLevel = std::stoi(value);   // Convert to integer
                }
                catch (const std::exception&)
                {

                }

				EngineMain::MainInstance->MainRenderer->MultiSampleCount = msaaLevel;
                
            });

        // FXAA checkbox – listens for change event, reads the checked state
        onEvent(doc, "fxaa", "change", [](Rml::Event& ev) {
            Rml::Element* elem = ev.GetTargetElement();
            auto* input = dynamic_cast<Rml::ElementFormControlInput*>(elem);
            if (!input) return;
            // Prefer IsChecked()
            bool enabled = input->GetAttribute("checked") != nullptr;
            // If IsChecked() doesn't exist, try:
            // bool enabled = input->GetAttribute<bool>("checked", false);
            EngineMain::MainInstance->MainRenderer->FXAAEnabled = enabled;
            });

    }


}