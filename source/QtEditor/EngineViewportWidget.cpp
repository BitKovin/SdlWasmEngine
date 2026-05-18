#include "EngineViewportWidget.h"
#include "EngineInstance.h"

#include <QShowEvent>
#include <QHideEvent>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QApplication>

#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_mouse.h>

// ─────────────────────────────────────────────────────────────────────────────
// Translation helpers
// ─────────────────────────────────────────────────────────────────────────────

// Map a Qt::Key to the SDL_Scancode for the physical key in the same position
// on a standard US layout. SDL input systems typically work off scancodes,
// so this is the correct level to translate at.
static SDL_Scancode qtKeyToSdlScancode(int key)
{
    // Letters — both ranges are sequential and aligned.
    if (key >= Qt::Key_A && key <= Qt::Key_Z)
        return static_cast<SDL_Scancode>(SDL_SCANCODE_A + (key - Qt::Key_A));

    // Digit row — SDL has 1-9 then 0; Qt has 0-9 sequentially.
    if (key >= Qt::Key_1 && key <= Qt::Key_9)
        return static_cast<SDL_Scancode>(SDL_SCANCODE_1 + (key - Qt::Key_1));
    if (key == Qt::Key_0)
        return SDL_SCANCODE_0;

    // Function keys F1-F12 are sequential in both enums.
    if (key >= Qt::Key_F1 && key <= Qt::Key_F12)
        return static_cast<SDL_Scancode>(SDL_SCANCODE_F1 + (key - Qt::Key_F1));

    switch (key)
    {
    // ── Navigation ───────────────────────────────────────────────────────────
    case Qt::Key_Up:        return SDL_SCANCODE_UP;
    case Qt::Key_Down:      return SDL_SCANCODE_DOWN;
    case Qt::Key_Left:      return SDL_SCANCODE_LEFT;
    case Qt::Key_Right:     return SDL_SCANCODE_RIGHT;
    case Qt::Key_Home:      return SDL_SCANCODE_HOME;
    case Qt::Key_End:       return SDL_SCANCODE_END;
    case Qt::Key_PageUp:    return SDL_SCANCODE_PAGEUP;
    case Qt::Key_PageDown:  return SDL_SCANCODE_PAGEDOWN;
    case Qt::Key_Insert:    return SDL_SCANCODE_INSERT;
    case Qt::Key_Delete:    return SDL_SCANCODE_DELETE;

    // ── Editing / whitespace ─────────────────────────────────────────────────
    case Qt::Key_Return:    return SDL_SCANCODE_RETURN;
    case Qt::Key_Enter:     return SDL_SCANCODE_KP_ENTER;
    case Qt::Key_Escape:    return SDL_SCANCODE_ESCAPE;
    case Qt::Key_Tab:       return SDL_SCANCODE_TAB;
    case Qt::Key_Backspace: return SDL_SCANCODE_BACKSPACE;
    case Qt::Key_Space:     return SDL_SCANCODE_SPACE;

    // ── Modifiers ────────────────────────────────────────────────────────────
    case Qt::Key_Shift:     return SDL_SCANCODE_LSHIFT;
    case Qt::Key_Control:   return SDL_SCANCODE_LCTRL;
    case Qt::Key_Alt:       return SDL_SCANCODE_LALT;
    case Qt::Key_Meta:      return SDL_SCANCODE_LGUI;
    case Qt::Key_CapsLock:  return SDL_SCANCODE_CAPSLOCK;

    // ── Punctuation / symbols (US layout) ────────────────────────────────────
    case Qt::Key_Minus:        return SDL_SCANCODE_MINUS;
    case Qt::Key_Equal:        return SDL_SCANCODE_EQUALS;
    case Qt::Key_BracketLeft:  return SDL_SCANCODE_LEFTBRACKET;
    case Qt::Key_BracketRight: return SDL_SCANCODE_RIGHTBRACKET;
    case Qt::Key_Backslash:    return SDL_SCANCODE_BACKSLASH;
    case Qt::Key_Semicolon:    return SDL_SCANCODE_SEMICOLON;
    case Qt::Key_Apostrophe:   return SDL_SCANCODE_APOSTROPHE;
    case Qt::Key_QuoteLeft:    return SDL_SCANCODE_GRAVE;
    case Qt::Key_Comma:        return SDL_SCANCODE_COMMA;
    case Qt::Key_Period:       return SDL_SCANCODE_PERIOD;
    case Qt::Key_Slash:        return SDL_SCANCODE_SLASH;

    // ── Numpad ───────────────────────────────────────────────────────────────
    case Qt::Key_multiply:  return SDL_SCANCODE_KP_MULTIPLY;
    case Qt::Key_Plus:      return SDL_SCANCODE_KP_PLUS;
    case Qt::Key_Minus + 0x20000000: return SDL_SCANCODE_KP_MINUS; // Qt numpad minus
    case Qt::Key_division:  return SDL_SCANCODE_KP_DIVIDE;

    // ── Print / Scroll / Pause ───────────────────────────────────────────────
    case Qt::Key_Print:     return SDL_SCANCODE_PRINTSCREEN;
    case Qt::Key_ScrollLock:return SDL_SCANCODE_SCROLLLOCK;
    case Qt::Key_Pause:     return SDL_SCANCODE_PAUSE;

    default:                return SDL_SCANCODE_UNKNOWN;
    }
}

