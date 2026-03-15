#define _HAS_STD_BYTE 0
#include "../malloc_override.h"
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_bgfx.h"
#include "../imgui/imgui_impl_sdl2.h"
#include <deque>
#include <algorithm>
#include <array>
#include <dinput.h>
#include <SDL2/SDL_syswm.h>
#include <direct.h>
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
#include <Windows.h>
#include <DbgHelp.h>
#pragma comment(lib, "Dbghelp.lib")

#include "../EngineMain.h"
#include "PlatformWindowData.h"
#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
using namespace PlatformWindowData;

// Global variables
LPDIRECTINPUT8 g_pDI = nullptr;
LPDIRECTINPUTDEVICE8 g_pMouse = nullptr;
EngineMain* engine = nullptr;

// Function declarations
void update_screen_size(int w, int h);
void InitImGui();
void desktop_render_loop();
bool InitDirectInput(SDL_Window* sdlWindow);
vec2 ReadMouseDelta();
void ShutdownDirectInput();

// Function implementations
void update_screen_size(int w, int h) {
    SDL_SetWindowSize(window, w, h);
}

#include "../imgui/FA6FreeSolidFontData.h"
#include "../imgui/IconsFontAwesome6.h"

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

bool InitDirectInput(SDL_Window* sdlWindow) {
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    if (!SDL_GetWindowWMInfo(sdlWindow, &wmInfo)) {
        SDL_LogError(SDL_LOG_CATEGORY_INPUT, "Failed to get window handle: %s", SDL_GetError());
        return false;
    }
    HWND hwnd = wmInfo.info.win.window;
    HRESULT hr = DirectInput8Create(GetModuleHandle(nullptr), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&g_pDI, nullptr);
    if (FAILED(hr)) {
        SDL_LogError(SDL_LOG_CATEGORY_INPUT, "DirectInput8Create failed: %08X", hr);
        return false;
    }
    hr = g_pDI->CreateDevice(GUID_SysMouse, &g_pMouse, nullptr);
    if (FAILED(hr)) {
        SDL_LogError(SDL_LOG_CATEGORY_INPUT, "CreateDevice failed: %08X", hr);
        return false;
    }
    hr = g_pMouse->SetDataFormat(&c_dfDIMouse2);
    if (FAILED(hr)) {
        SDL_LogError(SDL_LOG_CATEGORY_INPUT, "SetDataFormat failed: %08X", hr);
        return false;
    }
    hr = g_pMouse->SetCooperativeLevel(hwnd, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);
    if (FAILED(hr)) {
        SDL_LogError(SDL_LOG_CATEGORY_INPUT, "SetCooperativeLevel failed: %08X", hr);
        return false;
    }
    hr = g_pMouse->Acquire();
    if (FAILED(hr)) {
        SDL_LogError(SDL_LOG_CATEGORY_INPUT, "Acquire failed: %08X", hr);
        return false;
    }
    SDL_ShowCursor(SDL_DISABLE);
    return true;
}

vec2 ReadMouseDelta() {
    DIMOUSESTATE2 mouseState;
    vec2 mouseDelta = vec2(0, 0);
    if (!g_pMouse) return mouseDelta;
    HRESULT hr = g_pMouse->GetDeviceState(sizeof(DIMOUSESTATE2), &mouseState);
    if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED) {
        g_pMouse->Acquire();
    }
    else if (SUCCEEDED(hr)) {
        mouseDelta.x = static_cast<float>(mouseState.lX);
        mouseDelta.y = static_cast<float>(mouseState.lY);
    }
    return mouseDelta;
}

void ShutdownDirectInput() {
    if (g_pMouse) {
        g_pMouse->Unacquire();
        g_pMouse->Release();
        g_pMouse = nullptr;
    }
    if (g_pDI) {
        g_pDI->Release();
        g_pDI = nullptr;
    }
}

bool pendingResize = false;

