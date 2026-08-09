#pragma once

#include <UI/UiButton.hpp>
#include <UI/UiVerticalBox.hpp>
#include <UI/UiHorizontalBox.hpp>
#include <UI/UiText.hpp>
#include <UI/UiImage.hpp>
#include <UI/UiCanvas.hpp>
#include <UI/UiNavigation.h>
#include "../UiSettingsStyle.hpp"
#include <functional>
#include <string>
#include <unordered_set>
#include <Settings/InputBindingTypes.h>

enum class RebindDevice { Keyboard, Gamepad };

class UiRebindCaptureModal : public UiCanvas
{
public:
    UiRebindCaptureModal(RebindDevice device,
        const std::string& actionDisplayName,
        int slotIndex,
        InputBindingSlotKB currentKB,
        InputBindingSlotGP currentGP,
        std::function<void(InputBindingSlotKB)> onKeyboardCaptured,
        std::function<void(InputBindingSlotGP)> onGamepadCaptured,
        std::function<void()> onCancelled,
        bool isCurrentlyBound = false,
        std::function<void()> onUnbind = nullptr)
        : m_device(device)
        , m_currentKB(currentKB)
        , m_currentGP(currentGP)
        , m_onKB(std::move(onKeyboardCaptured))
        , m_onGP(std::move(onGamepadCaptured))
        , m_onCancel(std::move(onCancelled))
        , m_onUnbind(std::move(onUnbind))
        , m_isBound(isCurrentlyBound)
    {
        FocusTrap = true;

        background = std::make_shared<UiImage>();
        // MATCH COLOR: Adjusted alpha from 0.7f to 0.6f to match UiConfirmDialog
        background->color = vec4(0.f, 0.f, 0.f, 0.6f);
        background->HitCheck = true;
        AddChild(background);

        panel = std::make_shared<UiVerticalBox>();
        // FIX ALIGNMENT: Center the panel container itself within the card
        panel->origin = vec2(0.5f);
        panel->pivot = vec2(0.5f);
        panel->ContentDistance = 14.f;

        title = std::make_shared<UiText>();
        title->text = "${SETTINGS_INPUT_REBIND_TITLE_PREFIX} " + actionDisplayName + " ${SETTINGS_INPUT_REBIND_SLOT_LABEL} " + std::to_string(slotIndex + 1);
        title->fontSize = 42.f;
        title->pivot = vec2(0.5f, 0.f);
        title->origin = vec2(0.5f, 0.f);

        hint = std::make_shared<UiText>();
        hint->text = (device == RebindDevice::Keyboard)
            ? "${SETTINGS_INPUT_REBIND_HINT_KEYBOARD}"
            : "${SETTINGS_INPUT_REBIND_HINT_GAMEPAD}";
        hint->fontSize = 30.f;
        hint->textColor = vec4(0.8f, 0.8f, 0.8f, 1.f);
        hint->pivot = vec2(0.5f, 0.f);
        hint->origin = vec2(0.5f, 0.f);

        buttonsRow = std::make_shared<UiHorizontalBox>();
        // FIX ALIGNMENT: Keep the horizontal row top-aligned for proper vertical stacking
        buttonsRow->origin = vec2(0.5f, 0.f);
        buttonsRow->pivot = vec2(0.5f, 0.f);
        buttonsRow->ContentDistance = 12.f;

        cancelButton = std::make_shared<UiButton>();
        cancelButton->size = vec2(150.f, 50.f);
        auto cancelLabel = std::make_shared<UiText>();
        cancelLabel->text = "${SETTINGS_CANCEL}";
        cancelLabel->fontSize = SettingsStyle::ButtonLabelSize;
        cancelLabel->pivot = vec2(0.5f);
        cancelLabel->origin = vec2(0.5f);
        cancelButton->AddChild(cancelLabel);
        cancelButton->onClick = [this]() { Cancel(); };
        buttonsRow->AddChild(cancelButton);

        if (m_isBound)
        {
            unbindButton = std::make_shared<UiButton>();
            unbindButton->size = vec2(150.f, 50.f);
            unbindButton->Color = SettingsStyle::DangerFill; // Assuming you still want Unbind to be red
            unbindButton->HoverColor = SettingsStyle::DangerHover;
            auto unbindLabel = std::make_shared<UiText>();
            unbindLabel->text = "${SETTINGS_INPUT_REBIND_UNBIND_BUTTON}";
            unbindLabel->fontSize = SettingsStyle::ButtonLabelSize;
            unbindLabel->pivot = vec2(0.5f);
            unbindLabel->origin = vec2(0.5f);
            unbindButton->AddChild(unbindLabel);
            unbindButton->onClick = [this]() { Unbind(); };
            buttonsRow->AddChild(unbindButton);

            cancelButton->NavRight = unbindButton;
            unbindButton->NavLeft = cancelButton;
        }

        panel->AddChild(title);

        // FIX ALIGNMENT & COLOR: Explicitly anchor the divider and use ConfirmAccent
        auto divider = MakeDivider(360.f, SettingsStyle::ConfirmAccent, 2.f);
        divider->origin = vec2(0.5f, 0.f);
        divider->pivot = vec2(0.5f, 0.f);
        panel->AddChild(divider);

        panel->AddChild(hint);
        // buttonsRow is purposefully NOT added yet. It is revealed only if they press the currently bound key.

		float cardWidth = 460.f;

        float textWidth = title->GetSize().x;

		if (cardWidth < textWidth + 40.f)
		{
			cardWidth = textWidth + 40.f;
		}

        // MATCH COLOR: Swapped CaptureFill and CaptureBorder for ConfirmFill and ConfirmBorder
        auto card = std::make_shared<UiCardPanel>(vec2(cardWidth, 260.f), panel,
            SettingsStyle::ConfirmFill, SettingsStyle::ConfirmBorder);
        card->origin = vec2(0.5f);
        card->pivot = vec2(0.5f);
        AddChild(card);

        // ── Baseline snapshot ────────────────────────────────────────────────
        m_baselineKeys = Input::activeKeys;
        m_baselineMouseButtons = Input::activeMouseButtons;
        m_baselineJoy = Input::activeJoystickButtons;
        m_baselineLT = Input::leftTriggerAxis > kTriggerThreshold;
        m_baselineRT = Input::rightTriggerAxis > kTriggerThreshold;

        UiNavigation::SetFocus(nullptr);
    }

