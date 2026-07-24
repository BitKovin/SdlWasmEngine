#define _HAS_STD_BYTE 0
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#define SDL_VIDEO_DRIVER_X11
#define SDL_VIDEO_DRIVER_WAYLAND
#include <SDL2/SDL_syswm.h>
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_bgfx.h"
#include "../imgui/imgui_impl_sdl2.h"
#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <BgfxResetManager.h>
#include <deque>
#include <algorithm>
#include <array>

#ifdef __linux__

#include <unistd.h>

#endif

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

    // bgfx + SDL2 backend (nullptr context because we no longer use OpenGL)
    ImGui_ImplSDL2_InitForOpenGL(window, nullptr);
    ImGui_Implbgfx_Init(255);   // 255 = standard ImGui view ID (drawn on top)
}

void desktop_render_loop() {
    SDL_Event event;
    int quit = 0;
    int currentWidth = 800, currentHeight = 600;

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
                BgfxResetManager::SetResolution({ currentWidth, currentHeight });
            }

            Input::ReceiveSdlEvent(event);
        }

        engine->MainLoop();
    }
}

int main(int argc, char* args[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) < 0) {
        fprintf(stderr, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    int flags = SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE;
    window = SDL_CreateWindow("Image", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        800, 600, flags);
    if (!window) {
        fprintf(stderr, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    std::vector<std::string> args_s(args, args + argc);
    auto args_m = EngineMain::ParseCommands(args_s);
    for (auto a : args_m)
    {
        Logger::Log(a.first);
        for (auto o : a.second)
        {
            Logger::Log("arg: " + o);
        }
    }


    auto workingDirOverride = args_m.find("working_dir");
    if (workingDirOverride != args_m.end())
    {
        Logger::Log("switching working directory to " + workingDirOverride->second[0]);


#ifdef __linux__

        chdir(workingDirOverride->second[0].c_str());

#endif


    }

    bgfx::RendererType::Enum renderApi = bgfx::RendererType::OpenGL;

    auto renderApiOverride = args_m.find("renderapi");
    if (renderApiOverride != args_m.end())
    {
        const std::string& renderApiStr = renderApiOverride->second[0];
        if (renderApiStr == "dx11")
        {
            renderApi = bgfx::RendererType::Direct3D11;
            Logger::Log("Using Direct3D11 renderer");
        }
        else if (renderApiStr == "dx12")
        {
            renderApi = bgfx::RendererType::Direct3D12;
            Logger::Log("Using Direct3D12 renderer");
        }
        else if (renderApiStr == "vk")
        {
            renderApi = bgfx::RendererType::Vulkan;
            Logger::Log("Using Vulkan renderer");
        }
        else if (renderApiStr == "gl")
        {
            renderApi = bgfx::RendererType::OpenGL;
            Logger::Log("Using OpenGL renderer");
        }
        else
        {
            Logger::Log("Unknown render API specified: " + renderApiStr + ", using default");
        }
    }

    // ====================== BGFX INITIALIZATION ======================
    bgfx::Init init;
    init.type = bgfx::RendererType::Vulkan;   // or Vulkan/Metal; OpenGL is safe on Linux
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
#if defined(SDL_VIDEO_DRIVER_X11)
    if (wmInfo.subsystem == SDL_SYSWM_X11) {
        init.platformData.nwh = (void*)wmInfo.info.x11.window;
        init.platformData.ndt = wmInfo.info.x11.display;   // also needed on X11
        handleSet = true;
    }
#endif
#if defined(SDL_VIDEO_DRIVER_WAYLAND)
    if (wmInfo.subsystem == SDL_SYSWM_WAYLAND) {
        init.platformData.nwh = wmInfo.info.wl.surface;
        init.platformData.ndt = wmInfo.info.wl.display;
        handleSet = true;
    }
#endif
  printf("SDL subsystem: %d\n", wmInfo.subsystem);
    if (!handleSet) {
        fprintf(stderr, "Unsupported or unrecognized SDL video subsystem\n");
        return 1;
    }

    init.resolution.width = 800;
    init.resolution.height = 600;
    init.resolution.reset = BGFX_RESET_NONE;   // no vsync

    if (!bgfx::init(init)) {
        fprintf(stderr, "bgfx::init failed!\n");
        return 1;
    }

    // Default clear + views
    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x000000ff, 1.0f, 0);
    bgfx::setViewRect(0, 0, 0, 800, 600);
    bgfx::setViewRect(255, 0, 0, 800, 600);

    InitImGui();

    SDL_SetHintWithPriority(SDL_HINT_MOUSE_RELATIVE_MODE_CENTER, "1", SDL_HINT_OVERRIDE);
    SDL_SetRelativeMouseMode(SDL_TRUE);

    printf("bgfx initialized successfully.\n");

    Input::AddAction("fullscreen")->AddKeyboardKey(SDL_GetScancodeFromKey(SDLK_F11));

    engine = new EngineMain(window);
    EngineMain::MainInstance = engine;
    engine->Init();

    desktop_render_loop();

    delete engine;

    bgfx::frame();
    bgfx::shutdown();

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
