#pragma once

#include "UiElement.h"
#include "UiButton.hpp"
#include "UiImage.hpp"
#include "UiVerticalBox.hpp"
#include "../Input.h"

#include <algorithm>
#include <cfloat>

// ---------------------------------------------------------------------------
// UiScrollRegion
//
// KEYBOARD / GAMEPAD NAVIGATION
//   The region itself is a single nav node (HitCheck = true).
//   On receiving focus it internally manages item selection:
//     Up / Down   → move between items; consumed unless at the boundary,
//                   in which case global nav continues
//     Left/Right  → not consumed; global nav handles them
//     ui_confirm  → fires the focused item's onClick
//     ui_cancel   → calls onNavCancel if set
//
//   Auto-scroll keeps the keyboard-selected item visible.
//   onNavCancel — assign to handle cancel (e.g. UiDropdown closes itself).
// ---------------------------------------------------------------------------

class UiScrollRegion : public UiElement
{
public:
    // ── Appearance ────────────────────────────────────────────────────────────
    float ScrollBarWidth  = 14.f;
    float MinThumbHeight  = 30.f;
    float DragThreshold   = 8.f;

    vec4 TrackColor       = vec4(0.08f, 0.08f, 0.08f, 1.f);
    vec4 ThumbColor       = vec4(0.40f, 0.40f, 0.40f, 1.f);
    vec4 ThumbActiveColor = vec4(0.65f, 0.65f, 0.65f, 1.f);

    std::string ScrollBarImage = "GameData/textures/generic/white.png";

    float ContentDistance = 0.f;

    // Assign to handle ui_cancel while this region is focused.
    std::function<void()> onNavCancel = nullptr;

    // ── Construction ──────────────────────────────────────────────────────────
    UiScrollRegion()
    {
        HitCheck = true;

        m_content = std::make_shared<UiVerticalBox>();
        m_content->origin = vec2(0.f);
        m_content->pivot  = vec2(0.f);
        UiElement::AddChild(m_content);

        m_track = std::make_shared<UiImage>();
        m_track->origin = vec2(0.f);
        m_track->pivot  = vec2(0.f);
        UiElement::AddChild(m_track);

        m_thumb = std::make_shared<UiButton>();
        m_thumb->HitCheck = true;
        m_thumb->DisableFocus = true;
        m_thumb->origin   = vec2(0.f);
        m_thumb->pivot    = vec2(0.f);
        UiElement::AddChild(m_thumb);
    }

    // ── Child redirection ─────────────────────────────────────────────────────
    void AddChild(std::shared_ptr<UiElement> child) override
    {
        m_content->UiElement::AddChild(child);
    }
    void RemoveChild(std::shared_ptr<UiElement> child) override
    {
        m_content->UiElement::RemoveChild(child);
    }
    void ClearChildren() override
    {
        m_content->UiElement::ClearChildren();
        m_navIndex = -1;
    }

    // ── Accessors ─────────────────────────────────────────────────────────────
    float GetScrollOffset() const  { return m_scrollOffset; }
    void  SetScrollOffset(float v) { m_scrollOffset = v; }
    float GetContentHeight() const { return m_contentHeight; }

    // Exposed so UiDropdown can iterate items for width sync.
    std::shared_ptr<UiVerticalBox> m_content;

    // ── Keyboard nav callbacks ────────────────────────────────────────────────

    void OnFocused() override
    {
        if (m_navIndex < 0 && !m_content->children.empty())
            m_navIndex = 0;
        UpdateNavHighlight();
    }

    void OnUnfocused() override
    {
        ClearAllHighlights();
    }

    // Up/Down navigate items; Left/Right pass through to global nav.
    bool OnNav(UiNavDir dir) override
    {
        if (m_content->children.empty()) return false;

        const int count = static_cast<int>(m_content->children.size());

        if (dir == UiNavDir::Up)
        {
            if (m_navIndex > 0)
            {
                m_navIndex--;
                UpdateNavHighlight();
                ScrollToNavItem();
                return true; // consumed
            }
            return false; // at top — let global nav exit upward
        }

        if (dir == UiNavDir::Down)
        {
            if (m_navIndex < count - 1)
            {
                m_navIndex++;
                UpdateNavHighlight();
                ScrollToNavItem();
                return true; // consumed
            }
            return false; // at bottom — let global nav exit downward
        }

        return false; // Left/Right not consumed here
    }

    void OnNavConfirm() override
    {
        FireNavItem();
    }

    void OnNavCancel() override
    {
        if (onNavCancel) onNavCancel();
    }

