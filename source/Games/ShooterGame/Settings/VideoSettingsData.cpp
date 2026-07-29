#include "VideoSettingsData.h"

#include <SDL2/SDL.h>
#include <EngineMain.h>
#include <BgfxResetManager.h>

void VideoSettingsData::ApplyToEngine() const
{
    SDL_Window* gWindow = EngineMain::MainInstance->Window;
    if (!gWindow) return;

#if !defined(__EMSCRIPTEN__) && !defined(__ANDROID__)




    // Window mode first, same three-way switch UiVideoSettings::UpdateWindowMode uses.
    if (WindowMode == "fullscreen")
        SDL_SetWindowFullscreen(gWindow, SDL_WINDOW_FULLSCREEN);
    else if (WindowMode == "borderless")
        SDL_SetWindowFullscreen(gWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);
    else
        SDL_SetWindowFullscreen(gWindow, 0);

    Uint32 flags = SDL_GetWindowFlags(gWindow);
    if (flags & SDL_WINDOW_FULLSCREEN)
    {
        SDL_DisplayMode mode;
        mode.format = SDL_PIXELFORMAT_UNKNOWN;
        mode.w = Width;
        mode.h = Height;
        mode.refresh_rate = 0;
        mode.driverdata = nullptr;
        SDL_SetWindowDisplayMode(gWindow, &mode);
    }
    else
    {
        SDL_SetWindowSize(gWindow, Width, Height);
    }

#endif // !__EMSCRIPTEN__ ! __ANDROID__

    if (EngineMain::MainInstance->MainRenderer)
    {
        EngineMain::MainInstance->MainRenderer->MultiSampleCount = MSAA;
        EngineMain::MainInstance->MainRenderer->FXAAEnabled = FXAA;
    }

    BgfxResetManager::SetVSync(VSync);

    EngineMain::MainInstance->MainRenderer->DynamicShadows = DynamicShadows;

}

void VideoSettingsData::FromCurrentState()
{
    SDL_Window* gWindow = EngineMain::MainInstance->Window;
    if (!gWindow) return;

    // Resolution: same source UiVideoSettings/VideoSettings::InitModelData
    // treat as the authoritative "current resolution" -- not the raw SDL
    // window size, which can lag behind in exclusive fullscreen.
    Width = EngineMain::MainInstance->ScreenSize.x;
    Height = EngineMain::MainInstance->ScreenSize.y;

    // Window mode: FULLSCREEN_DESKTOP's bits are a superset of plain
    // FULLSCREEN, so it must be checked with an exact match first, or
    // borderless windows get misread as exclusive fullscreen. Same order
    // UiVideoSettings uses to pick the dropdown's starting index.
    Uint32 flags = SDL_GetWindowFlags(gWindow);
    if ((flags & SDL_WINDOW_FULLSCREEN_DESKTOP) == SDL_WINDOW_FULLSCREEN_DESKTOP)
        WindowMode = "borderless";
    else if (flags & SDL_WINDOW_FULLSCREEN)
        WindowMode = "fullscreen";
    else
        WindowMode = "windowed";

    if (EngineMain::MainInstance->MainRenderer)
    {
        MSAA = EngineMain::MainInstance->MainRenderer->MultiSampleCount;
        FXAA = EngineMain::MainInstance->MainRenderer->FXAAEnabled;
    }

    VSync = BgfxResetManager::GetVSync();

    DynamicShadows = EngineMain::MainInstance->MainRenderer->DynamicShadows;

}