// Translate Qt keyboard modifiers to the SDL_Keymod bitmask expected in
// SDL_Keysym::mod.
static SDL_Keymod qtModsToSdlMod(Qt::KeyboardModifiers mods)
{
    int sdlMod = KMOD_NONE;
    if (mods & Qt::ShiftModifier)   sdlMod |= KMOD_LSHIFT;
    if (mods & Qt::ControlModifier) sdlMod |= KMOD_LCTRL;
    if (mods & Qt::AltModifier)     sdlMod |= KMOD_LALT;
    if (mods & Qt::MetaModifier)    sdlMod |= KMOD_LGUI;
    return static_cast<SDL_Keymod>(sdlMod);
}

// Map a Qt mouse button to the SDL button constant.
static Uint8 qtButtonToSdl(Qt::MouseButton btn)
{
    switch (btn)
    {
    case Qt::LeftButton:   return SDL_BUTTON_LEFT;
    case Qt::RightButton:  return SDL_BUTTON_RIGHT;
    case Qt::MiddleButton: return SDL_BUTTON_MIDDLE;
    case Qt::XButton1:     return SDL_BUTTON_X1;
    case Qt::XButton2:     return SDL_BUTTON_X2;
    default:               return SDL_BUTTON_LEFT;
    }
}

// Build the SDL button-state mask (SDL_BUTTON_LMASK etc.) from the Qt button
// set currently held down. Used for SDL_MOUSEMOTION's state field.
static Uint32 qtButtonsToSdlMask(Qt::MouseButtons btns)
{
    Uint32 mask = 0;
    if (btns & Qt::LeftButton)   mask |= SDL_BUTTON_LMASK;
    if (btns & Qt::MiddleButton) mask |= SDL_BUTTON_MMASK;
    if (btns & Qt::RightButton)  mask |= SDL_BUTTON_RMASK;
    if (btns & Qt::XButton1)     mask |= SDL_BUTTON_X1MASK;
    if (btns & Qt::XButton2)     mask |= SDL_BUTTON_X2MASK;
    return mask;
}

// ─────────────────────────────────────────────────────────────────────────────
// Construction / destruction
// ─────────────────────────────────────────────────────────────────────────────

EngineViewportWidget::EngineViewportWidget(QWidget* parent)
    : QWidget(parent)
{
    // Must have a real HWND so we can reparent the SDL window into it.
    setAttribute(Qt::WA_NativeWindow);
    // Stop Qt painting over the bgfx surface.
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_NoSystemBackground);

    // Accept keyboard focus so keyPress/keyRelease events reach this widget.
    setFocusPolicy(Qt::StrongFocus);

    // Receive mouse-move events even when no button is held (needed for
    // camera look/hover logic that depends on continuous cursor deltas).
    setMouseTracking(true);
}

EngineViewportWidget::~EngineViewportWidget()
{
    EngineInstance::get().detachWidget();
}