    // ── Update ────────────────────────────────────────────────────────────────
    void Update() override
    {
        const float viewH     = size.y;
        const float contentW  = size.x - ScrollBarWidth;
        const float contentH  = m_contentHeight;
        const float maxScroll = std::max(0.f, contentH - viewH);
        const bool  needsBar  = contentH > viewH + 0.5f;

        const float thumbH     = needsBar
            ? std::max(MinThumbHeight, (viewH / contentH) * viewH)
            : viewH;
        const float trackRange = viewH - thumbH;

        // ── 1. Mouse wheel ─────────────────────────────────────────────────────
        // TouchEvents non-empty ↔ cursor is over this element (HitCheck=true).
        if (TouchEvents.size() > 0)
            m_scrollOffset -= Input::MouseScrollDelta * 20.f;

        // ── 2. Thumb drag ──────────────────────────────────────────────────────
        if (needsBar) UpdateThumbDrag(maxScroll, trackRange);
        else          m_thumbDrag = {};

        // ── 3. Content gesture + mouse hover ──────────────────────────────────
        UpdateContentGesture(contentW);

        // ── Clamp ──────────────────────────────────────────────────────────────
        m_scrollOffset = std::clamp(m_scrollOffset, 0.f, maxScroll);

        // ── Layout ─────────────────────────────────────────────────────────────
        m_content->ContentDistance = ContentDistance;
        m_content->size     = vec2(contentW, std::max(viewH, contentH));
        m_content->position = vec2(0.f, -m_scrollOffset);

        m_track->visible   = needsBar;
        m_thumb->visible   = needsBar;

        m_track->ImagePath = ScrollBarImage;
        m_track->color     = TrackColor;
        m_track->size      = vec2(ScrollBarWidth, viewH);
        m_track->position  = vec2(contentW, 0.f);

        const float thumbY = (maxScroll > 0.f) ? (m_scrollOffset / maxScroll) * trackRange : 0.f;
        m_thumb->ImagePath = ScrollBarImage;
        m_thumb->Color     = m_thumbDrag.active ? ThumbActiveColor : ThumbColor;
        m_thumb->HoverColor= ThumbActiveColor;
        m_thumb->size      = vec2(ScrollBarWidth, thumbH);
        m_thumb->position  = vec2(contentW, thumbY);

        UiElement::Update();

        MeasureContentHeight();
    }

    // ── Draw ──────────────────────────────────────────────────────────────────
    void Draw() override
    {
        const vec2  pos      = finalizedPosition + finalizedOffset;
        const float contentW = finalizedSize.x - ScrollBarWidth;

        if (finalizedChildren.size() > 0 && finalizedChildren[0]->visible)
        {
            UiRenderer::PushMask(pos, vec2(contentW, finalizedSize.y));
            finalizedChildren[0]->Draw();
            UiRenderer::PopMask();
        }
        if (finalizedChildren.size() > 1 && finalizedChildren[1]->visible)
            finalizedChildren[1]->Draw();
        if (finalizedChildren.size() > 2 && finalizedChildren[2]->visible)
            finalizedChildren[2]->Draw();

        if (drawBorder || drawAllBorders)
            UiRenderer::DrawBorderRect(pos, finalizedSize, vec4(1.f, 0.f, 0.f, 0.3f));
    }

private:
    std::shared_ptr<UiImage>  m_track;
    std::shared_ptr<UiButton> m_thumb;

    float m_scrollOffset  = 0.f;
    float m_contentHeight = 0.f;

    // Keyboard-selected item index (-1 = none).
    int m_navIndex = -1;

    struct ThumbDrag
    {
        bool  active        = false;
        float startY        = 0.f;
        float scrollAtStart = 0.f;
    } m_thumbDrag;

    struct ContentGesture
    {
        bool  active        = false;
        bool  dragging      = false;
        float maxMovement   = 0.f;
        vec2  startPos      = {};
        float scrollAtStart = 0.f;
    } m_gesture;

    // ── Thumb drag ────────────────────────────────────────────────────────────
    void UpdateThumbDrag(float maxScroll, float trackRange)
    {
        const auto& events = m_thumb->TouchEvents;
        if (events.empty()) { m_thumbDrag = {}; return; }

        const TouchEvent& touch = events.front();

        if (touch.pressed)
        {
            m_thumbDrag = { true, touch.position.y, m_scrollOffset };
        }
        else if (touch.released)
        {
            if (m_thumbDrag.active && trackRange > 0.f)
            {
                float delta = ((touch.position.y - m_thumbDrag.startY) / trackRange) * maxScroll;
                m_scrollOffset = m_thumbDrag.scrollAtStart + delta;
            }
            m_thumbDrag = {};
        }
        else if (m_thumbDrag.active && trackRange > 0.f)
        {
            float delta = ((touch.position.y - m_thumbDrag.startY) / trackRange) * maxScroll;
            m_scrollOffset = m_thumbDrag.scrollAtStart + delta;
        }
    }

