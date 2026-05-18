#include "EngineInstance.h"
#include "EngineViewportWidget.h"

#include <QDebug>

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl2.h>
#include <imgui/imgui_impl_bgfx.h>

#include <EngineMain.h>

#ifdef _WIN32
#  include <windows.h>
#endif

#if defined(SDL_VIDEO_DRIVER_X11)
#  include <X11/Xlib.h>
#endif

// ─────────────────────────────────────────────────────────────────────────────
// Singleton
// ─────────────────────────────────────────────────────────────────────────────

EngineInstance& EngineInstance::get()
{
    static EngineInstance instance;
    return instance;
}

EngineInstance::~EngineInstance()
{
    if (m_timer)
        m_timer->stop();

    delete m_engine;

    if (m_initialized)
    {
        bgfx::frame();
        bgfx::shutdown();
    }

    if (m_sdlWindow)
        SDL_DestroyWindow(m_sdlWindow);

    SDL_Quit();
}

// ─────────────────────────────────────────────────────────────────────────────
// initialize
// ─────────────────────────────────────────────────────────────────────────────

void EngineInstance::initialize()
{
    if (m_initialized)
        return;

    constexpr int W = 1280;
    constexpr int H = 720;

    qDebug() << "[Engine] initSdl";
    initSdl(W, H);

    qDebug() << "[Engine] initBgfx";
    initBgfx(W, H);

    qDebug() << "[Engine] ImGui";
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForOpenGL(m_sdlWindow, nullptr);
    ImGui_Implbgfx_Init(255);


    qDebug() << "[Engine] EngineMain";
    m_engine = new EngineMain(m_sdlWindow);
    EngineMain::MainInstance = m_engine;
    m_engine->asyncGameUpdate = false;
    m_engine->DebugUiEnabled = true;
    m_engine->Init();

    m_timer = new QTimer(this);
    m_timer->setTimerType(Qt::PreciseTimer);
    connect(m_timer, &QTimer::timeout, this, &EngineInstance::tick);
    m_timer->start(0);

    m_initialized = true;
    qDebug() << "[Engine] initialized";
}

// ─────────────────────────────────────────────────────────────────────────────
// sdlWindowId
// ─────────────────────────────────────────────────────────────────────────────

