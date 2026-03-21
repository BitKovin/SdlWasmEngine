#pragma once

#include "UiElement.h"
#include "UiImage.hpp"
#include "UiNavigation.h"

// ---------------------------------------------------------------------------
// UiFocusPointer
//
// A purely visual indicator that tracks UiNavigation::Focused.
// Place it anywhere in the UI tree (typically as a direct child of the
// viewport so its position is in screen space).
//
// Each frame it reads the focused element's screen-space topLeft/bottomRight,
// then positions itself to the left of that element, vertically centred.
//
//   [pointer image]   [focused element                    ]
//         <-- PointerOffset -->
//
// Properties:
//   ImagePath     — texture for the pointer image
//   PointerSize   — size of the pointer image
//   PointerOffset — gap between the right edge of the pointer and the
//                   left edge of the focused element
//   Color         — tint applied to the image
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
        // Not interactable — purely visual.
        HitCheck = false;
        // Draw on top of everything else.
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

        visible = Input::LockCursor == false;

        if (visible == false)
            UiNavigation::ClearFocus();

        // Focused element's screen-space bounds were written by the tree's
        // own Update() which runs before this element (we're a sibling/child
        // of the viewport, updated after the main tree completes).
        const vec2  drawTopLeft = focused->position + focused->offset;
        const vec2  drawSize = focused->size;
        const float drawCentreY = drawTopLeft.y + drawSize.y * 0.5f;
        const float focusedRightX = drawTopLeft.x + drawSize.x;

        // Position: right of focused element, vertically centred.
        size = PointerSize;
        position = vec2(
            focusedRightX + PointerOffset,
            drawCentreY - PointerSize.y * 0.5f + 0
        );

        m_image->ImagePath = ImagePath;
        m_image->color = Color;
        m_image->size = PointerSize;
        m_image->position = vec2(0.f);

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