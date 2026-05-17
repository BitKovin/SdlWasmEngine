#pragma once

#include <QWidget>
#include <QPoint>

// ─────────────────────────────────────────────────────────────────────────────
// EngineViewportWidget — a thin Qt widget that hosts the engine's SDL/bgfx
// surface. The engine itself (SDL, bgfx, EngineMain) lives in EngineInstance
// and is never destroyed when this widget is.
//
// INPUT ARCHITECTURE
// ──────────────────
// Qt drains the Win32 message queue before QTimer fires, so SDL's WndProc
// never sees keyboard/mouse messages. The fix is to bypass SDL's message
// pump entirely: every Qt input event is translated here and pushed into
// SDL's queue via SDL_PushEvent. The existing SDL_PollEvent loop in
// EngineInstance::tick() picks them up normally — nothing else changes.
//
// HOW TO USE IN QT DESIGNER:
//   1. Drop a QWidget placeholder where you want the viewport.
//   2. Right-click it → "Promote to..."
//   3. Base class name : QWidget
//      Promoted class  : EngineViewportWidget
//      Header file     : EngineViewportWidget.h
//   4. Click Add → Promote.
// ─────────────────────────────────────────────────────────────────────────────
class EngineViewportWidget : public QWidget
{
    Q_OBJECT

public:
    explicit EngineViewportWidget(QWidget* parent = nullptr);
    ~EngineViewportWidget() override;

protected:
    // ── Lifecycle ────────────────────────────────────────────────────────────
    void showEvent  (QShowEvent*   event) override;
    void hideEvent  (QHideEvent*   event) override;
    void resizeEvent(QResizeEvent* event) override;

    // ── Keyboard ─────────────────────────────────────────────────────────────
    void keyPressEvent  (QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;

    // ── Mouse ────────────────────────────────────────────────────────────────
    void mousePressEvent  (QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent   (QMouseEvent* event) override;
    void wheelEvent       (QWheelEvent* event) override;

    // Prevent Qt from painting over the native bgfx surface.
    QPaintEngine* paintEngine() const override { return nullptr; }

private:
    // Tracks previous cursor position inside the widget so we can compute
    // xrel/yrel for SDL_MOUSEMOTION without relying on SDL's own tracking.
    QPoint m_lastMousePos;
    bool   m_lastMousePosValid = false;
};