Uint32 EngineInstance::sdlWindowId() const
{
    return m_sdlWindow ? SDL_GetWindowID(m_sdlWindow) : 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// Windows WndProc subclass — the real input fix
//
// The SDL child HWND sits on top of the Qt widget in the Z-order.  Win32
// routes all mouse and keyboard messages to it directly.  SDL's own WndProc
// is registered for that HWND, but SDL silently discards the messages because
// it considers the window hidden or unfocused internally.
//
// We replace the HWND's WndProc with sdlSubclassProc, which:
//   • intercepts every input message (key, char, mouse button, motion, wheel)
//   • translates it to the matching SDL_Event
//   • pushes it into SDL's queue with SDL_PushEvent
//   • returns without calling SDL's original proc (to prevent double-queueing)
//
// All non-input messages (WM_PAINT, WM_SIZE, WM_DESTROY, etc.) are forwarded
// to SDL's original proc unchanged so SDL's internal window state stays valid.
// ─────────────────────────────────────────────────────────────────────────────

#ifdef _WIN32

static WNDPROC s_origSdlWndProc = nullptr;
static Uint32  s_sdlWinId       = 0;

// Tracks cursor position across WM_MOUSEMOVE so we can compute xrel / yrel.
// (We can't rely on SDL's own relative tracking because the window is embedded.)
static LONG s_lastMouseX = 0;
static LONG s_lastMouseY = 0;

// ── VK → SDL_Scancode ────────────────────────────────────────────────────────
// SDL_Scancode is USB HID Usage Page 07.  We map directly from Windows VK
// codes, handling left/right modifier disambiguation via MapVirtualKey on
// the scan code embedded in lParam (bits 16–23 + extended-key bit 24).
static SDL_Scancode vkToSdlScancode(WPARAM vk, LPARAM lp)
{
    // Use MapVirtualKey to distinguish left/right shift, ctrl, alt.
    // Windows sends VK_SHIFT / VK_CONTROL / VK_MENU (the generic codes) in
    // WM_KEYDOWN; the scan code in lParam tells us which physical key it is.
    if (vk == VK_SHIFT || vk == VK_CONTROL || vk == VK_MENU)
    {
        UINT sc = (lp >> 16) & 0xFF;
        WPARAM extended = MapVirtualKey(sc, MAPVK_VSC_TO_VK_EX);
        if (extended) vk = extended;
    }

    // Letters A–Z: VK_A==0x41 … VK_Z==0x5A; SDL_SCANCODE_A==4 … sequential.
    if (vk >= 'A' && vk <= 'Z')
        return static_cast<SDL_Scancode>(SDL_SCANCODE_A + (vk - 'A'));

    // Digit row 1–9 then 0.
    if (vk >= '1' && vk <= '9')
        return static_cast<SDL_Scancode>(SDL_SCANCODE_1 + (vk - '1'));
    if (vk == '0')
        return SDL_SCANCODE_0;

    // Function keys F1–F24.
    if (vk >= VK_F1 && vk <= VK_F12)
        return static_cast<SDL_Scancode>(SDL_SCANCODE_F1 + (vk - VK_F1));

    // Numpad digits.
    if (vk >= VK_NUMPAD0 && vk <= VK_NUMPAD9)
        return static_cast<SDL_Scancode>(SDL_SCANCODE_KP_0 + (vk - VK_NUMPAD0));

    const bool extKey = (lp & (1u << 24)) != 0;

    switch (vk)
    {
    // ── Editing / whitespace ──────────────────────────────────────────────────
    case VK_RETURN:     return extKey ? SDL_SCANCODE_KP_ENTER : SDL_SCANCODE_RETURN;
    case VK_ESCAPE:     return SDL_SCANCODE_ESCAPE;
    case VK_BACK:       return SDL_SCANCODE_BACKSPACE;
    case VK_TAB:        return SDL_SCANCODE_TAB;
    case VK_SPACE:      return SDL_SCANCODE_SPACE;
    case VK_DELETE:     return SDL_SCANCODE_DELETE;
    case VK_INSERT:     return SDL_SCANCODE_INSERT;

    // ── Navigation ───────────────────────────────────────────────────────────
    case VK_UP:         return SDL_SCANCODE_UP;
    case VK_DOWN:       return SDL_SCANCODE_DOWN;
    case VK_LEFT:       return SDL_SCANCODE_LEFT;
    case VK_RIGHT:      return SDL_SCANCODE_RIGHT;
    case VK_HOME:       return SDL_SCANCODE_HOME;
    case VK_END:        return SDL_SCANCODE_END;
    case VK_PRIOR:      return SDL_SCANCODE_PAGEUP;
    case VK_NEXT:       return SDL_SCANCODE_PAGEDOWN;

    // ── Modifiers ────────────────────────────────────────────────────────────
    case VK_LSHIFT:     return SDL_SCANCODE_LSHIFT;
    case VK_RSHIFT:     return SDL_SCANCODE_RSHIFT;
    case VK_LCONTROL:   return SDL_SCANCODE_LCTRL;
    case VK_RCONTROL:   return SDL_SCANCODE_RCTRL;
    case VK_LMENU:      return SDL_SCANCODE_LALT;
    case VK_RMENU:      return SDL_SCANCODE_RALT;
    case VK_LWIN:       return SDL_SCANCODE_LGUI;
    case VK_RWIN:       return SDL_SCANCODE_RGUI;
    case VK_APPS:       return SDL_SCANCODE_APPLICATION;
    case VK_CAPITAL:    return SDL_SCANCODE_CAPSLOCK;
    case VK_NUMLOCK:    return SDL_SCANCODE_NUMLOCKCLEAR;
    case VK_SCROLL:     return SDL_SCANCODE_SCROLLLOCK;

    // ── Lock / system ────────────────────────────────────────────────────────
    case VK_SNAPSHOT:   return SDL_SCANCODE_PRINTSCREEN;
    case VK_PAUSE:      return SDL_SCANCODE_PAUSE;

    // ── Numpad operators ─────────────────────────────────────────────────────
    case VK_MULTIPLY:   return SDL_SCANCODE_KP_MULTIPLY;
    case VK_ADD:        return SDL_SCANCODE_KP_PLUS;
    case VK_SUBTRACT:   return SDL_SCANCODE_KP_MINUS;
    case VK_DECIMAL:    return SDL_SCANCODE_KP_PERIOD;
    case VK_DIVIDE:     return SDL_SCANCODE_KP_DIVIDE;

    // ── OEM / punctuation (US layout) ────────────────────────────────────────
    case VK_OEM_MINUS:  return SDL_SCANCODE_MINUS;
    case VK_OEM_PLUS:   return SDL_SCANCODE_EQUALS;
    case VK_OEM_4:      return SDL_SCANCODE_LEFTBRACKET;   // [
    case VK_OEM_6:      return SDL_SCANCODE_RIGHTBRACKET;  // ]
    case VK_OEM_5:      return SDL_SCANCODE_BACKSLASH;     // backslash
    case VK_OEM_1:      return SDL_SCANCODE_SEMICOLON;     // ;
    case VK_OEM_7:      return SDL_SCANCODE_APOSTROPHE;    // '
    case VK_OEM_3:      return SDL_SCANCODE_GRAVE;         // `
    case VK_OEM_COMMA:  return SDL_SCANCODE_COMMA;
    case VK_OEM_PERIOD: return SDL_SCANCODE_PERIOD;
    case VK_OEM_2:      return SDL_SCANCODE_SLASH;         // /

    default:            return SDL_SCANCODE_UNKNOWN;
    }
}

// Current SDL modifier mask sampled from the live keyboard state.
// Used to fill SDL_Keysym::mod so engines can test for Shift/Ctrl/Alt
// without maintaining their own modifier state.
static SDL_Keymod currentSdlKeyMod()
{
    int mod = KMOD_NONE;
    if (GetKeyState(VK_LSHIFT)   & 0x8000) mod |= KMOD_LSHIFT;
    if (GetKeyState(VK_RSHIFT)   & 0x8000) mod |= KMOD_RSHIFT;
    if (GetKeyState(VK_LCONTROL) & 0x8000) mod |= KMOD_LCTRL;
    if (GetKeyState(VK_RCONTROL) & 0x8000) mod |= KMOD_RCTRL;
    if (GetKeyState(VK_LMENU)    & 0x8000) mod |= KMOD_LALT;
    if (GetKeyState(VK_RMENU)    & 0x8000) mod |= KMOD_RALT;
    if (GetKeyState(VK_LWIN)     & 0x8000) mod |= KMOD_LGUI;
    if (GetKeyState(VK_RWIN)     & 0x8000) mod |= KMOD_RGUI;
    if (GetKeyState(VK_CAPITAL)  & 0x0001) mod |= KMOD_CAPS;
    if (GetKeyState(VK_NUMLOCK)  & 0x0001) mod |= KMOD_NUM;
    return static_cast<SDL_Keymod>(mod);
}

// Helpers for filling SDL_KeyboardEvent.
static void fillKeysym(SDL_Keysym& ks, WPARAM vk, LPARAM lp)
{
    ks.scancode = vkToSdlScancode(vk, lp);
    ks.sym      = SDL_GetKeyFromScancode(ks.scancode);
    ks.mod      = static_cast<Uint16>(currentSdlKeyMod());
}

// ── The subclassed WndProc ────────────────────────────────────────────────────
static LRESULT CALLBACK sdlSubclassProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{


    switch (msg)
    {
    case WM_KEYDOWN:
    {
        UINT scanCode = (lp >> 16) & 0xFF;
        char keyName[128] = {};

        GetKeyNameTextA(scanCode << 16, keyName, sizeof(keyName));

        qDebug() << "[DOWN]" << keyName;
        break;
    }

    case WM_KEYUP:
    {
        UINT scanCode = (lp >> 16) & 0xFF;
        char keyName[128] = {};

        GetKeyNameTextA(scanCode << 16, keyName, sizeof(keyName));

        qDebug() << "[UP]" << keyName;
        break;
    }
    }


    switch (msg)
    {
    // ── Keyboard ─────────────────────────────────────────────────────────────
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    {
        SDL_Event e{};
        e.type              = SDL_KEYDOWN;
        e.key.timestamp     = SDL_GetTicks();
        e.key.windowID      = s_sdlWinId;
        e.key.state         = SDL_PRESSED;
        e.key.repeat        = (lp & (1u << 30)) ? 1 : 0;   // bit 30 = previous key state
        fillKeysym(e.key.keysym, wp, lp);
        SDL_PushEvent(&e);
        return 0;
    }

    case WM_KEYUP:
    case WM_SYSKEYUP:
    {
        SDL_Event e{};
        e.type              = SDL_KEYUP;
        e.key.timestamp     = SDL_GetTicks();
        e.key.windowID      = s_sdlWinId;
        e.key.state         = SDL_RELEASED;
        e.key.repeat        = 0;
        fillKeysym(e.key.keysym, wp, lp);
        SDL_PushEvent(&e);
        return 0;
    }

    // WM_CHAR / WM_UNICHAR carry the translated Unicode character produced by
    // the key press.  These drive SDL_TEXTINPUT so that ImGui text fields and
    // any other text-input consumers work correctly.
    case WM_CHAR:
    case WM_UNICHAR:
    {
        if (msg == WM_UNICHAR && wp == UNICODE_NOCHAR)
            return TRUE;   // signal that we handle WM_UNICHAR
        if (wp < 32 && wp != '\t')
            break;          // skip non-printable control characters

        wchar_t wc = static_cast<wchar_t>(wp);
        char utf8[5] = {};
        WideCharToMultiByte(CP_UTF8, 0, &wc, 1, utf8, 4, nullptr, nullptr);

        SDL_Event e{};
        e.type           = SDL_TEXTINPUT;
        e.text.timestamp = SDL_GetTicks();
        e.text.windowID  = s_sdlWinId;
        strncpy(e.text.text, utf8, sizeof(e.text.text) - 1);
        SDL_PushEvent(&e);
        return 0;
    }

    // ── Mouse motion ─────────────────────────────────────────────────────────
    case WM_MOUSEMOVE:
    {
        LONG x = static_cast<LONG>(static_cast<short>(LOWORD(lp)));
        LONG y = static_cast<LONG>(static_cast<short>(HIWORD(lp)));

        SDL_Event e{};
        e.type              = SDL_MOUSEMOTION;
        e.motion.timestamp  = SDL_GetTicks();
        e.motion.windowID   = s_sdlWinId;
        e.motion.which      = 0;
        e.motion.x          = x;
        e.motion.y          = y;
        e.motion.xrel       = static_cast<Sint32>(x - s_lastMouseX);
        e.motion.yrel       = static_cast<Sint32>(y - s_lastMouseY);

        // Build SDL button-state mask from wParam MK_* flags.
        e.motion.state = 0;
        if (wp & MK_LBUTTON)  e.motion.state |= SDL_BUTTON_LMASK;
        if (wp & MK_RBUTTON)  e.motion.state |= SDL_BUTTON_RMASK;
        if (wp & MK_MBUTTON)  e.motion.state |= SDL_BUTTON_MMASK;
        if (wp & MK_XBUTTON1) e.motion.state |= SDL_BUTTON_X1MASK;
        if (wp & MK_XBUTTON2) e.motion.state |= SDL_BUTTON_X2MASK;

        s_lastMouseX = x;
        s_lastMouseY = y;
        SDL_PushEvent(&e);
        return 0;
    }

    // ── Mouse buttons ─────────────────────────────────────────────────────────
    case WM_LBUTTONDOWN: case WM_LBUTTONUP:
    case WM_RBUTTONDOWN: case WM_RBUTTONUP:
    case WM_MBUTTONDOWN: case WM_MBUTTONUP:
    case WM_XBUTTONDOWN: case WM_XBUTTONUP:
    {
        const bool down = (msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN ||
                           msg == WM_MBUTTONDOWN || msg == WM_XBUTTONDOWN);

        Uint8 button;
        switch (msg)
        {
        case WM_LBUTTONDOWN: case WM_LBUTTONUP: button = SDL_BUTTON_LEFT;   break;
        case WM_RBUTTONDOWN: case WM_RBUTTONUP: button = SDL_BUTTON_RIGHT;  break;
        case WM_MBUTTONDOWN: case WM_MBUTTONUP: button = SDL_BUTTON_MIDDLE; break;
        default: // XBUTTON
            button = (HIWORD(wp) == XBUTTON1) ? SDL_BUTTON_X1 : SDL_BUTTON_X2;
            break;
        }

        LONG x = static_cast<LONG>(static_cast<short>(LOWORD(lp)));
        LONG y = static_cast<LONG>(static_cast<short>(HIWORD(lp)));

        SDL_Event e{};
        e.type              = down ? SDL_MOUSEBUTTONDOWN : SDL_MOUSEBUTTONUP;
        e.button.timestamp  = SDL_GetTicks();
        e.button.windowID   = s_sdlWinId;
        e.button.which      = 0;
        e.button.button     = button;
        e.button.state      = down ? SDL_PRESSED : SDL_RELEASED;
        e.button.clicks     = 1;
        e.button.x          = static_cast<Sint32>(x);
        e.button.y          = static_cast<Sint32>(y);
        SDL_PushEvent(&e);

        // Keep focus on the SDL child window so subsequent keyboard messages
        // keep arriving here rather than going to whichever Qt widget Qt would
        // focus on a click.
        if (down)
            SetFocus(hwnd);

        // WM_XBUTTON* must return TRUE per MSDN.
        return (msg == WM_XBUTTONDOWN || msg == WM_XBUTTONUP) ? TRUE : 0;
    }

    // ── Scroll wheel (vertical) ───────────────────────────────────────────────
    case WM_MOUSEWHEEL:
    {
        // WM_MOUSEWHEEL position is in screen coordinates.
        POINT pt = { static_cast<LONG>(static_cast<short>(LOWORD(lp))),
                    static_cast<LONG>(static_cast<short>(HIWORD(lp))) };
        ScreenToClient(hwnd, &pt);

        const int notches = GET_WHEEL_DELTA_WPARAM(wp) / WHEEL_DELTA;

        SDL_Event e{};
        e.type              = SDL_MOUSEWHEEL;
        e.wheel.timestamp   = SDL_GetTicks();
        e.wheel.windowID    = s_sdlWinId;
        e.wheel.which       = 0;
        e.wheel.x           = 0;
        e.wheel.y           = notches;   // positive = away from user (SDL convention)
        e.wheel.direction   = SDL_MOUSEWHEEL_NORMAL;
        SDL_PushEvent(&e);
        return 0;
    }

    // ── Horizontal scroll ─────────────────────────────────────────────────────
    case WM_MOUSEHWHEEL:
    {
        const int notches = GET_WHEEL_DELTA_WPARAM(wp) / WHEEL_DELTA;

        SDL_Event e{};
        e.type              = SDL_MOUSEWHEEL;
        e.wheel.timestamp   = SDL_GetTicks();
        e.wheel.windowID    = s_sdlWinId;
        e.wheel.which       = 0;
        e.wheel.x           = notches;
        e.wheel.y           = 0;
        e.wheel.direction   = SDL_MOUSEWHEEL_NORMAL;
        SDL_PushEvent(&e);
        return 0;
    }

    default:
        break;
    }

    // All non-input messages go to SDL's original WndProc so SDL's internal
    // window bookkeeping (WM_SIZE, WM_PAINT, WM_DESTROY, etc.) stays intact.
    return CallWindowProc(s_origSdlWndProc, hwnd, msg, wp, lp);
}

#endif // _WIN32

// ─────────────────────────────────────────────────────────────────────────────
// installInputHook — called once from attachToWidget
// ─────────────────────────────────────────────────────────────────────────────

void EngineInstance::installInputHook()
{
#ifdef _WIN32
    if (m_hookInstalled)
        return;

    SDL_SysWMinfo wm;
    SDL_VERSION(&wm.version);
    if (!SDL_GetWindowWMInfo(m_sdlWindow, &wm))
    {
        qWarning("[Engine] installInputHook: SDL_GetWindowWMInfo failed");
        return;
    }

    HWND sdlHwnd = wm.info.win.window;

    s_sdlWinId   = SDL_GetWindowID(m_sdlWindow);
    s_lastMouseX = 0;
    s_lastMouseY = 0;

    // Replace the SDL HWND's WndProc with our subclass.
    // SetWindowLongPtr returns the previous proc, which we must keep to
    // forward non-input messages and to avoid breaking SDL's internal state.
    s_origSdlWndProc = reinterpret_cast<WNDPROC>(
        SetWindowLongPtr(sdlHwnd, GWLP_WNDPROC,
                         reinterpret_cast<LONG_PTR>(sdlSubclassProc)));

    if (!s_origSdlWndProc)
        qWarning("[Engine] installInputHook: SetWindowLongPtr failed (error %lu)",
                 GetLastError());
    else
        qDebug() << "[Engine] WndProc subclass installed on SDL HWND";

    m_hookInstalled = true;
#endif
}

// ─────────────────────────────────────────────────────────────────────────────
// attachToWidget / detachWidget
// ─────────────────────────────────────────────────────────────────────────────

void EngineInstance::attachToWidget(EngineViewportWidget* widget)
{
    m_activeWidget = widget;

    const int w = qMax(widget->width(),  1);
    const int h = qMax(widget->height(), 1);

    SDL_SysWMinfo wm;
    SDL_VERSION(&wm.version);
    SDL_GetWindowWMInfo(m_sdlWindow, &wm);

#ifdef _WIN32
    HWND sdlHwnd = wm.info.win.window;
    HWND qtHwnd  = reinterpret_cast<HWND>(widget->winId());

    SetWindowLongPtr(sdlHwnd, GWL_STYLE,
                     (GetWindowLongPtr(sdlHwnd, GWL_STYLE) & ~WS_POPUP) | WS_CHILD);

    SetParent(sdlHwnd, qtHwnd);
    SetWindowPos(sdlHwnd, nullptr, 0, 0, w, h, SWP_NOZORDER | SWP_SHOWWINDOW);
    SetFocus(sdlHwnd);

    // Install our WndProc subclass the first time an SDL window is embedded.
    // Must be done AFTER reparenting so the HWND is already in its final state.
    installInputHook();
#endif

#if defined(SDL_VIDEO_DRIVER_X11)
    if (wm.subsystem == SDL_SYSWM_X11) {
        Display* dpy    = wm.info.x11.display;
        Window   sdlWin = wm.info.x11.window;
        Window   qtWin  = static_cast<Window>(widget->winId());
        XReparentWindow(dpy, sdlWin, qtWin, 0, 0);
        XMapWindow(dpy, sdlWin);
        XSetInputFocus(dpy, sdlWin, RevertToParent, CurrentTime);
        XFlush(dpy);
    }
#endif

    SDL_SetWindowSize(m_sdlWindow, w, h);
    bgfx::reset((uint32_t)w, (uint32_t)h, BGFX_RESET_NONE);
    bgfx::setViewRect(0,   0, 0, (uint16_t)w, (uint16_t)h);
    bgfx::setViewRect(255, 0, 0, (uint16_t)w, (uint16_t)h);
}

void EngineInstance::detachWidget()
{
#ifdef _WIN32
    // Only touch Win32 geometry; do NOT uninstall the WndProc subclass.
    // The subclass must stay in place for the lifetime of the HWND — removing
    // it while messages may still be in flight would corrupt the chain.
    SDL_SysWMinfo wm;
    SDL_VERSION(&wm.version);
    SDL_GetWindowWMInfo(m_sdlWindow, &wm);

    HWND sdlHwnd = wm.info.win.window;

    SetWindowLongPtr(sdlHwnd, GWL_STYLE,
                     (GetWindowLongPtr(sdlHwnd, GWL_STYLE) & ~WS_CHILD) | WS_POPUP);
    SetParent(sdlHwnd, nullptr);
    ShowWindow(sdlHwnd, SW_HIDE);
#endif

    m_activeWidget = nullptr;
}

// ─────────────────────────────────────────────────────────────────────────────
// onWidgetResized
// ─────────────────────────────────────────────────────────────────────────────

void EngineInstance::onWidgetResized(int w, int h)
{
    w = qMax(w, 1);
    h = qMax(h, 1);

    SDL_SetWindowSize(m_sdlWindow, w, h);

#ifdef _WIN32
    SDL_SysWMinfo wm;
    SDL_VERSION(&wm.version);
    SDL_GetWindowWMInfo(m_sdlWindow, &wm);
    SetWindowPos(wm.info.win.window, nullptr, 0, 0, w, h, SWP_NOZORDER);
#endif

    bgfx::reset((uint32_t)w, (uint32_t)h, BGFX_RESET_NONE);
    bgfx::setViewRect(0,   0, 0, (uint16_t)w, (uint16_t)h);
    bgfx::setViewRect(255, 0, 0, (uint16_t)w, (uint16_t)h);
}

// ─────────────────────────────────────────────────────────────────────────────
// tick
// ─────────────────────────────────────────────────────────────────────────────

bool wasCursorLocked = false;

void EngineInstance::tick()
{
    Input::PendingMouseDelta = vec2(0);
    Input::StartEventsFrame();
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (EngineMain::MainInstance->DebugUiEnabled)
            ImGui_ImplSDL2_ProcessEvent(&event);
        Input::ReceiveSdlEvent(event);
    }

    QWidget* widget = m_activeWidget;
    if (Input::LockCursor && widget && widget->hasFocus())
    {
        QPoint center      = widget->rect().center();
        QPoint globalCenter = widget->mapToGlobal(center);
        QPoint currentPos  = widget->mapFromGlobal(QCursor::pos());
        QPoint delta       = currentPos - center;

        // Only snap if the cursor drifted more than 50 px from center
        if (delta.x() * delta.x() + delta.y() * delta.y() > 2 * 2)
        {
            QCursor::setPos(globalCenter);

            if(wasCursorLocked)
                Input::PendingMouseDelta = vec2(delta.x(), delta.y());
        }
    }
    wasCursorLocked = Input::LockCursor;
    m_engine->MainLoop();
}