void desktop_render_loop() {
    SDL_Event event;
    int quit = 0;
    int currentWidth = 800;
    int currentHeight = 600;

    while (!quit)
    {
        Input::PendingMouseDelta = ReadMouseDelta();
        Input::StartEventsFrame();

        while (SDL_PollEvent(&event))
        {
            if (EngineMain::MainInstance->DebugUiEnabled)
                ImGui_ImplSDL2_ProcessEvent(&event);

            if (event.type == SDL_QUIT) quit = 1;

            // BGFX resize handling (high-DPI aware)
            if (event.type == SDL_WINDOWEVENT &&
                (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED ||
                    event.window.event == SDL_WINDOWEVENT_RESIZED))
            {
                SDL_GetWindowSize(window, &currentWidth, &currentHeight);
				pendingResize = true;
            }

            Input::ReceiveSdlEvent(event);
        }

        if (pendingResize)
        {
            bgfx::reset(currentWidth, currentHeight, BGFX_RESET_NONE);
            bgfx::setViewRect(0, 0, 0, currentWidth, currentHeight); // game view
            bgfx::setViewRect(255, 0, 0, currentWidth, currentHeight); // ImGui overlay
			pendingResize = false;
			EngineMain::MainInstance->UpdateScreenSize();
        }

        engine->MainLoop();

    }
}

// Crash handler (unchanged)
static bool g_SymbolsInitialized = false;
static bool g_SymbolsAvailable = false;

void EnsureDbgHelpInitialized()
{
    if (g_SymbolsInitialized)
        return;
    SymSetOptions(SYMOPT_DEFERRED_LOADS | SYMOPT_UNDNAME | SYMOPT_LOAD_LINES);
    if (SymInitialize(GetCurrentProcess(), nullptr, TRUE))
    {
        g_SymbolsAvailable = true;
    }
    else
    {
        g_SymbolsAvailable = false;
    }
    g_SymbolsInitialized = true;
}

LONG WINAPI EngineUnhandledExceptionFilter(EXCEPTION_POINTERS* pExceptionPointers)
{
    EnsureDbgHelpInitialized();
    SYSTEMTIME st;
    GetLocalTime(&st);
    std::wostringstream dumpName;
    dumpName << L"CrashDump_"
        << std::setw(4) << std::setfill(L'0') << st.wYear
        << std::setw(2) << std::setfill(L'0') << st.wMonth
        << std::setw(2) << std::setfill(L'0') << st.wDay
        << L"_"
        << std::setw(2) << std::setfill(L'0') << st.wHour
        << std::setw(2) << std::setfill(L'0') << st.wMinute
        << std::setw(2) << std::setfill(L'0') << st.wSecond
        << L".dmp";
    std::wstring dumpPath = dumpName.str();
    HANDLE hDumpFile = CreateFileW(
        dumpPath.c_str(),
        GENERIC_WRITE,
        0,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );
    if (hDumpFile != INVALID_HANDLE_VALUE)
    {
        MINIDUMP_EXCEPTION_INFORMATION dumpInfo;
        dumpInfo.ThreadId = GetCurrentThreadId();
        dumpInfo.ExceptionPointers = pExceptionPointers;
        dumpInfo.ClientPointers = TRUE;
        MiniDumpWriteDump(
            GetCurrentProcess(),
            GetCurrentProcessId(),
            hDumpFile,
            MiniDumpNormal,
            &dumpInfo,
            nullptr,
            nullptr
        );
        CloseHandle(hDumpFile);
    }
    DWORD exceptionCode = pExceptionPointers->ExceptionRecord->ExceptionCode;
    void* exceptionAddr = pExceptionPointers->ExceptionRecord->ExceptionAddress;
    const USHORT MaxFrames = 62;
    void* backTrace[MaxFrames];
    USHORT framesCaptured = CaptureStackBackTrace(0, MaxFrames, backTrace, nullptr);
    std::wostringstream msg;
    msg << L"An unhandled exception has occurred!\n\n";
    msg << L"Exception code: 0x" << std::hex << exceptionCode << std::dec << L"\n";
    msg << L"Exception address: 0x"
        << std::hex << reinterpret_cast<uintptr_t>(exceptionAddr) << std::dec << L"\n";
    msg << L"Crash time: "
        << std::setw(4) << std::setfill(L'0') << st.wYear << L"-"
        << std::setw(2) << std::setfill(L'0') << st.wMonth << L"-"
        << std::setw(2) << std::setfill(L'0') << st.wDay << L" "
        << std::setw(2) << std::setfill(L'0') << st.wHour << L":"
        << std::setw(2) << std::setfill(L'0') << st.wMinute << L":"
        << std::setw(2) << std::setfill(L'0') << st.wSecond << L"\n\n";
    if (!g_SymbolsAvailable)
    {
        msg << L"*** Symbols not available. Displaying raw addresses only ***\n\n";
    }
    else
    {
        msg << L"Call stack (symbolicated where possible):\n\n";
    }
    for (USHORT i = 2; i < framesCaptured; ++i)
    {
        DWORD64 addr = reinterpret_cast<DWORD64>(backTrace[i]);
        std::wostringstream line;
        line << L"[" << (i - 2) << L"] ";
        if (g_SymbolsAvailable)
        {
            DWORD64 displacementSym = 0;
            DWORD displacementLine = 0;
            BYTE symbolBuffer[sizeof(SYMBOL_INFO) + (MAX_SYM_NAME * sizeof(WCHAR))];
            PSYMBOL_INFO pSymbol = reinterpret_cast<PSYMBOL_INFO>(symbolBuffer);
            pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
            pSymbol->MaxNameLen = MAX_SYM_NAME;
            IMAGEHLP_LINEW64 lineInfo = {};
            lineInfo.SizeOfStruct = sizeof(IMAGEHLP_LINEW64);
            bool gotSymbol = SymFromAddr(GetCurrentProcess(), addr, &displacementSym, pSymbol) != FALSE;
            bool gotLineInfo = SymGetLineFromAddrW64(GetCurrentProcess(), addr, &displacementLine, &lineInfo) != FALSE;
            if (gotSymbol)
            {
                line << pSymbol->Name << L" +0x"
                    << std::hex << displacementSym << std::dec;
                if (gotLineInfo)
                {
                    line << L" (" << lineInfo.FileName << L":" << lineInfo.LineNumber << L")";
                }
            }
            else
            {
                line << L"0x" << std::hex << addr << std::dec;
            }
        }
        else
        {
            line << L"0x" << std::hex << addr << std::dec;
        }
        msg << line.str() << L"\n";
    }
    msg << L"\nMinidump saved to:\n" << dumpPath;
    MessageBoxW(
        nullptr,
        msg.str().c_str(),
        L"Application Crash",
        MB_ICONERROR | MB_OK
    );
    return EXCEPTION_EXECUTE_HANDLER;
}

