#define _HAS_STD_BYTE 0
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#define SDL_VIDEO_DRIVER_ANDROID
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
#include <SDL2/SDL_main.h>
#ifdef __ANDROID__
#include <unistd.h>
#include <android/log.h>
#endif
#include <FileSystem/AndroidFileSystem.h>
#include <SaveSystem/GameSaveSystem.h>
#include "../EngineMain.h"
#include "PlatformWindowData.h"
using namespace PlatformWindowData;

EngineMain* engine = nullptr;

// On Android the OS owns the window size (fullscreen), so this is mostly a
// no-op vs the desktop version, but kept for API parity with engine code
// that may call it (e.g. in response to a display mode change).
void update_screen_size(int w, int h) {
    SDL_SetWindowSize(window, w, h);
}

// ============================================================================
// ImGui lifecycle, split in two on purpose:
//
//   - "Platform" (SDL2 backend + ImGuiContext) is tied to the SDL_Window,
//     which is stable across Android background/foreground -- init once,
//     shut down once, at process start/end.
//   - "Renderer" (bgfx backend) is tied to bgfx's device, which is NOT
//     stable across background/foreground (see ShutdownBgfxGraphics /
//     InitBgfx below) -- it gets torn down and rebuilt every time bgfx does.
//
// Mixing these together was the original cause of the
// "Already initialized a platform backend!" assert: any code path that
// re-ran full ImGui init (e.g. as part of recovering from a lost surface)
// would call ImGui_ImplSDL2_InitForOpenGL a second time on a context that
// still had its old platform backend attached.
// ============================================================================
static bool s_imguiPlatformInitialized = false;

void InitImGuiPlatform() {
    if (s_imguiPlatformInitialized) return;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    // NOTE: no NavEnableKeyboard on Android by default -- there's usually no
    // physical keyboard. ImGui_ImplSDL2 will still translate touch input to
    // an emulated mouse pointer well enough for basic interaction.
    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForOpenGL(window, nullptr);
    s_imguiPlatformInitialized = true;
}

void ShutdownImGuiPlatform() {
    if (!s_imguiPlatformInitialized) return;
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    s_imguiPlatformInitialized = false;
}

// ============================================================================
// bgfx lifecycle.
//
// Android destroys the ANativeWindow/EGLSurface every time the app is
// backgrounded (this is standard SurfaceView behavior and can't be avoided
// while rendering through one), and bgfx has no supported way to point an
// already-running instance at a new native window: bgfx::setPlatformData
// only takes effect *before* bgfx::init. So "survive backgrounding" means
// a real shutdown+reinit cycle, not a resize.
// ============================================================================
static bool s_bgfxValid = false;
static bgfx::RendererType::Enum s_renderApi = bgfx::RendererType::OpenGLES;

bool InitBgfx(void* nwh, int width, int height) {

    bgfx::Init init;
    init.type = s_renderApi;
    init.debug = false;
    init.profile = false;
    init.platformData.nwh = nwh;
    init.resolution.width = width;
    init.resolution.height = height;
    init.resolution.reset = BGFX_RESET_NONE;   // no vsync

    if (!bgfx::init(init)) {
        fprintf(stderr, "bgfx::init failed!\n");
        return false;
    }

    bgfx::setDebug(BGFX_DEBUG_STATS);

    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x000000ff, 1.0f, 0);
    bgfx::setViewRect(0, 0, 0, width, height);
    bgfx::setViewRect(255, 0, 0, width, height);

    ImGui_Implbgfx_Init(255);   // 255 = standard ImGui view ID (drawn on top)

    s_bgfxValid = true;
    return true;
}

// Survives full engine teardown/recreation across a background/foreground
// cycle -- this is the *only* thing that's allowed to.
GameSaveData unfocusSaveGame {};