    void OnNavCancel() override { Cancel(); }

    void FinalizeChildren() override
    {
        background->size = GetSize();
        UiCanvas::FinalizeChildren();
    }

    void Update() override
    {

        // If we are showing the prompt, we wait for UI interaction.
        if (m_askingUnbind)
        {
            UiCanvas::Update();
            if (Input::GetAction("ui_cancel")->Pressed())
            {
                Cancel();
            }
            return;
        }

        // The reliable, always-safe way out 
        if (Input::GetAction("ui_cancel")->Pressed())
        {
            Cancel();
            return;
        }

        UiCanvas::Update();
        if (m_resolved) return;

        if (m_device == RebindDevice::Keyboard)
        {
            // Cancel if gamepad input is registered instead
            if (HasGamepadInput())
            {
                Cancel();
                return;
            }
            PollKeyboard();
        }
        else
        {
            // Cancel if keyboard or mouse input is registered instead
            if (HasKeyboardMouseInput())
            {
                Cancel();
                return;
            }
            PollGamepad();
        }

    }

private:
    static constexpr float kTriggerThreshold = 0.5f;

    RebindDevice m_device;
    InputBindingSlotKB m_currentKB;
    InputBindingSlotGP m_currentGP;

    std::function<void(InputBindingSlotKB)> m_onKB;
    std::function<void(InputBindingSlotGP)> m_onGP;
    std::function<void()> m_onCancel;
    std::function<void()> m_onUnbind;
    bool m_isBound = false;
    bool m_resolved = false;
    bool m_askingUnbind = false;

