#pragma once

#include <UI/UiElement.h>
#include <UI/UiImage.hpp>

#include <memory>

// ---------------------------------------------------------------------------
// UiSettingsStyle
//
// Shared visual language for the settings screens (UiInputSettings today --
// worth reusing from UiVideoSettings / UiSettingsMenu too) and the popups
// that belong to the same flow (UiConfirmDialog, UiRebindCaptureModal), so
// the whole thing reads as one consistent surface instead of a styled
// screen with plain black popups glued on top.
//
// Pulled into its own header (rather than living inside UiInputSettings.hpp)
// because UiConfirmDialog.hpp and UiRebindCaptureModal.hpp both want these
// constants too, and both get included by UiInputSettings.hpp -- defining
// the same namespace contents in more than one header that lands in the
// same translation unit would double-define everything in it.
// ---------------------------------------------------------------------------
namespace SettingsStyle
{
    constexpr float ContentWidth = 940.f;  // shared width for every panel so edges line up
    constexpr float PanelPadding = 24.f;   // inset between a panel's edge and its content
    constexpr float DividerThickness = 2.f;

    const vec4 Scrim = vec4(0.10f, 0.02f, 0.02f, 0.82f); // full-screen dim behind the menu
    const vec4 PanelFill = vec4(0.07f, 0.035f, 0.035f, 0.95f);
    const vec4 PanelBorder = vec4(0.55f, 0.16f, 0.16f, 0.55f);
    const vec4 Divider = vec4(0.85f, 0.35f, 0.35f, 0.30f);
    const vec4 RowAlt = vec4(1.f, 1.f, 1.f, 0.035f);
    const vec4 RowBase = vec4(0.f, 0.f, 0.f, 0.f);
    const vec4 CategoryAccent = vec4(0.60f, 0.78f, 1.f, 1.f);
    const vec4 CaptionColor = vec4(0.78f, 0.80f, 0.86f, 0.85f);
    const vec4 ToggleOn = vec4(0.24f, 0.52f, 0.30f, 0.92f);
    const vec4 ToggleOff = vec4(0.24f, 0.24f, 0.27f, 0.92f);
    const vec4 DangerFill = vec4(0.45f, 0.12f, 0.12f, 1.f);  // same red as the old per-slot clear button
    const vec4 DangerHover = vec4(0.62f, 0.16f, 0.16f, 1.f);

    // Each modal gets its own fill/border/accent so the two popup types are
    // visually distinct at a glance, not just two copies of the same box.
    //   Confirm dialog (Yes/No decisions)      -> wide/short, warm amber
    //   Rebind capture modal (listening state) -> narrow/tall, cool blue
    const vec4 ConfirmFill = vec4(0.07f, 0.055f, 0.03f, 0.97f);
    const vec4 ConfirmBorder = vec4(0.80f, 0.60f, 0.24f, 0.55f);
    const vec4 ConfirmAccent = vec4(0.85f, 0.66f, 0.30f, 0.9f);

    const vec4 CaptureFill = vec4(0.035f, 0.045f, 0.065f, 0.97f);
    const vec4 CaptureBorder = vec4(0.35f, 0.55f, 0.80f, 0.55f);
    const vec4 CaptureAccent = vec4(0.40f, 0.60f, 0.85f, 0.9f);

    constexpr float TitleSize = 47.f;
    constexpr float PanelHeaderSize = 40.f;
    constexpr float RowLabelSize = 27.f;
    constexpr float CategorySize = 40.f;
    constexpr float ActionLabelSize = 33.f;
    constexpr float CaptionSize = 25.f;
    constexpr float ButtonLabelSize = 45.f;
    constexpr float RowHeight = 88.f; // tall enough for a 72px UiBindSlotButton plus breathing room

    // Must match UiBindSlotButton::SlotSize -- keep in sync if that changes.
    // Only used to size the (invisible) placeholders in the binding list's
    // column-caption row so the captions land over the right slot group.
    const vec2 SlotButtonSize = vec2(72.f, 72.f);
    constexpr float SlotGroupGap = 6.f;   // gap *within* a kb/gp pair
    constexpr float SlotGroupSep = 35.f;  // gap *between* the kb group and the gp group
}

// A fixed-size card: a thin border rect, an inset fill rect, and a single
// content element centered on top.
//
// Built the same way UiBindSlotButton is -- UiElement base, children added
// via UiElement::AddChild, and a fixed size reported through an overridden
// GetSize() -- rather than as a UiCanvas. In this codebase UiCanvas is only
// ever used as a *root* element handed straight to a Viewport (see
// UiInputSettings / UiConfirmDialog / UiRebindCaptureModal); nesting one
// inside a UiVerticalBox/UiHorizontalBox/UiScrollRegion flow, which every
// panel and row here needs to do, produced invalid (NaN) layout offsets.
// UiElement with a fixed GetSize() is the pattern already proven to work
// nested inside exactly that kind of flow (every UiBindSlotButton in the
// binding list already does this).
class UiCardPanel : public UiElement
{
public:
    UiCardPanel(vec2 panelSize, std::shared_ptr<UiElement> content,
        vec4 fill = SettingsStyle::PanelFill, vec4 border = SettingsStyle::PanelBorder)
        : m_size(panelSize)
    {
        HitCheck = false; // purely a decorative wrapper -- its children are the real targets

        auto borderImage = std::make_shared<UiImage>();
        borderImage->color = border;
        borderImage->size = panelSize;
        UiElement::AddChild(borderImage);

        auto fillImage = std::make_shared<UiImage>();
        fillImage->color = fill;
        fillImage->size = vec2(panelSize.x - 3.f, panelSize.y - 3.f);
        fillImage->origin = vec2(0.5f);
        fillImage->pivot = vec2(0.5f);
        UiElement::AddChild(fillImage);

        content->origin = vec2(0.5f);
        content->pivot = vec2(0.5f);
        UiElement::AddChild(content);
    }

    glm::vec2 GetSize() override { return m_size; }

private:
    vec2 m_size;
};

// A fixed-size, left-aligned row with a flat background tint -- gives each
// binding row a visible strip instead of floating in open space, and lets
// alternating rows (zebra striping) read clearly as separate entries.
// Same UiElement + fixed-GetSize() reasoning as UiCardPanel above.
class UiRowCard : public UiElement
{
public:
    UiRowCard(vec2 rowSize, vec4 fill, std::shared_ptr<UiElement> content)
        : m_size(rowSize)
    {
        HitCheck = false;

        auto rowBackground = std::make_shared<UiImage>();
        rowBackground->color = fill;
        rowBackground->size = rowSize;
        UiElement::AddChild(rowBackground);

        content->origin = vec2(0.f, 0.5f);
        content->pivot = vec2(0.f, 0.5f);
        UiElement::AddChild(content);
    }

    glm::vec2 GetSize() override { return m_size; }

private:
    vec2 m_size;
};

inline std::shared_ptr<UiImage> MakeDivider(float width,
    vec4 color = SettingsStyle::Divider,
    float thickness = SettingsStyle::DividerThickness)
{
    auto divider = std::make_shared<UiImage>();
    divider->color = color;
    divider->size = vec2(width, thickness);
	divider->origin = vec2(0.5f, 0.0f);
	divider->pivot = vec2(0.5f, 0.0f);
    return divider;
}

inline std::shared_ptr<UiElement> MakeSpacer(vec2 size)
{
    auto spacer = std::make_shared<UiElement>();
    spacer->size = size;
    return spacer;
}