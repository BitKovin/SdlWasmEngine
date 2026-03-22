#pragma once

#include "UiElement.h"
#include "UiImage.hpp"
#include "UiNavigation.h"

// ---------------------------------------------------------------------------
// UiFocusPointer — purely visual indicator that tracks UiNavigation::Focused.
//
// Positioning is fully transform-aware:
//
//   1. The attachment point is the mid-point of the focused element's right
//      edge in local space — (sz.x, sz.y * 0.5) — transformed through
//      worldMatrix into screen space.
//
//   2. The offset direction is the element's own world-space X axis
//      (worldMatrix column 0, normalized), so the pointer always moves
//      away from the right edge regardless of rotation.
//
//   3. The pointer's own rotation is set to match the focused element so
//      it visually aligns with the edge rather than staying upright.
//
//   This means the pointer behaves correctly for any combination of
//   translation, rotation, and parent rotations.
// ---------------------------------------------------------------------------

class UiFocusPointer : public UiElement
{
public:
    std::string ImagePath = "GameData/textures/ui/white.png";
    vec2        PointerSize = vec2(20.f, 20.f);
    float       PointerOffset = 8.f;
    vec4        Color = vec4(1.f);

    UiFocusPointer()
    {
        HitCheck = false;
        useLateDraw = true;

        m_image = std::make_shared<UiImage>();
        m_image->origin = vec2(0.f);
        m_image->pivot = vec2(0.f);
        UiElement::AddChild(m_image);
    }

    void Update() override
    {
        UiElement* focused = UiNavigation::Focused;

        if (!focused || !focused->visible)
        {
            visible = false;
            UiElement::Update();
            return;
        }

        visible = !Input::LockCursor;

        if (!visible)
        {
            UiNavigation::ClearFocus();
            UiElement::Update();
            return;
        }

        const glm::mat3& m = focused->worldMatrix;
        const vec2        sz = focused->GetSize();

        // ── 1. Attachment point ───────────────────────────────────────────────
        // Mid-point of the focused element's right edge in its local space,
        // transformed to screen space through the full world matrix.
        const vec2 attachPoint = UiElement::TransformPoint(m, vec2(sz.x, sz.y * 0.5f));

        // ── 2. World-space right direction ────────────────────────────────────
        // Column 0 of the mat3 is the world-space X axis of the element.
        // Normalize so any scale in the matrix doesn't affect the offset length.
        const vec2  xAxis = vec2(m[0][0], m[0][1]);
        const float axisLen = glm::length(xAxis);
        const vec2  rightDir = (axisLen > 1e-6f) ? xAxis / axisLen : vec2(1.f, 0.f);

        // The perpendicular (element's Y axis, pointing down in local space)
        // is used to shift the pointer's top-left so the image centres
        // vertically on the attachment point.
        const vec2 downDir = vec2(-rightDir.y, rightDir.x);

        // ── 3. Place and orient this element ──────────────────────────────────
        size = PointerSize;
        // Start at the attachment point, step right by the gap, then step
        // back half a pointer height along the perpendicular so the pointer
        // image is vertically centred on the edge mid-point.
        position = attachPoint
            + rightDir * PointerOffset
            - downDir * (PointerSize.y * 0.5f);

        // Rotate the pointer to match the focused element's world orientation.
        rotation = glm::degrees(std::atan2(rightDir.y, rightDir.x));

        m_image->ImagePath = ImagePath;
        m_image->color = Color;
        m_image->size = PointerSize;
        m_image->position = vec2(0.f);
        m_image->rotation = 0.f;

        UiElement::Update();
    }

    void FinalizeChildren() override
    {
        Update();
        UiElement::FinalizeChildren();
    }

private:
    std::shared_ptr<UiImage> m_image;
};