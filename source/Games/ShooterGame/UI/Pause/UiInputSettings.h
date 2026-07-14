#pragma once

#include <UI/UiButton.hpp>
#include <UI/UiVerticalBox.hpp>
#include <UI/UiHorizontalBox.hpp>
#include <UI/UiText.hpp>
#include <UI/UiImage.hpp>
#include <UI/UiCanvas.hpp>
#include <UI/UiSlider.hpp>
#include <UI/UiTextBox.hpp>
#include <UI/UiScrollRegion.hpp>

#include <PauseGameManager.hpp>

#include "Settings/GameSettings.h"
#include "UI/InputIconLibrary.h"
#include "UI/UiConfirmDialog.hpp"
#include "UI/UiBindSlotButton.hpp"
#include "UI/UiRebindCaptureModal.hpp"

#include <sstream>
#include <iomanip>
#include <algorithm>
#include <unordered_map>

// ---------------------------------------------------------------------------
// UiInputSettings
//
// Same shape as UiVideoSettings/UiSettingsMenu (UiCanvas, FocusTrap, a
// backButton wired to both onClick and OnNavCancel), plus:
//
//   - a sensitivity section (slider + numeric textbox, kept in sync)
//   - a scrollable, category-grouped rebind list, 2 keyboard/mouse + 2
//     gamepad slots per action, each slot a UiBindSlotButton
//   - a per-row Reset and a global "Reset to Defaults" (confirmed via
//     UiConfirmDialog)
//
// Everything reads/writes GameSettings::Instance().Input directly; this
// screen owns no settings state of its own. Every mutation immediately
// calls ApplyToEngine() (so it's live right away) and SaveToFile() (so it
// survives a crash/relaunch without needing an explicit "Apply" step).
// ---------------------------------------------------------------------------

class UiInputSettings : public UiCanvas
{
public:

    std::shared_ptr<UiElement> parentMenu;

    UiInputSettings(std::shared_ptr<UiElement> parentMenu_)
    {
        FocusTrap = true;
        this->parentMenu = parentMenu_;

        background = std::make_shared<UiImage>();
        background->color = vec4(0.5f, 0.0f, 0.0f, 0.5f);
        AddChild(background);

        rootBox = std::make_shared<UiVerticalBox>();
        rootBox->origin = vec2(0.5f);
        rootBox->pivot = vec2(0.5f);
        rootBox->ContentDistance = 18.f;

        titleText = std::make_shared<UiText>();
        titleText->text = "Input Settings";
        //titleText->fontSize = 40.f;
        titleText->pivot = vec2(0.5f, 0.f);
        titleText->origin = vec2(0.5f, 0.f);

        BuildSensitivitySection();
        BuildBindingsSection();

        buttonsRow = std::make_shared<UiHorizontalBox>();
        buttonsRow->origin = vec2(0.5f, 0);
        buttonsRow->pivot = vec2(0.5f, 0);
        buttonsRow->ContentDistance = 16.f;

        resetButton = MakeButton("Reset to Defaults");
        backButton = MakeButton("Back");
        buttonsRow->AddChild(resetButton);
        buttonsRow->AddChild(backButton);

        rootBox->AddChild(titleText);
        rootBox->AddChild(sensitivityBox);
        rootBox->AddChild(scrollRegion);
        rootBox->AddChild(buttonsRow);

        AddChild(rootBox);

        backButton->onClick = [this]()
        {
            parentMenu->visible = true;
            RemoveFromParent();
        };

        resetButton->onClick = [this]() { ConfirmResetAll(); };

        UpdateChildrenOffsetRecursive();
    }

    void OnNavCancel() override { backButton->onClick(); }

    void FinalizeChildren() override
    {
        background->size = GetSize();
        UiCanvas::FinalizeChildren();
    }

private:

    std::shared_ptr<UiImage> background;
    std::shared_ptr<UiVerticalBox> rootBox;
    std::shared_ptr<UiText> titleText;

    // ── Sensitivity ──────────────────────────────────────────────────────────
    std::shared_ptr<UiVerticalBox> sensitivityBox;
    std::shared_ptr<UiSlider> mouseSensSlider;
    std::shared_ptr<UiTextBox> mouseSensText;
    std::shared_ptr<UiSlider> gpSensSlider;
    std::shared_ptr<UiTextBox> gpSensText;
    std::shared_ptr<UiButton> invertYButton;
    std::shared_ptr<UiText> invertYLabel;

