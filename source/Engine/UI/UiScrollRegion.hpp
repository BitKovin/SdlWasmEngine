#pragma once

#include "UiElement.h"
#include "UiButton.hpp"
#include "UiImage.hpp"
#include "UiVerticalBox.hpp"
#include "UiNavigation.h"
#include "../Input.h"

#include <algorithm>
#include <cfloat>

// ---------------------------------------------------------------------------
// UiScrollRegion
//
// KEYBOARD / GAMEPAD NAVIGATION
//   The region itself is NOT the keyboard target — its children are.
//   When the region receives focus (e.g. because nothing else is focused),
//   it immediately delegates to its first child via UiNavigation::SetFocus.
//
//   If placed inside a FocusTrap (e.g. UiDropdown's panel), child items
//   with HitCheck=true are collected by UiNavigation and navigated
//   spatially — no internal bookkeeping needed here.
//
//   Each frame Update() checks which child is currently focused and
//   auto-scrolls to keep it visible.
//
//   onNavCancel — assign to handle ui_cancel at the trap level
//                 (UiDropdown wires this to close the panel).
// ---------------------------------------------------------------------------

class UiScrollRegion : public UiElement
{
public:
    // ── Appearance ────────────────────────────────────────────────────────────
    float ScrollBarWidth = 14.f;
    float MinThumbHeight = 30.f;
    float DragThreshold = 8.f;

    vec4 TrackColor = vec4(0.08f, 0.08f, 0.08f, 1.f);
    vec4 ThumbColor = vec4(0.40f, 0.40f, 0.40f, 1.f);
    vec4 ThumbActiveColor = vec4(0.65f, 0.65f, 0.65f, 1.f);

    std::string ScrollBarImage = "GameData/textures/generic/white.png";

    float ContentDistance = 0.f;

    // Called when ui_cancel fires while this region's FocusTrap is active.
    std::function<void()> onNavCancel = nullptr;

    // ── Construction ──────────────────────────────────────────────────────────
    UiScrollRegion()
    {
        // HitCheck=true so the region receives wheel/hover events when no
        // child intercepts the cursor (e.g. scrollbar area, empty space).
        HitCheck = true;

        m_content = std::make_shared<UiVerticalBox>();
        m_content->origin = vec2(0.f);
        m_content->pivot = vec2(0.f);
        UiElement::AddChild(m_content);

        m_track = std::make_shared<UiImage>();
        m_track->origin = vec2(0.f);
        m_track->pivot = vec2(0.f);
        UiElement::AddChild(m_track);

        m_thumb = std::make_shared<UiButton>();
        m_thumb->HitCheck = true;
        m_thumb->origin = vec2(0.f);
        m_thumb->pivot = vec2(0.f);
        m_thumb->DisableFocus = true;
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
    }

    // ── Accessors ─────────────────────────────────────────────────────────────
    float GetScrollOffset() const { return m_scrollOffset; }
    void  SetScrollOffset(float v) { m_scrollOffset = v; }
    float GetContentHeight() const { return m_contentHeight; }

    std::shared_ptr<UiVerticalBox> m_content;

    // ── Nav callbacks ─────────────────────────────────────────────────────────

    // When this region itself gains focus, immediately delegate to first child.
    void OnFocused() override
    {
        if (!m_content->children.empty())
            UiNavigation::SetFocus(m_content->children.front().get());
    }

    // Cancel is handled at the trap level by UiNavigation, which calls
    // OnNavCancel on the trap element. Wire onNavCancel from outside.
    void OnNavCancel() override
    {
        if (onNavCancel) onNavCancel();
    }

