#pragma once

#pragma message ("Compiling GDK/Xbox main.")

#include <OnlineSubsystems/GDKAuth.h>
#include <FileSystem/GDKFileSystem.h>

#define _HAS_STD_BYTE 0
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_main.h>
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_bgfx.h"
#include "../imgui/imgui_impl_sdl2.h"
#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <deque>
#include <algorithm>
#include <array>
#include <direct.h>



#include <Logger.hpp>



#include "../EngineMain.h"
#include "PlatformWindowData.h"
using namespace PlatformWindowData;

EngineMain* engine = nullptr;

void update_screen_size(int w, int h) {
    SDL_SetWindowSize(window, w, h);
}

void InitImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    ImGui::StyleColorsDark();

    // bgfx + SDL2 backend
    ImGui_ImplSDL2_InitForOpenGL(window, nullptr);
    ImGui_Implbgfx_Init(255);   // 255 = standard ImGui view ID (drawn on top)
}

void desktop_render_loop() {
    SDL_Event event;
    int quit = 0;
    int currentWidth = 1920, currentHeight = 1080;

    while (!quit) {
        Input::PendingMouseDelta = vec2(0);
        Input::StartEventsFrame();

        while (SDL_PollEvent(&event)) {
            if (EngineMain::MainInstance->DebugUiEnabled)
                ImGui_ImplSDL2_ProcessEvent(&event);

            if (event.type == SDL_MOUSEMOTION) {
                Input::PendingMouseDelta += vec2(event.motion.xrel, event.motion.yrel);
            }

            if (event.type == SDL_QUIT) quit = 1;

            if (event.type == SDL_WINDOWEVENT &&
                (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED ||
                    event.window.event == SDL_WINDOWEVENT_RESIZED)) {
                SDL_GetWindowSize(window, &currentWidth, &currentHeight);
                bgfx::reset(currentWidth, currentHeight, BGFX_RESET_MAXANISOTROPY);
                bgfx::setViewRect(0, 0, 0, currentWidth, currentHeight);
                bgfx::setViewRect(255, 0, 0, currentWidth, currentHeight);
            }

            Input::ReceiveSdlEvent(event);
        }

        engine->MainLoop();
    }
}

void FixWorkingDirectory()
{
    char exePath[MAX_PATH];
    GetModuleFileNameA(nullptr, exePath, MAX_PATH);

    std::filesystem::path exeDir = std::filesystem::path(exePath).parent_path();

    _chdir(exeDir.string().c_str());
}

static constexpr const char* SCID = "00000000-0000-0000-0000-000071c4344d";

int main(int argc, char* args[]) 
{


    FixWorkingDirectory();

    // GDK often relies on game controllers as primary input
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) < 0) {
        fprintf(stderr, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    int flags = SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE;

    // GDK defaults usually target standard TV/Monitor resolutions natively
    window = SDL_CreateWindow("Engine GDK", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        800, 600, flags);
    if (!window) {
        fprintf(stderr, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    // ====================== BGFX INITIALIZATION ======================
    bgfx::Init init;
    // Direct3D12 is strongly recommended and sometimes strictly required for modern GDK/Xbox titles
    init.type = bgfx::RendererType::Direct3D12;
    init.debug = false;
    init.profile = false;

    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    if (!SDL_GetWindowWMInfo(window, &wmInfo)) {
        fprintf(stderr, "SDL_GetWindowWMInfo failed: %s\n", SDL_GetError());
        return 1;
    }

    // Set native window handle based on the available SDL video driver
    bool handleSet = false;

    // GDK targets map to standard Windows handles internally inside SDL
#if defined(SDL_VIDEO_DRIVER_WINDOWS) || defined(__GDK__)
    if (wmInfo.subsystem == SDL_SYSWM_WINDOWS || wmInfo.subsystem == SDL_SYSWM_WINRT) {
        init.platformData.nwh = (void*)wmInfo.info.win.window; // Native HWND
        handleSet = true;
    }
#endif

    printf("SDL subsystem: %d\n", wmInfo.subsystem);
    if (!handleSet) {
        fprintf(stderr, "Unsupported or unrecognized SDL video subsystem for GDK\n");
        return 1;
    }

    init.resolution.width = 1920;
    init.resolution.height = 1080;
    init.resolution.reset = BGFX_RESET_NONE; // No vsync by default

    if (!bgfx::init(init)) {
        fprintf(stderr, "bgfx::init failed!\n");
        return 1;
    }

    // Default clear + views
    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x000000ff, 1.0f, 0);
    bgfx::setViewRect(0, 0, 0, 1920, 1080);
    bgfx::setViewRect(255, 0, 0, 1920, 1080);

    InitImGui();

    SDL_SetHintWithPriority(SDL_HINT_MOUSE_RELATIVE_MODE_CENTER, "1", SDL_HINT_OVERRIDE);
    SDL_SetRelativeMouseMode(SDL_TRUE);

    Logger::Log("bgfx initialized successfully on GDK.\n");

    Input::AddAction("fullscreen")->AddKeyboardKey(SDL_GetScancodeFromKey(SDLK_F11));

    GDKSession session = GDKAuth_Init(SCID);

    if (!session.user)
    {
        Logger::Error("[Main] GDK Authentication failed");
        //return 1;
    }

    engine = new EngineMain(window);
    EngineMain::MainInstance = engine;

    //engine->FileSystem = std::make_shared<GDKFileSystem>(SCID);

    //engine->FileSystem->Init();

    engine->Init();

    desktop_render_loop();

    delete engine;

    bgfx::frame();
    bgfx::shutdown();

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}