    // ── Bindings ─────────────────────────────────────────────────────────────
    std::shared_ptr<UiScrollRegion> scrollRegion;

    struct RowWidgets
    {
        std::shared_ptr<UiBindSlotButton> kb[2];
        std::shared_ptr<UiBindSlotButton> gp[2];
    };
    std::unordered_map<std::string, RowWidgets> rowWidgets;

    std::shared_ptr<UiHorizontalBox> buttonsRow;
    std::shared_ptr<UiButton> resetButton;
    std::shared_ptr<UiButton> backButton;

    // ───────────────────────────────────────────────────────────────────────
    // Sensitivity section
    // ───────────────────────────────────────────────────────────────────────

    void BuildSensitivitySection()
    {
        sensitivityBox = std::make_shared<UiVerticalBox>();
        sensitivityBox->origin = vec2(0.5f,0);
        sensitivityBox->pivot = vec2(0.5f,0);
        sensitivityBox->ContentDistance = 10.f;

        InputSensitivitySettings& sens = GameSettings::Instance().Input.Sensitivity;

        // ── Mouse sensitivity ────────────────────────────────────────────────
        mouseSensSlider = std::make_shared<UiSlider>();
        mouseSensSlider->MinValue = InputSensitivitySettings::MinMouseSensitivity;
        mouseSensSlider->MaxValue = InputSensitivitySettings::MaxMouseSensitivity;
        mouseSensSlider->Value = sens.MouseSensitivity;
        mouseSensSlider->Step = 0.01f;
        mouseSensSlider->size = vec2(360.f, 28.f);

        mouseSensText = std::make_shared<UiTextBox>();
        mouseSensText->NumericOnly = true;
        mouseSensText->SetText(FormatFloat(sens.MouseSensitivity));
        mouseSensText->size = vec2(90.f, 40.f);

        mouseSensSlider->onValueChanged = [this](float v)
        {
            GameSettings::Instance().Input.Sensitivity.MouseSensitivity = v;
            GameSettings::Instance().Input.Sensitivity.ApplyToEngine();
            mouseSensText->SetText(FormatFloat(v));
        };
        mouseSensSlider->onDragEnd = [this](float) { GameSettings::Instance().SaveToFile(); };

        mouseSensText->onSubmit = [this](const std::string& text)
        {
            float v = ParseFloatClamped(text, InputSensitivitySettings::MinMouseSensitivity, InputSensitivitySettings::MaxMouseSensitivity);
            GameSettings::Instance().Input.Sensitivity.MouseSensitivity = v;
            GameSettings::Instance().Input.Sensitivity.ApplyToEngine();
            mouseSensSlider->Value = v;
            mouseSensText->SetText(FormatFloat(v));
            GameSettings::Instance().SaveToFile();
        };

        sensitivityBox->AddChild(GetSliderRow("Mouse Sensitivity", mouseSensSlider, mouseSensText));

        // ── Gamepad look sensitivity ─────────────────────────────────────────
        gpSensSlider = std::make_shared<UiSlider>();
        gpSensSlider->MinValue = InputSensitivitySettings::MinGamepadSensitivity;
        gpSensSlider->MaxValue = InputSensitivitySettings::MaxGamepadSensitivity;
        gpSensSlider->Value = sens.GamepadLookSensitivity;
        gpSensSlider->Step = 0.05f;
        gpSensSlider->size = vec2(360.f, 28.f);

        gpSensText = std::make_shared<UiTextBox>();
        gpSensText->NumericOnly = true;
        gpSensText->SetText(FormatFloat(sens.GamepadLookSensitivity));
        //gpSensText->size = vec2(90.f, 40.f);

        gpSensSlider->onValueChanged = [this](float v)
        {
            GameSettings::Instance().Input.Sensitivity.GamepadLookSensitivity = v;
            gpSensText->SetText(FormatFloat(v));
        };
        gpSensSlider->onDragEnd = [this](float) { GameSettings::Instance().SaveToFile(); };

        gpSensText->onSubmit = [this](const std::string& text)
        {
            float v = ParseFloatClamped(text, InputSensitivitySettings::MinGamepadSensitivity, InputSensitivitySettings::MaxGamepadSensitivity);
            GameSettings::Instance().Input.Sensitivity.GamepadLookSensitivity = v;
            gpSensSlider->Value = v;
            gpSensText->SetText(FormatFloat(v));
            GameSettings::Instance().SaveToFile();
        };

        sensitivityBox->AddChild(GetSliderRow("Gamepad Sensitivity", gpSensSlider, gpSensText));

        // ── Invert Y ─────────────────────────────────────────────────────────
        invertYButton = std::make_shared<UiButton>();
        invertYButton->size = vec2(200.f, 44.f);

        invertYLabel = std::make_shared<UiText>();
        invertYLabel->pivot = vec2(0.5f);
        invertYLabel->origin = vec2(0.5f);
        invertYButton->AddChild(invertYLabel);
        RefreshInvertYLabel();

        invertYButton->onClick = [this]()
        {
            InputSensitivitySettings& s = GameSettings::Instance().Input.Sensitivity;
            s.InvertY = !s.InvertY;
            RefreshInvertYLabel();
            GameSettings::Instance().SaveToFile();
        };

        sensitivityBox->AddChild(GetSliderRow("Invert Look Y", invertYButton, nullptr));
    }