    std::unordered_set<SDL_Scancode> m_baselineKeys;
    std::unordered_set<uint8_t> m_baselineMouseButtons;
    std::unordered_set<int> m_baselineJoy;
    bool m_baselineLT = false, m_baselineRT = false;

    std::shared_ptr<UiImage> background;
    std::shared_ptr<UiVerticalBox> panel;
    std::shared_ptr<UiText> title;
    std::shared_ptr<UiText> hint;
    std::shared_ptr<UiHorizontalBox> buttonsRow;
    std::shared_ptr<UiButton> cancelButton;
    std::shared_ptr<UiButton> unbindButton;

    void Cancel()
    {
        if (m_resolved) return;
        m_resolved = true;
        if (m_onCancel) m_onCancel();
        RemoveFromParent();
    }

    void Unbind()
    {
        if (m_resolved) return;
        m_resolved = true;
        if (m_onUnbind) m_onUnbind();
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

    void ShowPrompt()
    {
        if (m_askingUnbind) return;
        m_askingUnbind = true;

        hint->text = "${SETTINGS_INPUT_REBIND_ALREADY_BOUND_PROMPT}";
        hint->textColor = vec4(1.0f, 0.8f, 0.2f, 1.0f); // Warning color

        panel->AddChild(buttonsRow);
        UiNavigation::SetFocus(cancelButton.get());
    }

    bool HasGamepadInput() const
    {
        for (int btn : Input::activeJoystickButtons)
        {
            if (m_baselineJoy.count(btn) == 0) return true;
        }
        if (Input::leftTriggerAxis > kTriggerThreshold && !m_baselineLT) return true;
        if (Input::rightTriggerAxis > kTriggerThreshold && !m_baselineRT) return true;
        return false;
    }

    bool HasKeyboardMouseInput() const
    {
        for (SDL_Scancode sc : Input::activeKeys)
        {
            if (m_baselineKeys.count(sc) == 0) return true;
        }
        for (uint8_t button : Input::activeMouseButtons)
        {
            if (m_baselineMouseButtons.count(button) == 0) return true;
        }
        return false;
    }

    void PollKeyboard()
    {
        for (SDL_Scancode sc : Input::activeKeys)
        {
            if (m_baselineKeys.count(sc)) continue;
            InputBindingSlotKB slot = InputBindingSlotKB::FromKey(sc);

            if (m_isBound && slot == m_currentKB) {
                ShowPrompt();
                return;
            }

            ResolveKB(slot);
            return;
        }
        for (uint8_t button : Input::activeMouseButtons)
        {
            if (m_baselineMouseButtons.count(button)) continue;
            InputBindingSlotKB slot = InputBindingSlotKB::FromMouseButton(button);

            if (m_isBound && slot == m_currentKB) {
                ShowPrompt();
                return;
            }

            ResolveKB(slot);
            return;
        }
    }

    void PollGamepad()
    {
        for (int btn : Input::activeJoystickButtons)
        {
            if (m_baselineJoy.count(btn)) continue;
            GamepadButton gb = static_cast<GamepadButton>(btn);
            InputBindingSlotGP slot = InputBindingSlotGP::FromButton(gb);

            if (m_isBound && slot == m_currentGP) {
                ShowPrompt();
                return;
            }

            ResolveGP(slot);
            return;
        }
        if (Input::leftTriggerAxis > kTriggerThreshold && !m_baselineLT)
        {
            InputBindingSlotGP slot = InputBindingSlotGP::FromButton(GamepadButton::LeftTrigger);
            if (m_isBound && slot == m_currentGP) { ShowPrompt(); return; }
            ResolveGP(slot);
            return;
        }
        if (Input::rightTriggerAxis > kTriggerThreshold && !m_baselineRT)
        {
            InputBindingSlotGP slot = InputBindingSlotGP::FromButton(GamepadButton::RightTrigger);
            if (m_isBound && slot == m_currentGP) { ShowPrompt(); return; }
            ResolveGP(slot);
            return;
        }
    }
};