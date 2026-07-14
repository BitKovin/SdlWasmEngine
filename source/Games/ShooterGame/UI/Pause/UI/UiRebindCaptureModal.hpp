#pragma once

#include "UiButton.hpp"
#include "UiVerticalBox.hpp"
#include "UiText.hpp"
#include "UiImage.hpp"
#include "UiCanvas.hpp"
#include "UiNavigation.h"

#include <functional>
#include <string>
#include <unordered_set>

#include "../Settings/InputBindingTypes.h"

enum class RebindDevice { Keyboard, Gamepad };

// ---------------------------------------------------------------------------
// UiRebindCaptureModal
//
// "Press a key or button..." prompt. Fully nav-capable like every other
// screen in this UI: FocusTrap=true, a focusable Cancel button, and
// OnNavCancel wired the same way UiSettingsMenu/UiVideoSettings/
// UiConfirmDialog all do it.
//
// The actual capture is raw-input polling (it has to be — we're listening
// for an arbitrary key/button, not a named action), diffed against a
// baseline snapshot taken the instant the modal opens so already-held keys
// don't immediately "capture". Whatever is currently bound to the
// "ui_cancel" action is reserved and skipped by the poll, so opening this
// modal and immediately pressing Escape/B always cancels rather than binding
// Escape/B to the action being rebound.
// ---------------------------------------------------------------------------

class UiRebindCaptureModal : public UiCanvas
{
public:
    UiRebindCaptureModal(RebindDevice device,
                          const std::string& actionDisplayName,
                          int slotIndex,
                          std::function<void(InputBindingSlotKB)> onKeyboardCaptured,
                          std::function<void(InputBindingSlotGP)> onGamepadCaptured,
                          std::function<void()> onCancelled)
        : m_device(device)
        , m_onKB(std::move(onKeyboardCaptured))
        , m_onGP(std::move(onGamepadCaptured))
        , m_onCancel(std::move(onCancelled))
    {
        FocusTrap = true;

        background = std::make_shared<UiImage>();
        background->color = vec4(0.f, 0.f, 0.f, 0.7f);
        AddChild(background);

        panel = std::make_shared<UiVerticalBox>();
        panel->origin = vec2(0.5f);
        panel->pivot = vec2(0.5f);
        panel->ContentDistance = 14.f;

        title = std::make_shared<UiText>();
        title->text = "Rebind " + actionDisplayName + " — Slot " + std::to_string(slotIndex + 1);
        title->fontSize = 32.f;
        title->pivot = vec2(0.5f, 0.f);
        title->origin = vec2(0.5f, 0.f);

        hint = std::make_shared<UiText>();
        hint->text = (device == RebindDevice::Keyboard)
            ? "Press a key or mouse button..."
            : "Press a gamepad button or trigger...";
        hint->fontSize = 22.f;
        hint->textColor = vec4(0.8f, 0.8f, 0.8f, 1.f);
        hint->pivot = vec2(0.5f, 0.f);
        hint->origin = vec2(0.5f, 0.f);

        cancelButton = std::make_shared<UiButton>();
        cancelButton->size = vec2(160.f, 52.f);
        auto cancelLabel = std::make_shared<UiText>();
        cancelLabel->text = "Cancel";
        cancelLabel->pivot = vec2(0.5f);
        cancelLabel->origin = vec2(0.5f);
        cancelButton->AddChild(cancelLabel);
        cancelButton->onClick = [this]() { Cancel(); };

        panel->AddChild(title);
        panel->AddChild(hint);
        panel->AddChild(cancelButton);

        AddChild(panel);

        // ── Baseline snapshot ────────────────────────────────────────────────
        m_baselineKeys         = Input::activeKeys;
        m_baselineMouseButtons = Input::activeMouseButtons;
        m_baselineJoy           = Input::activeJoystickButtons;
        m_baselineLT            = Input::leftTriggerAxis  > kTriggerThreshold;
        m_baselineRT            = Input::rightTriggerAxis > kTriggerThreshold;

        UiNavigation::SetFocus(cancelButton.get());
    }

    void OnNavCancel() override { Cancel(); }

    void FinalizeChildren() override
    {
        background->size = GetSize();
        UiCanvas::FinalizeChildren();
    }