    void RefreshInvertYLabel()
    {
        invertYLabel->text = GameSettings::Instance().Input.Sensitivity.InvertY ? "On" : "Off";
    }

    void RefreshSensitivityWidgets()
    {
        InputSensitivitySettings& sens = GameSettings::Instance().Input.Sensitivity;
        mouseSensSlider->Value = sens.MouseSensitivity;
        mouseSensText->SetText(FormatFloat(sens.MouseSensitivity));
        gpSensSlider->Value = sens.GamepadLookSensitivity;
        gpSensText->SetText(FormatFloat(sens.GamepadLookSensitivity));
        RefreshInvertYLabel();
    }

    std::shared_ptr<UiHorizontalBox> GetSliderRow(const std::string& label,
                                                   std::shared_ptr<UiElement> control,
                                                   std::shared_ptr<UiElement> secondaryControl)
    {
        auto labelCell = std::make_shared<UiElement>();
        labelCell->size = vec2(320.f, 40.f);

        auto txt = std::make_shared<UiText>();
        txt->text = label;
        //txt->fontSize = 24.f;
        txt->pivot = vec2(0.f, 0.5f);
        txt->origin = vec2(0.f, 0.5f);
        labelCell->AddChild(txt);

        auto row = std::make_shared<UiHorizontalBox>();
        row->ContentDistance = 14.f;
        row->AddChild(labelCell);
        row->AddChild(control);
        if (secondaryControl)
            row->AddChild(secondaryControl);

        return row;
    }

    // ───────────────────────────────────────────────────────────────────────
    // Bindings section
    // ───────────────────────────────────────────────────────────────────────

    void BuildBindingsSection()
    {
        scrollRegion = std::make_shared<UiScrollRegion>();
        scrollRegion->size = vec2(900.f, 600.f);

        scrollRegion->ContentDistance = 10.f;
        scrollRegion->onNavCancel = [this]() { backButton->onClick(); };

        for (const std::string& category : InputActionRegistry::GetOrderedCategories())
        {
            auto header = std::make_shared<UiText>();
            header->text = category;
            //header->fontSize = 26.f;
            header->textColor = vec4(0.75f, 0.85f, 1.f, 1.f);
			header->DisableFocus = true;
            header->HitCheck = false;
            header->DisableFocus = true;
            scrollRegion->AddChild(header);

            for (const std::string& action : InputActionRegistry::GetActionsInCategory(category))
                scrollRegion->AddChild(BuildActionRow(action));
        }
    }

