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

// ---------------------------------------------------------------------------
// UiConfirmDialog
//
// Small Yes/No modal used for rebind-conflict confirmation ("X is already
// bound to <action> — reassign it here?") and the "reset everything"
// prompt, but generic enough to reuse anywhere a confirm/cancel prompt is
// needed.
//
// FocusTrap=true + OnNavCancel routes gamepad/keyboard "cancel" (ui_cancel)
// straight to the No button, same idiom as every other modal in this UI
// (see UiSettingsMenu::OnNavCancel / UiVideoSettings::OnNavCancel).
//
// Shape/color is deliberately different from UiRebindCaptureModal: wide and
// short, amber-bordered, so the two modal types read as distinct even
// before you read the text on them.
//
// The card is sized generously (see kCardWidth) for the longest realistic
// rebind-conflict sentence this UI actually produces. If action/binding
// display names can run unusually long, this is the one place that could
// still overflow — worth adding real text wrapping to UiText if that turns
// out to matter in practice.
// ---------------------------------------------------------------------------

class UiConfirmDialog : public UiCanvas
{
public:
    UiConfirmDialog(const std::string& message,
        std::function<void()> onYes,
        std::function<void()> onNo,
        const std::string& yesLabel = "${SETTINGS_CONFIRM_YES}",
        const std::string& noLabel = "${SETTINGS_CONFIRM_NO}")
        : m_onYes(std::move(onYes)), m_onNo(std::move(onNo))
    {
        FocusTrap = true;

        background = std::make_shared<UiImage>();
        background->color = vec4(0.f, 0.f, 0.f, 0.6f);
        background->HitCheck = true;
        background->DisableFocus = true;
        AddChild(background);

        panel = std::make_shared<UiVerticalBox>();
        panel->origin = vec2(0.5f);
        panel->pivot = vec2(0.5f);
        panel->ContentDistance = 16.f;

        messageText = std::make_shared<UiText>();
        messageText->text = message;
        messageText->fontSize = 40.f;
        messageText->pivot = vec2(0.5f, 0.f);
        messageText->origin = vec2(0.5f, 0.f);

        buttonsRow = std::make_shared<UiHorizontalBox>();
        buttonsRow->origin = vec2(0.5f, 0.f);
        buttonsRow->pivot = vec2(0.5f, 0.f);
        buttonsRow->ContentDistance = 12.f;

        yesButton = MakeButton(yesLabel);
        noButton = MakeButton(noLabel);

        buttonsRow->AddChild(yesButton);
        buttonsRow->AddChild(noButton);

        panel->AddChild(messageText);

        // FIX: Capture the divider and apply horizontal centering (0.5f)
        auto divider = MakeDivider(kCardWidth - SettingsStyle::PanelPadding * 2.f, SettingsStyle::ConfirmAccent, 2.f);
        divider->origin = vec2(0.5f, 0.f);
        divider->pivot = vec2(0.5f, 0.f);
        panel->AddChild(divider);

        panel->AddChild(buttonsRow);

		float cardWidth = kCardWidth;

		if (cardWidth < messageText->GetSize().x + SettingsStyle::PanelPadding * 2.f)
		{
			cardWidth = messageText->GetSize().x + SettingsStyle::PanelPadding * 2.f;
		}

        auto card = std::make_shared<UiCardPanel>(vec2(cardWidth, 200.f), panel,
            SettingsStyle::ConfirmFill, SettingsStyle::ConfirmBorder);
        card->origin = vec2(0.5f);
        card->pivot = vec2(0.5f);
        AddChild(card);

        yesButton->onClick = [this]() { if (m_onYes) m_onYes(); RemoveFromParent(); };
        noButton->onClick = [this]() { if (m_onNo) m_onNo(); RemoveFromParent(); };

        // Default focus lands on the safe/cancel option
        UiNavigation::SetFocus(noButton.get());
    }

    void OnNavCancel() override { noButton->onClick(); }

    void FinalizeChildren() override
    {
        background->size = GetSize();
        UiCanvas::FinalizeChildren();
    }

private:
    static constexpr float kCardWidth = 720.f;

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
        txt->fontSize = SettingsStyle::ButtonLabelSize;
        txt->pivot = vec2(0.5f);
        txt->origin = vec2(0.5f);
        btn->AddChild(txt);

        return btn;
    }
};