// ─────────────────────────────────────────────────────────────────────────────
// Lifecycle
// ─────────────────────────────────────────────────────────────────────────────

void EngineViewportWidget::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    // Engine must already be initialized via EngineInstance::get().initialize()
    // called from main() before the first window is shown.
    EngineInstance::get().attachToWidget(this);
    m_lastMousePosValid = false;
}

void EngineViewportWidget::hideEvent(QHideEvent* event)
{
    QWidget::hideEvent(event);
    EngineInstance::get().detachWidget();
    m_lastMousePosValid = false;
}

void EngineViewportWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    if (EngineInstance::get().isInitialized())
        EngineInstance::get().onWidgetResized(event->size().width(), event->size().height());
}

// ─────────────────────────────────────────────────────────────────────────────
// Keyboard
//
// Qt has already consumed the Win32 WM_KEYDOWN/WM_KEYUP messages before
// SDL ever sees them. We rebuild the SDL event from Qt's decoded data and
// push it directly into SDL's queue. SDL_PollEvent in tick() picks it up.
// ─────────────────────────────────────────────────────────────────────────────

void EngineViewportWidget::keyPressEvent(QKeyEvent* event)
{
    QWidget::keyPressEvent(event);

    SDL_Event sdlEvent{};
    sdlEvent.type                   = SDL_KEYDOWN;
    sdlEvent.key.timestamp          = SDL_GetTicks();
    sdlEvent.key.windowID           = EngineInstance::get().sdlWindowId();
    sdlEvent.key.state              = SDL_PRESSED;
    sdlEvent.key.repeat             = static_cast<Uint8>(event->isAutoRepeat() ? 1 : 0);
    sdlEvent.key.keysym.scancode    = qtKeyToSdlScancode(event->key());
    sdlEvent.key.keysym.sym         = SDL_GetKeyFromScancode(sdlEvent.key.keysym.scancode);
    sdlEvent.key.keysym.mod         = static_cast<Uint16>(qtModsToSdlMod(event->modifiers()));
    SDL_PushEvent(&sdlEvent);

    // Also synthesise a SDL_TEXTINPUT so engines that read text input
    // (e.g. ImGui text fields) work without a separate IME integration.
    const QString text = event->text();
    if (!text.isEmpty() && !event->isAutoRepeat())
    {
        SDL_Event textEvent{};
        textEvent.type           = SDL_TEXTINPUT;
        textEvent.text.timestamp = SDL_GetTicks();
        textEvent.text.windowID  = EngineInstance::get().sdlWindowId();
        const QByteArray utf8 = text.toUtf8();
        // SDL_TextInputEvent::text is 32 bytes; guard against overflow.
        qstrncpy(textEvent.text.text, utf8.constData(),
                 sizeof(textEvent.text.text));
        SDL_PushEvent(&textEvent);
    }
}

void EngineViewportWidget::keyReleaseEvent(QKeyEvent* event)
{
    QWidget::keyReleaseEvent(event);

    if (event->isAutoRepeat())
        return; // SDL does not synthesise key-release for repeat; match that.

    SDL_Event sdlEvent{};
    sdlEvent.type                   = SDL_KEYUP;
    sdlEvent.key.timestamp          = SDL_GetTicks();
    sdlEvent.key.windowID           = EngineInstance::get().sdlWindowId();
    sdlEvent.key.state              = SDL_RELEASED;
    sdlEvent.key.repeat             = 0;
    sdlEvent.key.keysym.scancode    = qtKeyToSdlScancode(event->key());
    sdlEvent.key.keysym.sym         = SDL_GetKeyFromScancode(sdlEvent.key.keysym.scancode);
    sdlEvent.key.keysym.mod         = static_cast<Uint16>(qtModsToSdlMod(event->modifiers()));
    SDL_PushEvent(&sdlEvent);
}

// ─────────────────────────────────────────────────────────────────────────────
// Mouse buttons
// ─────────────────────────────────────────────────────────────────────────────