// Main function
int main(int argc, char* args[])
{
    EnsureDbgHelpInitialized();
    SetUnhandledExceptionFilter(EngineUnhandledExceptionFilter);

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
        _chdir(workingDirOverride->second[0].c_str());
    }

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) < 0) {
        fprintf(stderr, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    // ====================== WINDOW (no OpenGL flags) ======================
    int flags = SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE;
    window = SDL_CreateWindow("Image", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, flags);
    if (!window) {
        fprintf(stderr, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    InitDirectInput(window);

    // ====================== BGFX INITIALIZATION (replaces all OpenGL) ======================
    bgfx::Init init;
    init.type = bgfx::RendererType::Direct3D11;   // auto-select best renderer (D3D11/Vulkan/etc.)
    init.debug = false;
    init.profile = false;


    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    if (!SDL_GetWindowWMInfo(window, &wmInfo)) {
        fprintf(stderr, "SDL_GetWindowWMInfo failed: %s\n", SDL_GetError());
        return 1;
    }
    init.platformData.nwh = wmInfo.info.win.window;   // Windows native handle  

    init.resolution.width = 800;
    init.resolution.height = 600;
    init.resolution.reset = BGFX_RESET_NONE;   // no vsync (matches your old SDL_GL_SetSwapInterval(0))

    if (!bgfx::init(init)) {
        fprintf(stderr, "bgfx::init failed!\n");
        return 1;
    }
    //bgfx::setDebug(BGFX_DEBUG_STATS);
    // Default clear + views
    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x000000ff, 1.0f, 0);
    bgfx::setViewRect(0, 0, 0, 800, 600);
    bgfx::setViewRect(255, 0, 0, 800, 600);

    InitImGui();

    SDL_SetHintWithPriority(SDL_HINT_MOUSE_RELATIVE_MODE_WARP, "1", SDL_HINT_OVERRIDE);
    SDL_SetHintWithPriority(SDL_HINT_MOUSE_RELATIVE_MODE_CENTER, "1", SDL_HINT_OVERRIDE);
    SDL_SetRelativeMouseMode(SDL_TRUE);

    Input::AddAction("test")->AddKeyboardKey(SDL_GetScancodeFromKey(SDLK_t));
    Input::AddAction("fullscreen")->AddKeyboardKey(SDL_GetScancodeFromKey(SDLK_F11));

    engine = new EngineMain(window);
    engine->asyncGameUpdate = ThreadPool::Supported();
    EngineMain::MainInstance = engine;
    engine->Init(args_s);

    desktop_render_loop();

    delete engine;
    ShutdownDirectInput();

    bgfx::frame();      // final present before shutdown
    //bgfx::shutdown();   // replaces SDL_GL_DeleteContext

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}