void ShutdownBgfxGraphics() {
    if (!s_bgfxValid) return;
    // One last synchronous frame to make sure nothing is left mid-flight
    // (safe now that bgfx is single-threaded: frame() runs inline instead
    // of handing off to a separate render thread that could still be
    // draining after we return from here).
    bgfx::frame();

    // Every bgfx handle still outstanding at this point becomes invalid
    // the instant bgfx::shutdown() returns.
    ImGui_Implbgfx_Shutdown();
    bgfx::shutdown();
    s_bgfxValid = false;
}

void CreateAndInitEngine();

// Attempts to (re)create bgfx against the current ANativeWindow. Safe to
// call speculatively -- it's a no-op if bgfx is already valid, and it's
// harmless if Android hasn't actually handed us a real window yet (init
// just fails and we try again on the next relevant event).
bool TryReinitBgfxIfNeeded() {
    if (s_bgfxValid) return true;

    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    if (!SDL_GetWindowWMInfo(window, &wmInfo) || wmInfo.subsystem != SDL_SYSWM_ANDROID) {
        return false;
    }

    int w = 0, h = 0;
    SDL_GetWindowSize(window, &w, &h);

    if (!InitBgfx(wmInfo.info.android.window, w, h)) {
        return false;
    }

    // bgfx is back: rebuild the engine from scratch, then restore the game
    // state captured in HandleEnterBackground(). Loading strictly after
    // Init() mirrors main()'s own startup order and guarantees every engine
    // subsystem exists before anything tries to populate it.
    CreateAndInitEngine();
    AssetRegistry::BeginLevelLoad();
    AssetRegistry::EndLevelLoad();
    

    return true;
}

// ============================================================================
// Engine lifecycle.
//
// The engine (and everything it owns: game state, gameplay systems, any
// renderer-facing objects it holds) does NOT survive backgrounding. It is
// fully destroyed in HandleEnterBackground() and fully rebuilt in
// TryReinitBgfxIfNeeded(). unfocusSaveGame is the one piece of state that
// bridges the gap.
// ============================================================================
void CreateAndInitEngine() {
    engine = new EngineMain(window);
    EngineMain::MainInstance = engine;
    EngineMain::MainInstance->FileSystem = std::make_shared<AndroidFileSystem>();
    engine->Init();
    engine->MainRenderer->MultiSampleCount = 0;
    engine->MainRenderer->ResolutionScale = 0.5f;
    engine->MainRenderer->FXAAEnabled = true;
    engine->pendingRestoreSaveData = unfocusSaveGame;
}

void DestroyEngine() {
    if (!engine) return;

    AssetRegistry::BeginLevelLoad();
    AssetRegistry::EndLevelLoad();
    AssetRegistry::ClearMemory();

    // Mirror of the note in CreateAndInitEngine(): if any system holds a
    // raw pointer into this instance (or into something it owns), clear/
    // unbind it here, before `delete`, e.g.:
    //   Input::UnbindEngine();
    delete engine;
    engine = nullptr;
    // Nothing else in the loop (see android_render_loop's event handling)
    // may dereference MainInstance while it's null -- that's enforced at
    // the call site, not here.
    EngineMain::MainInstance = nullptr;
}

// Called on SDL_APP_WILLENTERBACKGROUND. Captures game state, then tears
// down first the engine and then bgfx -- in that order, deliberately: the
// engine is deleted *while bgfx is still valid*, so if EngineMain's own
// destructor releases any bgfx resources it holds directly (as opposed to
// through AssetRegistry), it does so against a live graphics device instead
// of one that's already been shut down.
void HandleEnterBackground() {
    if (!engine || !s_bgfxValid) return;

    unfocusSaveGame = GameSaveSystem::SaveGameToData();

    DestroyEngine();

    // Tear bgfx down now, while the window handle we have is still valid,
    // instead of racing Android's destruction of the surface. This is what
    // actually prevents the eglSwapBuffers fatal -- not the paused flag by
    // itself.
    ShutdownBgfxGraphics();
}