    void Update() override
    {
        // The reliable, always-safe way out — whatever key/button currently
        // means "cancel" anywhere else in the UI means it here too.
        if (Input::GetAction("ui_cancel")->Pressed())
        {
            Cancel();
            return;
        }

        if (m_device == RebindDevice::Keyboard) PollKeyboard();
        else                                    PollGamepad();

        if (m_resolved) return;

        UiCanvas::Update();
    }

private:
    static constexpr float kTriggerThreshold = 0.5f;

    RebindDevice m_device;
    std::function<void(InputBindingSlotKB)> m_onKB;
    std::function<void(InputBindingSlotGP)> m_onGP;
    std::function<void()> m_onCancel;
    bool m_resolved = false;

    std::unordered_set<SDL_Scancode> m_baselineKeys;
    std::unordered_set<uint8_t> m_baselineMouseButtons;
    std::unordered_set<int> m_baselineJoy;
    bool m_baselineLT = false, m_baselineRT = false;

    std::shared_ptr<UiImage> background;
    std::shared_ptr<UiVerticalBox> panel;
    std::shared_ptr<UiText> title;
    std::shared_ptr<UiText> hint;
    std::shared_ptr<UiButton> cancelButton;

    void Cancel()
    {
        if (m_resolved) return;
        m_resolved = true;
        if (m_onCancel) m_onCancel();
        RemoveFromParent();
    }

    void ResolveKB(InputBindingSlotKB slot)
    {
        if (m_resolved) return;
        m_resolved = true;
        if (m_onKB) m_onKB(slot);
        RemoveFromParent();
    }

    void ResolveGP(InputBindingSlotGP slot)
    {
        if (m_resolved) return;
        m_resolved = true;
        if (m_onGP) m_onGP(slot);
        RemoveFromParent();
    }

    // Whatever is presently bound to "ui_cancel" is off-limits to capture —
    // otherwise pressing Escape/B to back out would instead rebind Escape/B.
    bool IsReservedKey(SDL_Scancode sc) const
    {
        InputAction* cancelAction = Input::GetAction("ui_cancel");
        for (SDL_Scancode k : cancelAction->keys)
            if (k == sc) return true;
        return false;
    }

    bool IsReservedMouseButton(uint8_t button) const
    {
        InputAction* cancelAction = Input::GetAction("ui_cancel");
        for (uint8_t b : cancelAction->mouseButtons)
            if (b == button) return true;
        return false;
    }

    bool IsReservedButton(GamepadButton btn) const
    {
        InputAction* cancelAction = Input::GetAction("ui_cancel");
        for (GamepadButton b : cancelAction->buttons)
            if (b == btn) return true;
        return false;
    }

    void PollKeyboard()
    {
        for (SDL_Scancode sc : Input::activeKeys)
        {
            if (m_baselineKeys.count(sc)) continue;
            if (IsReservedKey(sc)) continue;
            ResolveKB(InputBindingSlotKB::FromKey(sc));
            return;
        }
        for (uint8_t button : Input::activeMouseButtons)
        {
            if (m_baselineMouseButtons.count(button)) continue;
            if (IsReservedMouseButton(button)) continue;
            ResolveKB(InputBindingSlotKB::FromMouseButton(button));
            return;
        }
    }

    void PollGamepad()
    {
        for (int btn : Input::activeJoystickButtons)
        {
            if (m_baselineJoy.count(btn)) continue;
            GamepadButton gb = static_cast<GamepadButton>(btn);
            if (IsReservedButton(gb)) continue;
            ResolveGP(InputBindingSlotGP::FromButton(gb));
            return;
        }
        if (Input::leftTriggerAxis > kTriggerThreshold && !m_baselineLT && !IsReservedButton(GamepadButton::LeftTrigger))
        {
            ResolveGP(InputBindingSlotGP::FromButton(GamepadButton::LeftTrigger));
            return;
        }
        if (Input::rightTriggerAxis > kTriggerThreshold && !m_baselineRT && !IsReservedButton(GamepadButton::RightTrigger))
        {
            ResolveGP(InputBindingSlotGP::FromButton(GamepadButton::RightTrigger));
            return;
        }
    }
};
