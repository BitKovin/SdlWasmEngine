#pragma once

#include <QObject>
#include <QTimer>
#include <SDL2/SDL_stdinc.h>   // Uint32

struct SDL_Window;
class EngineMain;
class EngineViewportWidget;

// ─────────────────────────────────────────────────────────────────────────────
// EngineInstance — singleton that owns SDL, bgfx, and EngineMain.
// Lives for the entire application lifetime regardless of which widgets
// are shown or destroyed. Call initialize() once from main() before
// any window is shown.
//
// INPUT ARCHITECTURE (Windows)
// ─────────────────────────────
// The SDL child HWND sits on top of the Qt widget and receives all Win32
// mouse/keyboard messages directly.  SDL itself drops those events because it
// considers the window hidden/unfocused internally.
//
// installInputHook() subclasses the SDL HWND's WndProc the moment the window
// is first reparented.  Our proc intercepts every input message, translates it
// to an SDL_Event, and pushes it via SDL_PushEvent.  tick()'s SDL_PollEvent
// loop picks them up exactly as if they had come from a normal SDL window.
// ─────────────────────────────────────────────────────────────────────────────
class EngineInstance : public QObject
{
    Q_OBJECT

public:
    static EngineInstance& get();

    // Boot SDL, bgfx, and EngineMain. Safe to call only once.
    void initialize();

    // Called by EngineViewportWidget when it becomes visible.
    void attachToWidget(EngineViewportWidget* widget);

    // Called by EngineViewportWidget when it is hidden or destroyed.
    void detachWidget();

    // Called by EngineViewportWidget from its resizeEvent.
    void onWidgetResized(int w, int h);

    // Kept for source compatibility; no longer needed.
    void focusSdlWindow() {}

    EngineMain*  engine()        const { return m_engine; }
    bool         isInitialized() const { return m_initialized; }
    SDL_Window*  sdlWindow()     const { return m_sdlWindow; }
    Uint32       sdlWindowId()   const;

private slots:
    void tick();

private:
    EngineInstance() = default;
    ~EngineInstance();

    void initSdl (int w, int h);
    void initBgfx(int w, int h);
    void installInputHook();   // called once from attachToWidget

    SDL_Window*           m_sdlWindow    = nullptr;
    EngineMain*           m_engine       = nullptr;
    QTimer*               m_timer        = nullptr;
    EngineViewportWidget* m_activeWidget = nullptr;
    bool                  m_initialized  = false;
    bool                  m_hookInstalled = false;

};