// Renamed from desktop_render_loop: also handles Android app-lifecycle
// events so we don't keep rendering (and burning battery / touching a dead
// GL context) while backgrounded.
void android_render_loop() {
    SDL_Event event;
    int quit = 0;
    int paused = 0;
    int currentWidth = 0, currentHeight = 0;
    SDL_GetWindowSize(window, &currentWidth, &currentHeight);

    while (!quit) {
        Input::PendingMouseDelta = vec2(0);
        Input::StartEventsFrame();

        while (SDL_PollEvent(&event)) {
            // engine (and MainInstance) can be null while backgrounded --
            // events still get pumped during that window (this check
            // itself runs on every one of them), so guard it explicitly
            // rather than relying on paused/s_bgfxValid staying in sync.
            if (EngineMain::MainInstance && EngineMain::MainInstance->DebugUiEnabled)
                ImGui_ImplSDL2_ProcessEvent(&event);

            if (event.type == SDL_MOUSEMOTION) {
                Input::PendingMouseDelta += vec2(event.motion.xrel, event.motion.yrel);
            }

            if (event.type == SDL_QUIT) quit = 1;

            if (event.type == SDL_WINDOWEVENT &&
                (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED ||
                 event.window.event == SDL_WINDOWEVENT_RESIZED)) {
                int newWidth = 0, newHeight = 0;
                SDL_GetWindowSize(window, &newWidth, &newHeight);
                // Android fires a size-changed event on most
                // background -> foreground transitions -- the native
                // surface gets torn down and recreated even when its
                // dimensions don't change -- so only push a resolution
                // change through when something actually changed.
                if (s_bgfxValid && (newWidth != currentWidth || newHeight != currentHeight)) {
                    currentWidth = newWidth;
                    currentHeight = newHeight;
                    BgfxResetManager::SetResolution({ currentWidth, currentHeight });
                }
                // This event is also one of the places a fresh surface can
                // show up after a resume, so use it as a retry point too.
                if (!s_bgfxValid) {
                    if (TryReinitBgfxIfNeeded()) {
                        SDL_GetWindowSize(window, &currentWidth, &currentHeight);
                        paused = 0;
                    }
                }
            }

            if (event.type == SDL_APP_WILLENTERBACKGROUND) {
                paused = 1;
                HandleEnterBackground();
            }
            if (event.type == SDL_APP_DIDENTERFOREGROUND) {
                // Android may not have handed us a new ANativeWindow yet at
                // the exact moment this fires; if it hasn't, we stay paused
                // and the SDL_WINDOWEVENT_SIZE_CHANGED retry above (or the
                // next DIDENTERFOREGROUND-adjacent event) will pick it up.
                if (TryReinitBgfxIfNeeded()) {
                    SDL_GetWindowSize(window, &currentWidth, &currentHeight);
                    paused = 0;
                }
            }
            if (event.type == SDL_APP_TERMINATING) {
                quit = 1;
            }

            // Defense-in-depth for the null-engine window while backgrounded.
            // NOTE: this alone will NOT fix a crash that happens right after
            // resume, once `engine` is non-null again -- that almost
            // certainly means Input is holding a stale pointer into the
            // *previous* (deleted) EngineMain instance rather than looking
            // up the current one. See the note above CreateAndInitEngine().
            if (engine)
                Input::ReceiveSdlEvent(event);
        }

        if (!paused && s_bgfxValid && engine) {
            engine->MainLoop();
        }
        else {
            // Avoid a busy-spin while backgrounded / while we don't have a
            // valid surface to render into yet.
            SDL_Delay(50);
        }
    }
}

