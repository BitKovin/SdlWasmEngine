#pragma once

#include "UiButton.hpp"
#include "UiVerticalBox.hpp"
#include "UiHorizontalBox.hpp"
#include "UiText.hpp"
#include "UiImage.hpp"
#include "UiCanvas.hpp"
#include "UiNavigation.h"

#include <functional>
#include <string>

// ---------------------------------------------------------------------------
// UiConfirmDialog
//
// Small Yes/No modal used for rebind-conflict confirmation ("X is already
// bound to <action> — reassign it here?"), but generic enough to reuse
// anywhere a confirm/cancel prompt is needed.
//
// FocusTrap=true + OnNavCancel routes gamepad/keyboard "cancel" (ui_cancel)
// straight to the No button, same idiom as every other modal in this UI
// (see UiSettingsMenu::OnNavCancel / UiVideoSettings::OnNavCancel).
// ---------------------------------------------------------------------------

class UiConfirmDialog : public UiCanvas
{
public:
    UiConfirmDialog(const std::string& message,
                     std::function<void()> onYes,
                     std::function<void()> onNo,
                     const std::string& yesLabel = "Yes",
                     const std::string& noLabel = "No")
        : m_onYes(std::move(onYes)), m_onNo(std::move(onNo))
    {
        FocusTrap = true;

        background = std::make_shared<UiImage>();
        background->color = vec4(0.f, 0.f, 0.f, 0.6f);
        AddChild(background);

        panel = std::make_shared<UiVerticalBox>();
        panel->origin = vec2(0.5f);
        panel->pivot = vec2(0.5f);
        panel->ContentDistance = 16.f;

        messageText = std::make_shared<UiText>();
        messageText->text = message;
        messageText->fontSize = 30.f;
        messageText->pivot = vec2(0.5f, 0.f);
        messageText->origin = vec2(0.5f, 0.f);

        buttonsRow = std::make_shared<UiHorizontalBox>();
        buttonsRow->origin = vec2(0.5f);
        buttonsRow->pivot = vec2(0.5f);
        buttonsRow->ContentDistance = 12.f;

        yesButton = MakeButton(yesLabel);
        noButton = MakeButton(noLabel);

        buttonsRow->AddChild(yesButton);
        buttonsRow->AddChild(noButton);

        panel->AddChild(messageText);
        panel->AddChild(buttonsRow);

        AddChild(panel);

        yesButton->onClick = [this]() { if (m_onYes) m_onYes(); RemoveFromParent(); };
        noButton->onClick  = [this]() { if (m_onNo) m_onNo(); RemoveFromParent(); };

        // Default focus lands on the safe/cancel option — matches the
        // "require an explicit extra step to do the destructive thing"
        // convention the rebind-conflict flow relies on.
        UiNavigation::SetFocus(noButton.get());
    }

    void OnNavCancel() override { noButton->onClick(); }

    void FinalizeChildren() override
    {
        background->size = GetSize();
        UiCanvas::FinalizeChildren();
    }

private:
    std::function<void()> m_onYes;
    std::function<void()> m_onNo;

    std::shared_ptr<UiImage> background;
    std::shared_ptr<UiVerticalBox> panel;
    std::shared_ptr<UiText> messageText;
    std::shared_ptr<UiHorizontalBox> buttonsRow;
    std::shared_ptr<UiButton> yesButton;
    std::shared_ptr<UiButton> noButton;

    std::shared_ptr<UiButton> MakeButton(const std::string& text)
    {
        auto btn = std::make_shared<UiButton>();
        auto txt = std::make_shared<UiText>();

        btn->size = vec2(160, 56);
        txt->text = text;
        txt->pivot = vec2(0.5f);
        txt->origin = vec2(0.5f);
        btn->AddChild(txt);

        return btn;
    }
};
