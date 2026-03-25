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
// UiScrollRegion — see original header comments for design notes.
// Draw() now uses finalizedMatrix + PushMask(mat3) so the clipping rectangle
// and all child quads are correct even when an ancestor is rotated.
// ---------------------------------------------------------------------------

class UiScrollRegion : public UiElement
{
public:
    float ScrollBarWidth = 14.f;
    float MinThumbHeight = 30.f;
    float DragThreshold = 8.f;

    vec4 TrackColor = vec4(0.08f, 0.08f, 0.08f, 1.f);
    vec4 ThumbColor = vec4(0.40f, 0.40f, 0.40f, 1.f);
    vec4 ThumbActiveColor = vec4(0.65f, 0.65f, 0.65f, 1.f);

    std::string ScrollBarImage = "GameData/textures/generic/white.png";

    float ContentDistance = 0.f;

    std::function<void()> onNavCancel = nullptr;

    // ── Construction ──────────────────────────────────────────────────────────
    UiScrollRegion()
    {
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
        m_thumb->DisableFocus = true;
        m_thumb->origin = vec2(0.f);
        m_thumb->pivot = vec2(0.f);
        UiElement::AddChild(m_thumb);

        FocusPointerSide = UiNavDir::Left;
    }

    // ── Child redirection ─────────────────────────────────────────────────────
    void AddChild(std::shared_ptr<UiElement> child) override { m_content->UiElement::AddChild(child); child->FocusPointerSide = FocusPointerSide; }
    void RemoveChild(std::shared_ptr<UiElement> child) override { m_content->UiElement::RemoveChild(child); }
    void ClearChildren() override { m_content->UiElement::ClearChildren(); }

    // ── Accessors ─────────────────────────────────────────────────────────────
    float GetScrollOffset()  const { return m_scrollOffset; }
    void  SetScrollOffset(float v) { m_scrollOffset = v; }
    float GetContentHeight() const { return m_contentHeight; }

    std::shared_ptr<UiVerticalBox> m_content;

    // ── Nav callbacks ─────────────────────────────────────────────────────────
    void OnFocused() override
    {
        if (!m_content->children.empty())
            UiNavigation::SetFocus(m_content->children.front().get());
    }

    void OnNavCancel() override
    {
        if (onNavCancel) onNavCancel();
    }

    // ── Update ────────────────────────────────────────────────────────────────
    void Update() override
    {
        MeasureContentHeight();

        const float viewH = size.y;
        const float contentW = size.x - ScrollBarWidth;
        const float contentH = m_contentHeight;
        const float maxScroll = std::max(0.f, contentH - viewH);
        const bool  needsBar = contentH > viewH + 0.5f;

        const float thumbH = needsBar
            ? std::max(MinThumbHeight, (viewH / contentH) * viewH)
            : viewH;
        const float trackRange = viewH - thumbH;

        // ── Mouse wheel ───────────────────────────────────────────────────────
        m_scrollOffset -= Input::MouseScrollDelta * 20.f;

        // ── Thumb drag ────────────────────────────────────────────────────────
        if (needsBar) UpdateThumbDrag(maxScroll, trackRange);
        else          m_thumbDrag = {};

        // ── Content gesture ───────────────────────────────────────────────────
        // Snapshot dragging state BEFORE updating the gesture so that on the
        // release frame — when UpdateContentGesture clears m_gesture — we can
        // still suppress child onClick events that fire on the same frame.
        const bool wasDragging = m_gesture.dragging;
        UpdateContentGesture(contentW);

        // Suppress child touch events whenever a drag was (or still is) active.
        // This prevents buttons from firing onClick on the release frame after
        // a scroll gesture, while leaving genuine taps unaffected.
        if (wasDragging || m_gesture.dragging)
            SuppressTouchEventsRecursive(m_content.get());

        // ── Auto-scroll to focused child ──────────────────────────────────────
        ScrollToFocusedItem();

        m_scrollOffset = std::clamp(m_scrollOffset, 0.f, maxScroll);

        // ── Layout ────────────────────────────────────────────────────────────
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
    }

    // ── Draw ──────────────────────────────────────────────────────────────────
    void Draw() override
    {
        const float contentW = finalizedSize.x - ScrollBarWidth;

        if (finalizedChildren.size() > 0 && finalizedChildren[0]->visible)
        {
            UiRenderer::PushMask(finalizedMatrix, vec2(contentW, finalizedSize.y));
            finalizedChildren[0]->Draw();
            UiRenderer::PopMask();
        }

        if (finalizedChildren.size() > 1 && finalizedChildren[1]->visible)
            finalizedChildren[1]->Draw();

        if (finalizedChildren.size() > 2 && finalizedChildren[2]->visible)
            finalizedChildren[2]->Draw();

        if (drawBorder || drawAllBorders)
            UiRenderer::DrawBorderRect(finalizedMatrix, finalizedSize, vec4(1.f, 0.f, 0.f, 0.3f));
    }

private:
    std::shared_ptr<UiImage>  m_track;
    std::shared_ptr<UiButton> m_thumb;

