#pragma once

#include <UI/UiHorizontalBox.hpp>
#include "HudStatElement.hpp"

// ---------------------------------------------------------------------------
// HudStatusBar
//
// Top-center strip of the retro HUD. Holds a row of fixed-size
// HudStatElement columns (STAMINA / HEALTH / AMMO, and anything added
// later). Because each column has a fixed width, the row's total width is
// deterministic frame to frame; pivoting/centering the row itself on the
// canvas keeps it dead-center regardless of the screen's aspect ratio
// (canvas is 1080 tall, variable width).
// ---------------------------------------------------------------------------
class HudStatusBar : public UiHorizontalBox
{
public:
    HudStatusBar()
    {
        ContentDistance = 250.f;

        // Center this whole row horizontally on its parent (the HUD canvas),
        // anchored near the top of the screen. 
        origin = vec2(0.5f, 1.f);
        pivot = vec2(0.5f, 1.f);
        position = vec2(0.f, -20.f);
    }
};