int main(int argc, char* args[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) < 0) {
        fprintf(stderr, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_SetHint(SDL_HINT_VIDEO_EXTERNAL_CONTEXT, "1");
    SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");
    SDL_SetHint(SDL_HINT_ANDROID_TRAP_BACK_BUTTON, "1");

    // No windowed mode on Android -- always fullscreen, sized by the OS.
    int flags = SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_FULLSCREEN | SDL_WINDOW_RESIZABLE;
    window = SDL_CreateWindow("Image", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              0, 0, flags);
    if (!window) {
        fprintf(stderr, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    int screenWidth = 0, screenHeight = 0;
    SDL_GetWindowSize(window, &screenWidth, &screenHeight);

    // NOTE: Android apps are launched via an Intent, not a CLI, so argc/args
    // from SDL_main will normally just contain the package/activity name --
    // ParseCommands is kept here for parity but won't receive real flags
    // unless you thread them through as Intent extras / manifest meta-data
    // and forward them yourself before calling SDL_main.
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

#ifdef __ANDROID__
        chdir(workingDirOverride->second[0].c_str());
#endif

    }
    else
    {
#ifdef __ANDROID__
        // Sensible default: app-private internal storage, since Android has
        // no concept of "the directory the exe was launched from".
        const char* internalPath = SDL_AndroidGetInternalStoragePath();
        if (internalPath) {
            Logger::Log(std::string("using internal storage path: ") + internalPath);
            chdir(internalPath);
        }
#endif
    }

    // OpenGLES is the safe default on Android (bgfx's desktop "OpenGL"
    // renderer is not valid here). Vulkan is a solid option on newer
    // devices/drivers if you want to opt into it.
    s_renderApi = bgfx::RendererType::Vulkan;

    auto renderApiOverride = args_m.find("renderapi");
    if (renderApiOverride != args_m.end())
    {
        const std::string& renderApiStr = renderApiOverride->second[0];
        if (renderApiStr == "vk")
        {
            s_renderApi = bgfx::RendererType::Vulkan;
            Logger::Log("Using Vulkan renderer");
        }
        else if (renderApiStr == "gl" || renderApiStr == "gles")
        {
            s_renderApi = bgfx::RendererType::OpenGLES;
            Logger::Log("Using OpenGLES renderer");
        }
        else
        {
            Logger::Log("Unknown or unsupported render API on Android: " + renderApiStr + ", using default (OpenGLES)");
        }
    }

    // ====================== BGFX INITIALIZATION ======================
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    if (!SDL_GetWindowWMInfo(window, &wmInfo)) {
        fprintf(stderr, "SDL_GetWindowWMInfo failed: %s\n", SDL_GetError());
        return 1;
    }

    if (wmInfo.subsystem != SDL_SYSWM_ANDROID) {
        fprintf(stderr, "Unsupported or unrecognized SDL video subsystem (expected Android)\n");
        return 1;
    }
    printf("SDL subsystem: %d\n", wmInfo.subsystem);

    // Android gives you a native window handle (ANativeWindow*), not a
    // display connection -- there's no ndt equivalent to set.
    if (!InitBgfx(wmInfo.info.android.window, screenWidth, screenHeight)) {
        return 1;
    }

    InitImGuiPlatform();

    // No cursor / relative-mouse concept on a touchscreen -- SDL emulates a
    // mouse pointer from touch events, which is enough for ImGui_ImplSDL2.
    // If your engine's camera/look controls rely on SDL's relative mouse
    // mode for desktop mouse-look, you'll want a touch-specific input path
    // instead (e.g. virtual joystick / drag-to-look) rather than porting
    // SDL_SetRelativeMouseMode as-is.

    printf("bgfx initialized successfully.\n");

    // F11 fullscreen toggle doesn't apply (already fullscreen, no physical
    // keyboard); bind the hardware/gesture back button instead if your
    // engine has a concept of "pause" or "exit" to route it to.
    Input::AddAction("back")->AddKeyboardKey(SDL_GetScancodeFromKey(SDLK_AC_BACK));

    CreateAndInitEngine();

    android_render_loop();

    DestroyEngine();

    ShutdownBgfxGraphics();
    ShutdownImGuiPlatform();

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}