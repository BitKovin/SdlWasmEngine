#define _HAS_STD_BYTE 0
#include "../malloc_override.h"
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_opengl3.h"
#include "../imgui/imgui_impl_sdl2.h"
#include "../gl.h"
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

extern "C" {
    _declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}

#include "../EngineMain.h"

#include "PlatformWindowData.h"
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
    ImGui_ImplSDL2_InitForOpenGL(window, glContext);
    ImGui_ImplOpenGL3_Init();
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

void desktop_render_loop() {
    SDL_Event event;
    int quit = 0;
    while (!quit) 
    {
        Input::PendingMouseDelta = ReadMouseDelta();

        Input::StartEventsFrame();
        while (SDL_PollEvent(&event)) 
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT) quit = 1;

            Input::ReceiveSdlEvent(event);

        }
        engine->MainLoop();
    }
}

#ifndef GL_ES_PROFILE


int g_DebugSeverityLevel = 1; // 0: All, 1: Warnings+Errors, 2: Errors only
void APIENTRY openglDebugCallback(GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar* message,
    const void* userParam)
{
    // Filter by severity level
    switch (g_DebugSeverityLevel) {
    case 2: // Errors only
        if (severity != GL_DEBUG_SEVERITY_HIGH)
            return;
        break;
    case 1: // Warnings + Errors
        if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)
            return;
        break;
    case 0: // All messages allowed
    default:
        break;
    }

    std::cerr << "OpenGL Debug Message:\n";
    std::cerr << "  Source: " << source << "\n";
    std::cerr << "  Type: " << type << "\n";
    std::cerr << "  ID: " << id << "\n";
    std::cerr << "  Severity: ";
    switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH: std::cerr << "HIGH"; break;
    case GL_DEBUG_SEVERITY_MEDIUM: std::cerr << "MEDIUM"; break;
    case GL_DEBUG_SEVERITY_LOW: std::cerr << "LOW"; break;
    case GL_DEBUG_SEVERITY_NOTIFICATION: std::cerr << "NOTIFICATION"; break;
    default: std::cerr << "UNKNOWN"; break;
    }
    std::cerr << "\n  Message: " << message << "\n\n";
}

#endif // !GL_ES_PROFILE

// Link against dbghelp.lib
#pragma comment(lib, "dbghelp.lib")

// Keeps track of whether DbgHelp was initialized and whether symbols are available
static bool g_SymbolsInitialized = false;
static bool g_SymbolsAvailable = false;

// Attempt to initialize the symbol engine exactly once.
void EnsureDbgHelpInitialized()
{
    if (g_SymbolsInitialized)
        return;

    // Try to set options: undecorate names, load line numbers
    SymSetOptions(SYMOPT_DEFERRED_LOADS | SYMOPT_UNDNAME | SYMOPT_LOAD_LINES);

    // Attempt to initialize. If this fails, we’ll fall back to raw addresses.
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
    // 1. Initialize the symbol engine (only once per process)
    EnsureDbgHelpInitialized();

    // 2. Grab the crash time (local)
    SYSTEMTIME st;
    GetLocalTime(&st);

    // 3. Build a filename like "CrashDump_YYYYMMDD_HHMMSS.dmp"
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

    // 4. Write the minidump
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

    // 5. Gather basic exception info
    DWORD exceptionCode = pExceptionPointers->ExceptionRecord->ExceptionCode;
    void* exceptionAddr = pExceptionPointers->ExceptionRecord->ExceptionAddress;

    // 6. Capture up to 62 frames on the stack
    const USHORT MaxFrames = 62;
    void* backTrace[MaxFrames];
    USHORT framesCaptured = CaptureStackBackTrace(0, MaxFrames, backTrace, nullptr);

    // 7. Build the message box text
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

    // 8. Header for the call stack
    if (!g_SymbolsAvailable)
    {
        msg << L"*** Symbols not available. Displaying raw addresses only ***\n\n";
    }
    else
    {
        msg << L"Call stack (symbolicated where possible):\n\n";
    }

    // 9. Iterate frames, skip first two (CaptureStackBackTrace + handler itself)
    for (USHORT i = 2; i < framesCaptured; ++i)
    {
        DWORD64 addr = reinterpret_cast<DWORD64>(backTrace[i]);
        std::wostringstream line;

        // Print frame index
        line << L"[" << (i - 2) << L"] ";

        if (g_SymbolsAvailable)
        {
            // Try to resolve symbol + line
            DWORD64 displacementSym = 0;
            DWORD   displacementLine = 0;
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
                // Print function name + offset
                line << pSymbol->Name << L" +0x"
                    << std::hex << displacementSym << std::dec;
                if (gotLineInfo)
                {
                    // Print "(File.cpp:Line)"
                    line << L" (" << lineInfo.FileName << L":" << lineInfo.LineNumber << L")";
                }
            }
            else
            {
                // Fallback to raw address
                line << L"0x" << std::hex << addr << std::dec;
            }
        }
        else
        {
            // No symbols at all → raw address
            line << L"0x" << std::hex << addr << std::dec;
        }


        // 11. Append a newline and add to the message
        msg << line.str() << L"\n";
    }

    // 12. Finally, tell the user where the dump was written
    msg << L"\nMinidump saved to:\n" << dumpPath;

    // 13. Show the MessageBox
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

#ifdef GL_ES_PROFILE

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, GL_TRUE);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

#else

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, GL_TRUE);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 2);

#endif // !GL_ES_PROFILE





#ifdef GL_ES_PROFILE

    SDL_SetHint(SDL_HINT_OPENGL_ES_DRIVER, "1");

#endif // GL_ES_PROFILE


#if DEBUG

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);

#endif


    int flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE;
    window = SDL_CreateWindow("Image", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, flags);
    if (!window) {
        fprintf(stderr, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    TTF_Init();
    InitDirectInput(window);
    glContext = SDL_GL_CreateContext(window);
    if (!glContext) {
        fprintf(stderr, "OpenGL context could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

#ifndef GL_ES_PROFILE
    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW: %s\n", glewGetErrorString(glewError));
        return 1;
    }
#endif

#if DEBUG 
#ifndef GL_ES_PROFILE

    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // For synchronous callback
    glDebugMessageCallback(openglDebugCallback, nullptr);

#endif
#endif

    InitImGui();
    SDL_SetHintWithPriority(SDL_HINT_MOUSE_RELATIVE_MODE_WARP, "1", SDL_HINT_OVERRIDE);
    SDL_SetHintWithPriority(SDL_HINT_MOUSE_RELATIVE_MODE_CENTER, "1", SDL_HINT_OVERRIDE);
    SDL_SetRelativeMouseMode(SDL_TRUE);

    printf("GL_VERSION = %s\n", glGetString(GL_VERSION));
    printf("GL_VENDOR = %s\n", glGetString(GL_VENDOR));
    printf("GL_RENDERER = %s\n", glGetString(GL_RENDERER));

    Input::AddAction("test")->AddKeyboardKey(SDL_GetScancodeFromKey(SDLK_t));
    Input::AddAction("fullscreen")->AddKeyboardKey(SDL_GetScancodeFromKey(SDLK_F11));


    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    SDL_GL_SetSwapInterval(0);

    engine = new EngineMain(window);

    engine->asyncGameUpdate = ThreadPool::Supported();

    EngineMain::MainInstance = engine;
    engine->Init(args_s);

    desktop_render_loop();

	delete engine;

    ShutdownDirectInput();
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}