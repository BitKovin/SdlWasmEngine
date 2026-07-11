#pragma once

#include "UiProgressBar.hpp"
#include "UiButton.hpp"
#include "../Input.h"

#include <algorithm>
#include <cmath>

// ---------------------------------------------------------------------------
// UiSlider
//
// A UiProgressBar with an optional draggable handle layered on top, turning
// the progress display into an interactive control.
//
//  Layout:
//    UiSlider (UiProgressBar)   — track background + fill, and the real
//                                 keyboard/gamepad nav target
//      └─ UiButton  m_handle    — draggable thumb. DisableFocus=true so it
//                                 never competes with the slider itself for
//                                 nav focus — the same trick UiScrollRegion
//                                 uses for its scrollbar thumb.
//
//  ShowHandle = false → HANDLE-LESS, READ-ONLY MODE
//    When ShowHandle is false, both the slider and its handle stop taking
//    HitCheck (no clicking, dragging, hovering, or keyboard focus) and the
//    element is purely the inherited UiProgressBar visual: a background plus
//    a fill proportional to Value. Flip ShowHandle back to true to make it
//    interactive again — e.g. a health bar that becomes an editable slider
//    in a debug/cheat menu.
//
//  INTERACTION
//    Press-and-drag anywhere on the slider (the track OR the handle) maps
//    the pointer position directly onto [MinValue, MaxValue] for as long as
//    the touch/click is held — the value always matches the pointer 1:1,
//    which is the standard feel for a slider (as opposed to a scrollbar,
//    where relative dragging is more appropriate).
//
//  KEYBOARD
//    Horizontal sliders: Left/Right adjust Value by Step (or KeyboardStep of
//    the range if Step == 0) and are consumed; Up/Down fall through to
//    spatial navigation so the slider behaves like a normal form control.
//    Vertical sliders: same, but on Up/Down, with Left/Right falling through.
//
//  DIRECTION
//    Update() keeps UiProgressBar::Direction synced to Vertical each frame
//    (BottomToTop when vertical, LeftToRight when horizontal), so the fill
//    always grows toward higher value the same way the handle moves.
//
//  NOTE ON COLOR INHERITANCE
//    UiProgressBar repurposes the inherited `color` field as the fill tint
//    (see UiProgressBar::Draw). The handle opts out of inheritParentColor so
//    it isn't tinted by the fill.
// ---------------------------------------------------------------------------

class UiSlider : public UiProgressBar
{
public:
    // ── Range ────────────────────────────────────────────────────────────────
    float MinValue     = 0.f;
    float MaxValue     = 1.f;
    float Value        = 0.f;
    float Step         = 0.f;   // 0 = continuous; >0 snaps to increments of Step
    float KeyboardStep = 0.1f;  // fraction of the range moved per key press when Step == 0

    bool Vertical = false;      // false = horizontal (left→right), true = vertical (bottom→top)

    // ── Handle ───────────────────────────────────────────────────────────────
    bool        ShowHandle = true;
    vec2        HandleSize = vec2(20.f, 20.f);
    std::string HandleImage = "GameData/textures/generic/white.png";
    vec4        HandleColor       = vec4(0.90f, 0.90f, 0.90f, 1.f);
    vec4        HandleHoverColor  = vec4(1.00f, 1.00f, 1.00f, 1.f);
    vec4        HandleActiveColor = vec4(1.00f, 0.80f, 0.30f, 1.f);

    // ── Callbacks ────────────────────────────────────────────────────────────
    std::function<void(float)> onValueChanged = nullptr; // fires as the value moves
    std::function<void(float)> onDragEnd      = nullptr; // fires once, on release

    // ── Construction ─────────────────────────────────────────────────────────
    UiSlider()
    {
        BackgroundImage = "GameData/textures/generic/white.png";
        ProgressImage   = "GameData/textures/generic/white.png";
        BackgroundColor = vec4(0.15f, 0.15f, 0.15f, 1.f);
        color           = vec4(0.40f, 0.65f, 1.00f, 1.f); // fill tint (see class comment)

        HitCheck = true;

        m_handle = std::make_shared<UiButton>();
        m_handle->HitCheck = true;
        m_handle->DisableFocus = true;      // slider itself is the nav target, not the handle
        m_handle->inheritParentColor = false;
        m_handle->origin = vec2(0.f);
        m_handle->pivot = vec2(0.5f, 0.5f);
        m_handle->Color = HandleColor;
        m_handle->HoverColor = HandleHoverColor;
        m_handle->ImagePath = HandleImage;
        UiElement::AddChild(m_handle);
    }