void EngineViewportWidget::mousePressEvent(QMouseEvent* event)
{
    QWidget::mousePressEvent(event);

    // Take keyboard focus so subsequent key events reach this widget.
    setFocus(Qt::MouseFocusReason);

    SDL_Event sdlEvent{};
    sdlEvent.type               = SDL_MOUSEBUTTONDOWN;
    sdlEvent.button.timestamp   = SDL_GetTicks();
    sdlEvent.button.windowID    = EngineInstance::get().sdlWindowId();
    sdlEvent.button.which       = 0;
    sdlEvent.button.button      = qtButtonToSdl(event->button());
    sdlEvent.button.state       = SDL_PRESSED;
    sdlEvent.button.clicks      = 1;
    sdlEvent.button.x           = event->x();
    sdlEvent.button.y           = event->y();
    SDL_PushEvent(&sdlEvent);
}

void EngineViewportWidget::mouseReleaseEvent(QMouseEvent* event)
{
    QWidget::mouseReleaseEvent(event);

    SDL_Event sdlEvent{};
    sdlEvent.type               = SDL_MOUSEBUTTONUP;
    sdlEvent.button.timestamp   = SDL_GetTicks();
    sdlEvent.button.windowID    = EngineInstance::get().sdlWindowId();
    sdlEvent.button.which       = 0;
    sdlEvent.button.button      = qtButtonToSdl(event->button());
    sdlEvent.button.state       = SDL_RELEASED;
    sdlEvent.button.clicks      = 1;
    sdlEvent.button.x           = event->x();
    sdlEvent.button.y           = event->y();
    SDL_PushEvent(&sdlEvent);
}

// ─────────────────────────────────────────────────────────────────────────────
// Mouse motion
//
// We calculate xrel/yrel ourselves from the previous Qt position.
// SDL's own relative-motion tracking is unreliable here because the SDL
// window is a hidden child (or completely bypassed by the event push path).
// ─────────────────────────────────────────────────────────────────────────────

void EngineViewportWidget::mouseMoveEvent(QMouseEvent* event)
{
    QWidget::mouseMoveEvent(event);

    const QPoint pos = event->pos();

    int xrel = 0;
    int yrel = 0;
    if (m_lastMousePosValid)
    {
        xrel = pos.x() - m_lastMousePos.x();
        yrel = pos.y() - m_lastMousePos.y();
    }
    m_lastMousePos      = pos;
    m_lastMousePosValid = true;

    SDL_Event sdlEvent{};
    sdlEvent.type               = SDL_MOUSEMOTION;
    sdlEvent.motion.timestamp   = SDL_GetTicks();
    sdlEvent.motion.windowID    = EngineInstance::get().sdlWindowId();
    sdlEvent.motion.which       = 0;
    sdlEvent.motion.state       = qtButtonsToSdlMask(event->buttons());
    sdlEvent.motion.x           = pos.x();
    sdlEvent.motion.y           = pos.y();
    sdlEvent.motion.xrel        = xrel;
    sdlEvent.motion.yrel        = yrel;
    SDL_PushEvent(&sdlEvent);
}

// ─────────────────────────────────────────────────────────────────────────────
// Mouse wheel
// ─────────────────────────────────────────────────────────────────────────────

void EngineViewportWidget::wheelEvent(QWheelEvent* event)
{
    QWidget::wheelEvent(event);

    const QPoint angle = event->angleDelta();

    if (angle.isNull())
        return;

    SDL_Event sdlEvent{};
    sdlEvent.type = SDL_MOUSEWHEEL;
    sdlEvent.wheel.timestamp = SDL_GetTicks();
    sdlEvent.wheel.windowID = EngineInstance::get().sdlWindowId();
    sdlEvent.wheel.which = 0;
    sdlEvent.wheel.direction = SDL_MOUSEWHEEL_NORMAL;

    // Integer "notches"
    sdlEvent.wheel.x = angle.x() / 120;
    sdlEvent.wheel.y = angle.y() / 120;

    // Precise continuous values (this is the important part)
    sdlEvent.wheel.preciseX = static_cast<float>(angle.x()) / 120.0f;
    sdlEvent.wheel.preciseY = static_cast<float>(angle.y()) / 120.0f;

    SDL_PushEvent(&sdlEvent);
}