    // ── Update ────────────────────────────────────────────────────────────────
    void Update() override
    {
        const float viewH = size.y;
        const float contentW = size.x - ScrollBarWidth;
        const float contentH = m_contentHeight;
        const float maxScroll = std::max(0.f, contentH - viewH);
        const bool  needsBar = contentH > viewH + 0.5f;

        const float thumbH = needsBar
            ? std::max(MinThumbHeight, (viewH / contentH) * viewH)
            : viewH;
        const float trackRange = viewH - thumbH;

        // ── 1. Mouse wheel ─────────────────────────────────────────────────────
        if (TouchEvents.size() > 0)
            m_scrollOffset -= Input::MouseScrollDelta * 20.f;

        // ── 2. Thumb drag ──────────────────────────────────────────────────────
        if (needsBar) UpdateThumbDrag(maxScroll, trackRange);
        else          m_thumbDrag = {};

        // ── 3. Content drag (region-level, for when items don't steal touch) ───
        UpdateContentGesture(contentW);

        // ── 4. Auto-scroll to the currently focused child ──────────────────────
        ScrollToFocusedItem();

        // ── Clamp ──────────────────────────────────────────────────────────────
        m_scrollOffset = std::clamp(m_scrollOffset, 0.f, maxScroll);

        // ── Layout ─────────────────────────────────────────────────────────────
        m_content->ContentDistance = ContentDistance;
        m_content->size = vec2(contentW, std::max(viewH, contentH));
        m_content->position = vec2(0.f, -m_scrollOffset);

        m_track->visible = needsBar;
        m_thumb->visible = needsBar;

        m_track->ImagePath = ScrollBarImage;
        m_track->color = TrackColor;
        m_track->size = vec2(ScrollBarWidth, viewH);
        m_track->position = vec2(contentW, 0.f);

        const float thumbY = (maxScroll > 0.f) ? (m_scrollOffset / maxScroll) * trackRange : 0.f;
        m_thumb->ImagePath = ScrollBarImage;
        m_thumb->Color = m_thumbDrag.active ? ThumbActiveColor : ThumbColor;
        m_thumb->HoverColor = ThumbActiveColor;
        m_thumb->size = vec2(ScrollBarWidth, thumbH);
        m_thumb->position = vec2(contentW, thumbY);

        UiElement::Update();

        MeasureContentHeight();
    }

    // ── Draw ──────────────────────────────────────────────────────────────────
    void Draw() override
    {
        const vec2  pos = finalizedPosition + finalizedOffset;
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

    float m_scrollOffset = 0.f;
    float m_contentHeight = 0.f;

    struct ThumbDrag
    {
        bool  active = false;
        float startY = 0.f;
        float scrollAtStart = 0.f;
    } m_thumbDrag;

    struct ContentGesture
    {
        bool  active = false;
        bool  dragging = false;
        float maxMovement = 0.f;
        vec2  startPos = {};
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

    // ── Content gesture ───────────────────────────────────────────────────────
    // Only fires if the scroll region itself owns the touch (no child stole it).
    void UpdateContentGesture(float contentW)
    {
        const TouchEvent* touch = nullptr;
        for (const auto& t : TouchEvents)
        {
            if (t.position.x > topLeft.x + contentW) continue;
            touch = &t;
            break;
        }

        if (!touch) { m_gesture = {}; return; }

        if (touch->pressed)
        {
            m_gesture = { true, false, 0.f, touch->position, m_scrollOffset };
        }
        else if (touch->released)
        {
            m_gesture = {};
        }
        else if (m_gesture.active)
        {
            const float moved = glm::distance(touch->position, m_gesture.startPos);
            m_gesture.maxMovement = std::max(m_gesture.maxMovement, moved);
            if (m_gesture.maxMovement > DragThreshold) m_gesture.dragging = true;
            if (m_gesture.dragging)
                m_scrollOffset = m_gesture.scrollAtStart + (m_gesture.startPos.y - touch->position.y);
        }
    }

    // ── Auto-scroll to focused item ───────────────────────────────────────────
    void ScrollToFocusedItem()
    {
        UiElement* focused = UiNavigation::Focused;
        if (!focused) return;

        // Find which of our content children is focused (or contains focus).
        float itemTop = 0.f;
        for (const auto& child : m_content->children)
        {
            const float itemH = child->GetSize().y;
            const float itemBot = itemTop + itemH;

            if (child.get() == focused)
            {
                if (itemTop < m_scrollOffset)
                    m_scrollOffset = itemTop;
                else if (itemBot > m_scrollOffset + size.y)
                    m_scrollOffset = itemBot - size.y;
                return;
            }

            itemTop = itemBot + ContentDistance;
        }
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