    // ── Nav callback ─────────────────────────────────────────────────────────
    bool OnNav(UiNavDir dir) override
    {
        if (!ShowHandle) return false;

        const float step = (Step > 0.f) ? Step : (KeyboardStep * (MaxValue - MinValue));

        if (!Vertical)
        {
            if (dir == UiNavDir::Left)  { ApplyValue(Value - step); return true; }
            if (dir == UiNavDir::Right) { ApplyValue(Value + step); return true; }
        }
        else
        {
            if (dir == UiNavDir::Up)   { ApplyValue(Value + step); return true; }
            if (dir == UiNavDir::Down) { ApplyValue(Value - step); return true; }
        }
        return false;
    }

    // ── Update ────────────────────────────────────────────────────────────────
    void Update() override
    {
        Value = std::clamp(Value, MinValue, MaxValue);

        HitCheck = ShowHandle;
        m_handle->HitCheck = ShowHandle;
        m_handle->visible = ShowHandle;

        if (ShowHandle)
        {
            UpdateDrag();
        }
        else if (m_drag.active)
        {
            m_drag = {};
            if (onDragEnd) onDragEnd(Value);
        }

        // Keep the inherited fill direction matching this slider's own axis
        // — "up"/"right" both mean higher value, same convention as
        // SetValueFromPointer below.
        Direction = Vertical ? UiProgressDirection::BottomToTop : UiProgressDirection::LeftToRight;

        Progress = (MaxValue > MinValue) ? (Value - MinValue) / (MaxValue - MinValue) : 0.f;

        LayoutHandle();

        UiProgressBar::Update();
    }

private:
    std::shared_ptr<UiButton> m_handle;

    struct Drag
    {
        bool active  = false;
        int  touchId = -1;
    } m_drag;

    void ApplyValue(float v)
    {
        v = std::clamp(v, MinValue, MaxValue);
        if (Step > 0.f)
            v = std::clamp(MinValue + std::round((v - MinValue) / Step) * Step, MinValue, MaxValue);

        if (v == Value) return;
        Value = v;
        if (onValueChanged) onValueChanged(Value);
    }

    void SetValueFromPointer(vec2 screenPos)
    {
        const glm::vec2 local = TransformPoint(glm::inverse(worldMatrix), screenPos);
        const float trackLen = Vertical ? size.y : size.x;
        if (trackLen <= 0.f) return;

        float t = std::clamp((Vertical ? local.y : local.x) / trackLen, 0.f, 1.f);
        if (Vertical) t = 1.f - t; // local Y grows downward; up should mean a higher value

        ApplyValue(MinValue + t * (MaxValue - MinValue));
    }

    // A press lands on either the slider itself (the track, since the handle
    // — being a more specific child hit — wins whenever the point is over it)
    // or directly on the handle. Exactly one of these will have it.
    TouchEvent FindPress() const
    {
        for (const auto& e : TouchEvents)
            if (e.pressed) return e;
        for (const auto& e : m_handle->TouchEvents)
            if (e.pressed) return e;

        TouchEvent none;
        none.id = -1;
        return none;
    }

    void UpdateDrag()
    {
        if (!m_drag.active)
        {
            const TouchEvent press = FindPress();
            if (press.id == -1) return;

            m_drag = { true, press.id };
            SetValueFromPointer(press.position);
            return;
        }

        const TouchEvent touch = Input::GetTouchEventFromId(m_drag.touchId);

        if (touch.released || (!touch.pressed && !Input::IsTouchEventHolding(m_drag.touchId)))
        {
            m_drag = {};
            if (onDragEnd) onDragEnd(Value);
            return;
        }

        SetValueFromPointer(touch.position);
    }

    void LayoutHandle()
    {
        const float t = (MaxValue > MinValue) ? (Value - MinValue) / (MaxValue - MinValue) : 0.f;

        m_handle->position = Vertical
            ? vec2(size.x * 0.5f, (1.f - t) * size.y)
            : vec2(t * size.x, size.y * 0.5f);

        m_handle->size = HandleSize;
        m_handle->ImagePath = HandleImage;
        m_handle->Color = m_drag.active ? HandleActiveColor : HandleColor;
        m_handle->HoverColor = HandleHoverColor;
    }
};
