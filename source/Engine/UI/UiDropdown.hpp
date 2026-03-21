#pragma once

#include "UiElement.h"
#include "UiButton.hpp"
#include "UiText.hpp"
#include "UiScrollRegion.hpp"

// ---------------------------------------------------------------------------
// UiDropdown
//
//  Header: UiButton
//            ├─ UiText  (m_headerLabel)  — current selection, left-aligned
//            └─ UiText  (m_headerArrow)  — "▼"/"▲", right-aligned, separate
//
//  Panel:  UiScrollRegion  (hidden until opened)
//            └─ [UiButton + UiText] × N  — one row per option
//
//  Padding
//  ───────
//  `Padding` insets all content from the element edges. Useful when the
//  background texture has a border that must not be overlapped by text.
//  It applies to:
//    - Header: label pushed right by Padding, arrow pulled left by Padding,
//              label/arrow Y centred within the padded height
//    - Items:  label pushed right by Padding, height padded top+bottom
//              (ItemHeight is the total row height including padding)
// ---------------------------------------------------------------------------

class UiDropdown : public UiElement
{
public:
    // ── Appearance ─────────────────────────────────────────────────────────────
    std::string BackgroundImage  = "GameData/textures/generic/white.png";

    vec4 HeaderColor      = vec4(0.20f, 0.20f, 0.20f, 1.f);
    vec4 HeaderHoverColor = vec4(0.30f, 0.30f, 0.30f, 1.f);
    vec4 ItemColor        = vec4(0.15f, 0.15f, 0.15f, 1.f);
    vec4 ItemHoverColor   = vec4(0.28f, 0.28f, 0.28f, 1.f);
    vec4 TextColor        = vec4(1.f);
    vec4 ArrowColor       = vec4(1.f);
    vec4 ScrollTrackColor = vec4(0.10f, 0.10f, 0.10f, 1.f);
    vec4 ScrollThumbColor = vec4(0.38f, 0.38f, 0.38f, 1.f);
    vec4 ScrollThumbHover = vec4(0.60f, 0.60f, 0.60f, 1.f);

    float FontSize       = 32.f;
    float ArrowFontSize  = 28.f;
    float ItemHeight     = 48.f;
    float ScrollBarWidth = 14.f;

    // Insets all text content from element edges. Respect textured borders here.
    float Padding = 8.f;

    // Additional spacing between label and the left/right edge, on top of Padding.
    float LabelSpacing = 4.f;
    float ArrowSpacing = 4.f;

    int MaxVisibleItems = 5;

    std::string Placeholder = "Select…";
    std::string ArrowDown   = "d";
    std::string ArrowUp     = "u";

    // ── State ──────────────────────────────────────────────────────────────────
    int SelectedIndex = -1;

    std::function<void(int index, const std::string& value)> onSelectionChanged = nullptr;

    // ── Construction ───────────────────────────────────────────────────────────
    UiDropdown()
    {
        HitCheck = false;

        m_header = std::make_shared<UiButton>();
        m_header->origin  = vec2(0.f);
        m_header->pivot   = vec2(0.f);
        m_header->onClick = [this]() { SetOpen(!m_isOpen); };

        // Label — left-aligned, pivot.y=0.5 centres vertically
        m_headerLabel = std::make_shared<UiText>();
        m_headerLabel->origin = vec2(0.f);
        m_headerLabel->pivot  = vec2(0.f, 0.5f);
        m_header->AddChild(m_headerLabel);

        // Arrow — separate element, right-aligned via pivot.x=1
        m_headerArrow = std::make_shared<UiText>();
        m_headerArrow->origin = vec2(0.f);
        m_headerArrow->pivot  = vec2(1.f, 0.5f);
        m_header->AddChild(m_headerArrow);

        UiElement::AddChild(m_header);

        m_panel = std::make_shared<UiScrollRegion>();
        m_panel->origin  = vec2(0.f);
        m_panel->pivot   = vec2(0.f);
        m_panel->visible = false;
        m_panel->useLateDraw = true;

        UiElement::AddChild(m_panel);
    }

    // ── Public API ─────────────────────────────────────────────────────────────
    void SetOptions(const std::vector<std::string>& options)
    {
        m_options = options;
        RebuildPanel();
    }

    const std::vector<std::string>& GetOptions() const { return m_options; }

    const std::string& GetSelectedValue() const
    {
        if (SelectedIndex >= 0 && SelectedIndex < static_cast<int>(m_options.size()))
            return m_options[SelectedIndex];
        static const std::string empty;
        return empty;
    }

    void SetSelectedIndex(int index) { SelectedIndex = index; }

    // Only header height contributes to layout; panel overlays content below.
    glm::vec2 GetSize() override { return size; }