    // ── Content gesture + mouse hover ─────────────────────────────────────────
    void UpdateContentGesture(float contentW)
    {
        // Clear mouse hover every frame; re-set below. Keyboard nav highlight
        // (m_navIndex) is managed separately and NOT cleared here.
        ClearMouseHover();

        const TouchEvent* touch = nullptr;
        for (const auto& t : TouchEvents)
        {
            if (t.position.x > topLeft.x + contentW) continue;
            touch = &t;
            break;
        }

        if (!touch)
        {
            m_gesture = {};
            return;
        }

        if (touch->pressed)
        {
            m_gesture.active        = true;
            m_gesture.dragging      = false;
            m_gesture.maxMovement   = 0.f;
            m_gesture.startPos      = touch->position;
            m_gesture.scrollAtStart = m_scrollOffset;
        }
        else if (touch->released)
        {
            if (m_gesture.active && !m_gesture.dragging)
            {
                // Tap: fire the item under the release position.
                if (auto item = FindItemAt(touch->position))
                    if (auto btn = std::dynamic_pointer_cast<UiButton>(item))
                        if (btn->onClick) btn->onClick();
            }
            m_gesture = {};
            return; // no hover on release frame
        }
        else if (m_gesture.active)
        {
            const float moved = glm::distance(touch->position, m_gesture.startPos);
            m_gesture.maxMovement = std::max(m_gesture.maxMovement, moved);

            if (m_gesture.maxMovement > DragThreshold)
                m_gesture.dragging = true;

            if (m_gesture.dragging)
            {
                m_scrollOffset = m_gesture.scrollAtStart
                               + (m_gesture.startPos.y - touch->position.y);
                return; // no hover while dragging
            }
        }

        // Hover: highlight item under cursor when not dragging.
        if (auto item = FindItemAt(touch->position))
            if (auto btn = std::dynamic_pointer_cast<UiButton>(item))
                btn->IsHovered = true;
    }

    // ── Keyboard nav helpers ──────────────────────────────────────────────────

    void UpdateNavHighlight()
    {
        for (int i = 0; i < static_cast<int>(m_content->children.size()); ++i)
            if (auto btn = std::dynamic_pointer_cast<UiButton>(m_content->children[i]))
                btn->IsHovered = (i == m_navIndex);
    }

    void ClearAllHighlights()
    {
        for (auto& child : m_content->children)
            if (auto btn = std::dynamic_pointer_cast<UiButton>(child))
                btn->IsHovered = false;
        m_navIndex = -1;
    }

    // Clear mouse hover without touching the keyboard-nav-selected item.
    void ClearMouseHover()
    {
        for (int i = 0; i < static_cast<int>(m_content->children.size()); ++i)
        {
            if (i == m_navIndex) continue;
            if (auto btn = std::dynamic_pointer_cast<UiButton>(m_content->children[i]))
                btn->IsHovered = false;
        }
    }

    void FireNavItem()
    {
        if (m_navIndex < 0 || m_navIndex >= static_cast<int>(m_content->children.size())) return;
        if (auto btn = std::dynamic_pointer_cast<UiButton>(m_content->children[m_navIndex]))
            if (btn->onClick) btn->onClick();
    }

    // Scroll so the keyboard-selected item is fully in view.
    void ScrollToNavItem()
    {
        if (m_navIndex < 0 || m_navIndex >= static_cast<int>(m_content->children.size())) return;

        // Build item position from sizes — doesn't rely on stale topLeft values.
        float itemTop = 0.f;
        for (int i = 0; i < m_navIndex; ++i)
            itemTop += m_content->children[i]->GetSize().y + ContentDistance;
        const float itemBot = itemTop + m_content->children[m_navIndex]->GetSize().y;

        if (itemTop < m_scrollOffset)
            m_scrollOffset = itemTop;
        else if (itemBot > m_scrollOffset + size.y)
            m_scrollOffset = itemBot - size.y;
    }

    // ── Helpers ───────────────────────────────────────────────────────────────

    std::shared_ptr<UiElement> FindItemAt(vec2 pos) const
    {
        for (const auto& child : m_content->children)
        {
            if (pos.x >= child->topLeft.x  && pos.x <= child->bottomRight.x &&
                pos.y >= child->topLeft.y  && pos.y <= child->bottomRight.y)
                return child;
        }
        return nullptr;
    }

    void MeasureContentHeight()
    {
        if (m_content->children.empty()) { m_contentHeight = 0.f; return; }

        float maxBottom = -FLT_MAX;
        for (const auto& child : m_content->children)
            maxBottom = std::max(maxBottom, child->bottomRight.y);

        m_contentHeight = maxBottom - m_content->topLeft.y;
    }
};
