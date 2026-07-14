#pragma once

#include "UiElement.h"
#include "UiImage.hpp"
#include "UiNavigation.h"

// ---------------------------------------------------------------------------
// UiFocusPointer
//
// Key design decisions:
//
//   FocusPointer element:  rotation = 0 always.  Setting rotation here
//     corrupts child offset calculations because localTL is computed in
//     the rotated parent's local space.
//
//   m_image pivot = (0.5, 0.5) always (center).  Using an off-center pivot
//     for rotation causes the image to orbit around that point when rotated,
//     which flips it to the wrong side of the attachment point.
//     Positioning is handled exclusively through m_image->position.
//
//   m_image->position = offsetDir * PointerOffset in FocusPointer local
//     space.  Because FocusPointer has rotation=0, its local space is
//     screen-aligned, so offsetDir (which is in screen space) can be used
//     directly.
//
//   imageAngle = atan2(...) + 180°.  The +180° is required because atan2
//     gives the angle of the axis vector, but the pointer image asset faces
//     in the opposite convention.
// ---------------------------------------------------------------------------

class UiFocusPointer : public UiElement
{
public:
    std::string ImagePath = "GameData/textures/ui/white.png";
    vec2        PointerSize = vec2(20.f, 20.f);
    float       PointerOffset = 8.f;
    vec4        Color = vec4(1.f, 1.f, 1.f, 1.f);

    UiFocusPointer()
    {
        HitCheck = false;
        useLateDraw = true;

        m_image = std::make_shared<UiImage>();
        m_image->origin = vec2(0.f, 0.f);
        m_image->pivot = vec2(0.5f, 0.5f);   // center pivot — never changes
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

        visible = !Input::LockCursor && UiNavigation::MouseNavigation == false;

        if (Input::LockCursor)
        {
            UiNavigation::ClearFocus();
            UiElement::Update();
            return;
        }

        const glm::mat3& m = focused->worldMatrix;
        const vec2        sz = focused->GetSize();

        // World-space axes of the focused element (normalized).
        const vec2  xAxis = vec2(m[0][0], m[0][1]);
        const float xLen = glm::length(xAxis);
        const vec2  rightDir = (xLen > 1e-6f) ? xAxis / xLen : vec2(1.f, 0.f);
        // 90° CW from rightDir = element's local down axis in screen space
        const vec2  downDir = vec2(-rightDir.y, rightDir.x);

        vec2  localAttach = vec2(0.f, 0.f);
        vec2  offsetDir = vec2(0.f, 0.f);
        float imageAngle = 0.f;

        switch (focused->FocusPointerSide)
        {
        default:
        case UiNavDir::Right:
            localAttach = vec2(sz.x, sz.y * 0.5f);
            offsetDir = rightDir;
            imageAngle = glm::degrees(std::atan2(rightDir.y, rightDir.x)) + 180.f;
            break;

        case UiNavDir::Left:
            localAttach = vec2(0.f, sz.y * 0.5f);
            offsetDir = -rightDir;
            imageAngle = glm::degrees(std::atan2(-rightDir.y, -rightDir.x)) + 180.f;
            break;

        case UiNavDir::Up:
            localAttach = vec2(sz.x * 0.5f, 0.f);
            offsetDir = -downDir;
            imageAngle = glm::degrees(std::atan2(-downDir.y, -downDir.x)) + 180.f;
            break;

        case UiNavDir::Down:
            localAttach = vec2(sz.x * 0.5f, sz.y);
            offsetDir = downDir;
            imageAngle = glm::degrees(std::atan2(downDir.y, downDir.x)) + 180.f;
            break;
        }

        imageAngle = MathHelper::NormalizeAngle(imageAngle);

        if (imageAngle > 90)
        {
            imageAngle -= 180;
        }
        else if (imageAngle < -90)
        {
            imageAngle += 180;
        }

        // FocusPointer sits exactly at the attachment point with no rotation.
        // The image is pushed away via its own position in local space.
        // Because FocusPointer.rotation = 0, local space == screen-aligned
        // space, so offsetDir (screen-space) is valid here directly.
        const vec2 attachPoint = UiElement::TransformPoint(m, localAttach);

        size = PointerSize;
        position = attachPoint;
        rotation = 0.f;

        m_image->ImagePath = ImagePath;
        m_image->color = Color;
        m_image->size = PointerSize;
        m_image->position = offsetDir * PointerOffset + m_image->size*0.5f * offsetDir;  // gap from attachment
        m_image->rotation = imageAngle;

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