    float m_scrollOffset = 0.f;
    float m_contentHeight = 0.f;

    struct ThumbDrag {
        bool  active = false;
        int   touchId = -1;
        float startY = 0.f;
        float scrollAtStart = 0.f;
    } m_thumbDrag;

    struct ContentGesture {
        bool  active = false;
        bool  dragging = false;
        float maxMovement = 0.f;
        vec2  startPos = {};
        float scrollAtStart = 0.f;
        int   touchId = -1;
    } m_gesture;

    // ── Touch helpers ─────────────────────────────────────────────────────────

    // Recursively clears TouchEvents on all elements in the subtree.
    // Called when a scroll drag is active so that child buttons do not fire
    // onClick when the finger is released.
    static void SuppressTouchEventsRecursive(UiElement* el)
    {
        el->TouchEvents.clear();
        for (const auto& child : el->children)
            SuppressTouchEventsRecursive(child.get());
    }

    // Scan the subtree of `el` for a pressed TouchEvent in the content area
    // (x <= contentRight).  Used only for gesture *start* detection; ongoing
    // tracking goes through Input::GetTouchEventFromId.
    static TouchEvent FindPressInTree(UiElement* el, float contentRight)
    {
        for (const auto& e : el->TouchEvents)
            if (e.pressed && e.position.x <= contentRight)
                return e;

        for (const auto& child : el->children)
        {
            TouchEvent found = FindPressInTree(child.get(), contentRight);
            if (found.id != -1) return found;
        }

        TouchEvent none;
        none.id = -1;
        return none;
    }

    // ── Thumb drag ────────────────────────────────────────────────────────────
    // Press detected from m_thumb->TouchEvents (thumb has HitCheck=true).
    // All subsequent movement/release fetched from the Input system by ID so
    // the drag survives the finger sliding off the thumb rect.
    void UpdateThumbDrag(float maxScroll, float trackRange)
    {
        if (!m_thumbDrag.active)
        {
            for (const auto& e : m_thumb->TouchEvents)
            {
                if (e.pressed)
                {
                    m_thumbDrag = { true, e.id, e.position.y, m_scrollOffset };
                    break;
                }
            }
        }

        if (!m_thumbDrag.active) return;

        const TouchEvent touch = Input::GetTouchEventFromId(m_thumbDrag.touchId);

        if (touch.released || (!touch.pressed && !Input::IsTouchEventHolding(m_thumbDrag.touchId)))
        {
            m_thumbDrag = {};
            return;
        }

        if (trackRange > 0.f)
        {
            const float delta = ((touch.position.y - m_thumbDrag.startY) / trackRange) * maxScroll;
            m_scrollOffset = m_thumbDrag.scrollAtStart + delta;
        }
    }

    // ── Content gesture ───────────────────────────────────────────────────────
    // Press detection scans the subtree because child buttons capture the event.
    // Once a gesture ID is pinned all updates come from Input::GetTouchEventFromId
    // so scrolling is unaffected by which element the finger is currently over.
    void UpdateContentGesture(float contentW)
    {
        if (!m_gesture.active)
        {
            const float contentRight = topLeft.x + contentW;
            const TouchEvent press = FindPressInTree(this, contentRight);
            if (press.id != -1)
                m_gesture = { true, false, 0.f, press.position, m_scrollOffset, press.id };
            return;
        }

        const TouchEvent touch = Input::GetTouchEventFromId(m_gesture.touchId);

        if (touch.released || (!touch.pressed && !Input::IsTouchEventHolding(m_gesture.touchId)))
        {
            m_gesture = {};
            return;
        }

        const float moved = glm::distance(touch.position, m_gesture.startPos);
        m_gesture.maxMovement = std::max(m_gesture.maxMovement, moved);
        if (m_gesture.maxMovement > DragThreshold)
            m_gesture.dragging = true;

        if (m_gesture.dragging)
            m_scrollOffset = m_gesture.scrollAtStart + (m_gesture.startPos.y - touch.position.y);
    }

    // ── Auto-scroll to focused item ───────────────────────────────────────────
    void ScrollToFocusedItem()
    {
        UiElement* focused = UiNavigation::Focused;
        if (!focused) return;

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

        float h = 0.f;
        for (const auto& child : m_content->children)
            h += child->GetSize().y + ContentDistance;

        m_contentHeight = h - ContentDistance;
    }
};