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
// A clipping + scrolling container. Children may overflow vertically and
// are scrolled via:
//
//   1. Mouse wheel   — ui_scroll_up / ui_scroll_down, only when
//                      TouchEvents is non-empty (cursor is over this element)
//   2. Scrollbar thumb drag
//   3. Content-area touch/mouse with tap-vs-drag disambiguation
//
// TOUCH EVENT STATES
//   touch.pressed  == true                    BEGIN  (first frame of contact)
//   touch.pressed  == false, .released==false HELD   (subsequent frames)
//   touch.released == true                    END    (finger/button lifted)
//
// TAP DISPATCH
//   Fired on END, only if movement never exceeded DragThreshold.
//   Item under the END position is looked up by bounding box and its
//   onClick is called directly.
//
// HOVER
//   Any non-drag touch event (including pure mouse-over if the engine
//   delivers position-only events) is used to highlight the item under
//   the cursor. IsHovered is cleared on every frame before being set,
//   so stale highlights never persist.
// ---------------------------------------------------------------------------

class UiScrollRegion : public UiElement
{
public:
    // ── Appearance ────────────────────────────────────────────────────────────
    float ScrollBarWidth  = 14.f;
    float MinThumbHeight  = 30.f;
    float ScrollSpeed     = 60.f;
    float DragThreshold   = 8.f;

    vec4 TrackColor       = vec4(0.08f, 0.08f, 0.08f, 1.f);
    vec4 ThumbColor       = vec4(0.40f, 0.40f, 0.40f, 1.f);
    vec4 ThumbActiveColor = vec4(0.65f, 0.65f, 0.65f, 1.f);

    std::string ScrollBarImage = "GameData/textures/generic/white.png";

    float ContentDistance = 0.f;

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
    }

    // ── Accessors ─────────────────────────────────────────────────────────────
    float GetScrollOffset() const  { return m_scrollOffset; }
    void  SetScrollOffset(float v) { m_scrollOffset = v; }
    float GetContentHeight() const { return m_contentHeight; }

    // Exposed so UiDropdown can iterate items for width sync.
    std::shared_ptr<UiVerticalBox> m_content;

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

        // ── 1. Mouse wheel ────────────────────────────────────────────────────
        // TouchEvents is non-empty exactly when the cursor is over this element
        // (HitCheck = true). Using this instead of a manual bounds check.
        const bool mouseInBounds = TouchEvents.size() > 0;
        if (mouseInBounds)
        {

            m_scrollOffset -= Input::MouseScrollDelta * 10;

        }

        // ── 2. Scrollbar thumb drag ───────────────────────────────────────────
        if (needsBar)
            UpdateThumbDrag(maxScroll, trackRange);
        else
            m_thumbDrag = {};

        // ── 3. Content gesture + hover ────────────────────────────────────────
        UpdateContentGesture(contentW);

        // ── Clamp ─────────────────────────────────────────────────────────────
        m_scrollOffset = std::clamp(m_scrollOffset, 0.f, maxScroll);

        // ── Layout ────────────────────────────────────────────────────────────
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
            // HELD
            float delta = ((touch.position.y - m_thumbDrag.startY) / trackRange) * maxScroll;
            m_scrollOffset = m_thumbDrag.scrollAtStart + delta;
        }
    }

    // ── Content gesture + hover ───────────────────────────────────────────────
    void UpdateContentGesture(float contentW)
    {
        // Clear all item hover states every frame before potentially re-setting.
        SetAllItemsHover(false);

        // Find first content-area touch (exclude scrollbar strip).
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
            // BEGIN
            m_gesture.active        = true;
            m_gesture.dragging      = false;
            m_gesture.maxMovement   = 0.f;
            m_gesture.startPos      = touch->position;
            m_gesture.scrollAtStart = m_scrollOffset;
        }
        else if (touch->released)
        {
            // END — fire tap if the gesture never became a drag
            if (m_gesture.active && !m_gesture.dragging)
            {
                if (auto item = FindItemAt(touch->position))
                    if (auto btn = std::dynamic_pointer_cast<UiButton>(item))
                        if (btn->onClick) btn->onClick();
            }
            m_gesture = {};
            return; // no hover on release frame
        }
        else if (m_gesture.active)
        {
            // HELD
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

        // Hover: cursor is over the content area and we are not dragging.
        // Highlight whichever item is under the current touch position.
        if (auto item = FindItemAt(touch->position))
            if (auto btn = std::dynamic_pointer_cast<UiButton>(item))
                btn->IsHovered = true;
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

    void SetAllItemsHover(bool state)
    {
        for (const auto& child : m_content->children)
            if (auto btn = std::dynamic_pointer_cast<UiButton>(child))
                btn->IsHovered = state;
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