    std::shared_ptr<UiHorizontalBox> BuildActionRow(const std::string& action)
    {
        auto row = std::make_shared<UiHorizontalBox>();
        row->ContentDistance = 10.f;

        auto labelCell = std::make_shared<UiElement>();
        labelCell->size = vec2(340.f, 72.f);

        auto labelTxt = std::make_shared<UiText>();
        labelTxt->text = DisplayNameFor(action);
        labelTxt->fontSize = 30.f;
        labelTxt->pivot = vec2(0.f, 0.5f);
        labelTxt->origin = vec2(0.f, 0.5f);
        labelCell->AddChild(labelTxt);
        row->AddChild(labelCell);

        RowWidgets rw;

        for (int i = 0; i < 2; ++i)
        {
            auto slot = std::make_shared<UiBindSlotButton>();
            slot->onActivate = [this, action, i]() { BeginKeyboardCapture(action, i); };
            slot->onClear = [this, action, i]() { ClearKeyboardSlot(action, i); };
            row->AddChild(slot);
            rw.kb[i] = slot;
        }

        for (int i = 0; i < 2; ++i)
        {
            auto slot = std::make_shared<UiBindSlotButton>();
            slot->onActivate = [this, action, i]() { BeginGamepadCapture(action, i); };
            slot->onClear = [this, action, i]() { ClearGamepadSlot(action, i); };
            row->AddChild(slot);
            rw.gp[i] = slot;
        }

        auto resetRowButton = std::make_shared<UiButton>();
        resetRowButton->size = vec2(84.f, 40.f);

        auto resetLabel = std::make_shared<UiText>();
        resetLabel->text = "Reset";
        //resetLabel->fontSize = 16.f;
        resetLabel->pivot = vec2(0.5f);
        resetLabel->origin = vec2(0.5f);
        resetRowButton->AddChild(resetLabel);

        resetRowButton->onClick = [this, action]()
        {
            GameSettings::Instance().Input.ResetBindingToDefault(action);
            GameSettings::Instance().Input.ApplyToEngine();
            GameSettings::Instance().SaveToFile();
            RefreshRow(action);
        };
        row->AddChild(resetRowButton);

        rowWidgets[action] = rw;
        RefreshRow(action);

        return row;
    }

    std::string DisplayNameFor(const std::string& action)
    {
        InputActionInfo* info = InputActionRegistry::Get(action);
        return info ? info->displayName : action;
    }

    void RefreshRow(const std::string& action)
    {
        auto it = rowWidgets.find(action);
        if (it == rowWidgets.end()) return;

        InputBindingOverride eff = GameSettings::Instance().Input.GetEffectiveBinding(action);

        for (int i = 0; i < 2; ++i)
        {
            it->second.kb[i]->SetContent(
                InputIconLibrary::GetKeyboardIcon(eff.kb[i]),
                InputBindingSerialization::KBDisplayName(eff.kb[i]),
                eff.kb[i].IsBound(), false);

            it->second.gp[i]->SetContent(
                InputIconLibrary::GetGamepadIcon(eff.gp[i]),
                InputBindingSerialization::GPDisplayName(eff.gp[i]),
                eff.gp[i].IsBound(), false);
        }
    }

    void RefreshAllRows()
    {
        for (auto& pair : rowWidgets) RefreshRow(pair.first);
    }

    void MarkListening(const std::string& action, bool isKeyboard, int slotIndex, bool listening)
    {
        auto it = rowWidgets.find(action);
        if (it == rowWidgets.end()) return;

        InputBindingOverride eff = GameSettings::Instance().Input.GetEffectiveBinding(action);

        if (isKeyboard)
        {
            it->second.kb[slotIndex]->SetContent(
                InputIconLibrary::GetKeyboardIcon(eff.kb[slotIndex]),
                InputBindingSerialization::KBDisplayName(eff.kb[slotIndex]),
                eff.kb[slotIndex].IsBound(), listening);
        }
        else
        {
            it->second.gp[slotIndex]->SetContent(
                InputIconLibrary::GetGamepadIcon(eff.gp[slotIndex]),
                InputBindingSerialization::GPDisplayName(eff.gp[slotIndex]),
                eff.gp[slotIndex].IsBound(), listening);
        }
    }

    // ── Capture flow ─────────────────────────────────────────────────────────

    void BeginKeyboardCapture(const std::string& action, int slotIndex)
    {
        MarkListening(action, true, slotIndex, true);

        auto modal = std::make_shared<UiRebindCaptureModal>(
            RebindDevice::Keyboard, DisplayNameFor(action), slotIndex,
            [this, action, slotIndex](InputBindingSlotKB value)
            {
                MarkListening(action, true, slotIndex, false);
                TryCommitKeyboard(action, slotIndex, value);
            },
            nullptr,
            [this, action, slotIndex]() { MarkListening(action, true, slotIndex, false); });

        EngineMain::MainInstance->Viewport.AddChild(modal);
    }