    // ── Update ─────────────────────────────────────────────────────────────────
    void Update() override
    {
        const vec2  headerSize  = size;
        const int   numOptions  = static_cast<int>(m_options.size());
        const int   visibleRows = std::min(numOptions, MaxVisibleItems);

        // Padded vertical centre for header text
        const float textCentreY = headerSize.y * 0.5f;

        // ── Header ─────────────────────────────────────────────────────────────
        m_header->size       = headerSize;
        m_header->ImagePath  = BackgroundImage;
        m_header->Color      = m_isOpen ? HeaderHoverColor : HeaderColor;
        m_header->HoverColor = HeaderHoverColor;

        // Label: left edge inset by Padding + LabelSpacing
        m_headerLabel->text      = CurrentLabel();
        m_headerLabel->fontSize  = FontSize;
        m_headerLabel->textColor = TextColor;
        m_headerLabel->position  = vec2(Padding + LabelSpacing, textCentreY);

        // Arrow: right edge pulled in by Padding + ArrowSpacing; pivot.x=1 aligns it
        m_headerArrow->text      = m_isOpen ? ArrowUp : ArrowDown;
        m_headerArrow->fontSize  = ArrowFontSize;
        m_headerArrow->textColor = ArrowColor;
        m_headerArrow->position  = vec2(headerSize.x - Padding - ArrowSpacing, textCentreY);

        // ── Panel ─────────────────────────────────────────────────────────────
        m_panel->visible          = m_isOpen;
        m_panel->position         = vec2(0.f, headerSize.y);
        m_panel->size             = ComputePanelSize(visibleRows, headerSize.x);
        m_panel->ScrollBarWidth   = ScrollBarWidth;
        m_panel->TrackColor       = ScrollTrackColor;
        m_panel->ThumbColor       = ScrollThumbColor;
        m_panel->ThumbActiveColor = ScrollThumbHover;
        m_panel->ScrollBarImage   = BackgroundImage;

        // Keep item widths in sync if size.x changed after RebuildPanel().
        for (auto& child : m_panel->m_content->children)
            child->size.x = headerSize.x;

        UiElement::Update();
    }

    void FinalizeChildren() override
    {
        m_panel->visible = m_isOpen;
        UiElement::FinalizeChildren();
    }

private:
    bool m_isOpen = false;
    std::vector<std::string> m_options;

    std::shared_ptr<UiButton>       m_header;
    std::shared_ptr<UiText>         m_headerLabel;
    std::shared_ptr<UiText>         m_headerArrow;
    std::shared_ptr<UiScrollRegion> m_panel;

    std::string CurrentLabel() const
    {
        if (SelectedIndex >= 0 && SelectedIndex < static_cast<int>(m_options.size()))
            return m_options[SelectedIndex];
        return Placeholder;
    }

    // Panel height = N visible rows. ItemHeight is the full row height.
    vec2 ComputePanelSize(int rows, float width) const
    {
        return vec2(width, static_cast<float>(rows) * ItemHeight);
    }

    void SetOpen(bool open)
    {
        m_isOpen         = open;
        m_panel->visible = open;
        if (!open) m_panel->SetScrollOffset(0.f);
    }

    void RebuildPanel()
    {
        m_panel->ClearChildren();

        for (int i = 0; i < static_cast<int>(m_options.size()); ++i)
        {
            auto itemBtn = std::make_shared<UiButton>();
            itemBtn->HitCheck   = false; // UiScrollRegion owns all content touches
            itemBtn->size       = vec2(size.x, ItemHeight);
            itemBtn->origin     = vec2(0.f);
            itemBtn->pivot      = vec2(0.f);
            itemBtn->Color      = ItemColor;
            itemBtn->HoverColor = ItemHoverColor;
            itemBtn->ImagePath  = BackgroundImage;

            auto itemLabel = std::make_shared<UiText>();
            itemLabel->origin    = vec2(0.f);
            itemLabel->pivot     = vec2(0.f, 0.5f);
            // Label X respects Padding + LabelSpacing, same as header label.
            // Label Y centred within the full item height.
            itemLabel->position  = vec2(Padding + LabelSpacing, ItemHeight * 0.5f);
            itemLabel->text      = m_options[i];
            itemLabel->fontSize  = FontSize;
            itemLabel->textColor = TextColor;

            itemBtn->AddChild(itemLabel);

            const int capturedIndex = i;
            itemBtn->onClick = [this, capturedIndex]()
            {
                SelectedIndex = capturedIndex;
                SetOpen(false);
                if (onSelectionChanged)
                    onSelectionChanged(capturedIndex, m_options[capturedIndex]);
            };

            m_panel->AddChild(itemBtn);
        }
    }
};
