#if __EMSCRIPTEN__

#define DISTRIBUTION

#define _HAS_STD_BYTE 0
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <emscripten.h>
#include <emscripten/html5.h>

#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_bgfx.h"
#include "../imgui/imgui_impl_sdl2.h"

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>          // ← REQUIRED for PlatformData on Emscripten

#include <BgfxResetManager.h>

#include <deque>
#include <algorithm>
#include <array>

#include "../EngineMain.h"
#include "PlatformWindowData.h"

#include <FileSystem/EmscriptenFileSystem.h>

using namespace PlatformWindowData;

// ------------------------------------------------------------
// Globals
// ------------------------------------------------------------
EngineMain* engine = nullptr;
std::deque<vec2> delta_history;
const size_t history_size = 3;

ivec2 initial_screen_size = ivec2(800, 600);

// ------------------------------------------------------------
void update_screen_size(int w, int h)
{
    SDL_SetWindowSize(window, w, h);
}

void InitImGui()
{
     
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForOpenGL(window, nullptr);
    ImGui_Implbgfx_Init(255);
}

// ------------------------------------------------------------
void emscripten_render_loop()
{
    Input::PendingMouseDelta = vec2(0, 0);

    SDL_Event event;
    Input::StartEventsFrame();

    vec2 screenSizeDifferenceFactor =
        vec2(
            (float)EngineMain::MainInstance->ScreenSize.x / initial_screen_size.x,
            (float)EngineMain::MainInstance->ScreenSize.y / initial_screen_size.y
        );

    while (SDL_PollEvent(&event))
    {
        if (EngineMain::MainInstance->DebugUiEnabled)
            ImGui_ImplSDL2_ProcessEvent(&event);

        switch (event.type)
        {
        case SDL_WINDOWEVENT:
        {
            if (event.window.event == SDL_WINDOWEVENT_RESIZED ||
                event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
            {
                int w, h;
                SDL_GetWindowSize(window, &w, &h);

                BgfxResetManager::SetResolution({ w,h });

                update_screen_size(w, h);
            }
            break;
        }

        case SDL_MOUSEMOTION:
        {
            event.motion.x =
                (int)((float)event.motion.x * screenSizeDifferenceFactor.x);
            event.motion.y =
                (int)((float)event.motion.y * screenSizeDifferenceFactor.y);

            Input::PendingMouseDelta +=
                vec2(event.motion.xrel, event.motion.yrel) *
                screenSizeDifferenceFactor;

            break;
        }

        default:
            break;
        }

        Input::ReceiveSdlEvent(event);
    }

    engine->MainLoop();

    bgfx::frame();
}

// ------------------------------------------------------------
// Main
// ------------------------------------------------------------
int main(int argc, char* args[])
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) < 0)
    {
        fprintf(stderr, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    int flags = SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_OPENGL;

    window = SDL_CreateWindow(
        "Image",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        800,
        600,
        flags
    );

    if (!window)
    {
        fprintf(stderr, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }


    // ------------------------------------------------------------
    // BGFX INIT (fixed for Emscripten)
    // ------------------------------------------------------------
    int w, h;
    SDL_GetWindowSize(window, &w, &h);

    // === EMSCRIPTEN PLATFORM DATA (this is what was missing) ===
    bgfx::PlatformData platformData{};
    platformData.nwh = (void*)"#canvas";   // Emscripten's default canvas ID when using SDL2 + USE_SDL=2
    platformData.context = nullptr;
    platformData.backBuffer = nullptr;
    platformData.backBufferDS = nullptr;

    // === INIT ===
    bgfx::Init init;
    init.type = bgfx::RendererType::OpenGL;   // WebGL2 (matches your existing -s USE_WEBGL2=1)
    // init.type            = bgfx::RendererType::Count;   // you can also use this for auto-select
    init.debug = true;
    init.resolution.width = w;
    init.resolution.height = h;
    init.resolution.reset = BGFX_RESET_NONE;
    init.platformData = platformData;                 // ← THIS MAKES IT WORK ON WEB

    if (!bgfx::init(init))
    {
        printf("bgfx init failed\n");
        return 1;
    }

    printf("bgfx initialized successfully! Renderer type: %d\n", (int)bgfx::getRendererType());

    bgfx::setViewClear(
        0,
        BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH,
        0x000000ff,
        1.0f,
        0
    );

    bgfx::setViewRect(0, 0, 0, w, h);
    bgfx::setViewRect(255, 0, 0, w, h);

    // ------------------------------------------------------------

    InitImGui();

    SDL_SetHintWithPriority(
        SDL_HINT_MOUSE_RELATIVE_MODE_CENTER,
        "1",
        SDL_HINT_OVERRIDE
    );

    SDL_SetRelativeMouseMode(SDL_TRUE);

    engine = new EngineMain(window);

#ifndef __EMSCRIPTEN_PTHREADS__
    engine->asyncGameUpdate = false;
#endif

    EngineMain::MainInstance = engine;

	engine->FileSystem = std::make_shared<EmscriptenFileSystem>();

    SDL_GetWindowSize(window, &w, &h);
    initial_screen_size = ivec2(w, h);

#ifdef emscripten_sleep
    emscripten_sleep(300);
#endif

    engine->Init();

    Input::AddAction("fullscreen")
        ->AddKeyboardKey(SDL_GetScancodeFromKey(SDLK_F11));

#ifdef emscripten_sleep
    emscripten_sleep(300);
#endif

    emscripten_set_main_loop(emscripten_render_loop, 0, 1);

    delete engine;

    bgfx::frame();
    //bgfx::shutdown();

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

#endif