// ─────────────────────────────────────────────────────────────────────────────
// Private init helpers
// ─────────────────────────────────────────────────────────────────────────────

void EngineInstance::initSdl(int w, int h)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) < 0)
        qFatal("SDL_Init failed: %s", SDL_GetError());

    m_sdlWindow = SDL_CreateWindow(
        "",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        w, h,
        SDL_WINDOW_BORDERLESS | SDL_WINDOW_HIDDEN | SDL_WINDOW_ALLOW_HIGHDPI
        );

    if (!m_sdlWindow)
        qFatal("SDL_CreateWindow failed: %s", SDL_GetError());
}

void EngineInstance::initBgfx(int w, int h)
{
    SDL_SysWMinfo wm;
    SDL_VERSION(&wm.version);
    if (!SDL_GetWindowWMInfo(m_sdlWindow, &wm))
        qFatal("SDL_GetWindowWMInfo failed: %s", SDL_GetError());

    bgfx::Init init;
    init.type    = bgfx::RendererType::Direct3D11;
    init.debug   = false;
    init.profile = false;

#if defined(SDL_VIDEO_DRIVER_X11)
    if (wm.subsystem == SDL_SYSWM_X11) {
        init.platformData.nwh = (void*)(uintptr_t)wm.info.x11.window;
        init.platformData.ndt = wm.info.x11.display;
    }
#endif
#if defined(SDL_VIDEO_DRIVER_WAYLAND)
    if (wm.subsystem == SDL_SYSWM_WAYLAND) {
        init.platformData.nwh = wm.info.wl.surface;
        init.platformData.ndt = wm.info.wl.display;
    }
#endif
#ifdef _WIN32
    init.platformData.nwh = wm.info.win.window;
#endif

    init.resolution.width  = (uint32_t)w;
    init.resolution.height = (uint32_t)h;
    init.resolution.reset  = BGFX_RESET_NONE;

    if (!bgfx::init(init))
        qFatal("bgfx::init failed");

    bgfx::setDebug(BGFX_DEBUG_NONE);

    bgfx::setViewClear(0,   BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x000000ff, 1.0f, 0);
    bgfx::setViewRect (0,   0, 0, (uint16_t)w, (uint16_t)h);
    bgfx::setViewRect (255, 0, 0, (uint16_t)w, (uint16_t)h);
}