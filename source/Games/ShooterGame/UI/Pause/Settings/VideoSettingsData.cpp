#include "VideoSettingsData.h"

#include <SDL2/SDL.h>
#include "../EngineMain.h"

void VideoSettingsData::ApplyToEngine() const
{
    SDL_Window* gWindow = EngineMain::MainInstance->Window;
    if (!gWindow) return;

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

    // VSync: this project renders through bgfx rather than a raw GL context,
    // so the swap-interval call belongs wherever bgfx::reset()/BGFX_RESET_VSYNC
    // is issued (e.g. EngineMain's render setup). Hook it up there — the flag
    // is stored and persisted here either way.
}