    void BeginGamepadCapture(const std::string& action, int slotIndex)
    {
        MarkListening(action, false, slotIndex, true);

        auto modal = std::make_shared<UiRebindCaptureModal>(
            RebindDevice::Gamepad, DisplayNameFor(action), slotIndex,
            nullptr,
            [this, action, slotIndex](InputBindingSlotGP value)
            {
                MarkListening(action, false, slotIndex, false);
                TryCommitGamepad(action, slotIndex, value);
            },
            [this, action, slotIndex]() { MarkListening(action, false, slotIndex, false); });

        EngineMain::MainInstance->Viewport.AddChild(modal);
    }

    void TryCommitKeyboard(const std::string& action, int slotIndex, InputBindingSlotKB value)
    {
        std::string conflict = GameSettings::Instance().Input.SetKeyboardSlot(action, slotIndex, value, false);
        if (conflict.empty())
        {
            FinishCommit();
            return;
        }

        std::string message = "'" + InputBindingSerialization::KBDisplayName(value) +
            "' is already bound to " + DisplayNameFor(conflict) + ". Reassign it here?";

        auto dialog = std::make_shared<UiConfirmDialog>(message,
            [this, action, slotIndex, value]()
            {
                GameSettings::Instance().Input.SetKeyboardSlot(action, slotIndex, value, true);
                FinishCommit();
            },
            [this, action]() { RefreshRow(action); });

        EngineMain::MainInstance->Viewport.AddChild(dialog);
    }

    void TryCommitGamepad(const std::string& action, int slotIndex, InputBindingSlotGP value)
    {
        std::string conflict = GameSettings::Instance().Input.SetGamepadSlot(action, slotIndex, value, false);
        if (conflict.empty())
        {
            FinishCommit();
            return;
        }

        std::string message = "'" + InputBindingSerialization::GPDisplayName(value) +
            "' is already bound to " + DisplayNameFor(conflict) + ". Reassign it here?";

        auto dialog = std::make_shared<UiConfirmDialog>(message,
            [this, action, slotIndex, value]()
            {
                GameSettings::Instance().Input.SetGamepadSlot(action, slotIndex, value, true);
                FinishCommit();
            },
            [this, action]() { RefreshRow(action); });

        EngineMain::MainInstance->Viewport.AddChild(dialog);
    }

    void FinishCommit()
    {
        GameSettings::Instance().Input.ApplyToEngine();
        GameSettings::Instance().SaveToFile();
        RefreshAllRows(); // a reassignment can touch the conflicting action's row too
    }

    void ClearKeyboardSlot(const std::string& action, int slotIndex)
    {
        GameSettings::Instance().Input.ClearKeyboardSlot(action, slotIndex);
        GameSettings::Instance().Input.ApplyToEngine();
        GameSettings::Instance().SaveToFile();
        RefreshRow(action);
    }

    void ClearGamepadSlot(const std::string& action, int slotIndex)
    {
        GameSettings::Instance().Input.ClearGamepadSlot(action, slotIndex);
        GameSettings::Instance().Input.ApplyToEngine();
        GameSettings::Instance().SaveToFile();
        RefreshRow(action);
    }

    void ConfirmResetAll()
    {
        auto dialog = std::make_shared<UiConfirmDialog>(
            "Reset all input settings to defaults?",
            [this]()
            {
                GameSettings::Instance().Input.ResetToDefaults();
                GameSettings::Instance().Input.ApplyToEngine();
                GameSettings::Instance().SaveToFile();
                RefreshAllRows();
                RefreshSensitivityWidgets();
            },
            []() {});

        EngineMain::MainInstance->Viewport.AddChild(dialog);
    }

    // ── Misc helpers ─────────────────────────────────────────────────────────

    std::shared_ptr<UiButton> MakeButton(std::string text)
    {
        std::shared_ptr<UiButton> btn = std::make_shared<UiButton>();
        std::shared_ptr<UiText> txt = std::make_shared<UiText>();

        btn->size = vec2(400, 70);
        txt->text = text;
        txt->pivot = vec2(0.5f);
        txt->origin = vec2(0.5f);
        btn->AddChild(txt);

        return btn;
    }

    static std::string FormatFloat(float v)
    {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(2) << v;
        return ss.str();
    }

    static float ParseFloatClamped(const std::string& text, float minV, float maxV)
    {
        try { return std::clamp(std::stof(text), minV, maxV); }
        catch (...) { return minV; }
    }
};
