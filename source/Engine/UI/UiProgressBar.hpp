#pragma once

#include "UiElement.h"
#include "../Texture.hpp"
#include "../AssetRegistry.h"

#include <algorithm>

// ---------------------------------------------------------------------------
// UiProgressBar
//
// Two-pass, geometry-masked fill — no fragment-shader masking anymore (that
// was ui/fs_progressBar.sc; this class no longer references it, only
// AssetRegistry-loaded textures + UiElement::DrawSelfTextured). Each frame
// draws up to two sub-rects of itself, split at the Progress boundary along
// Direction:
//
//   Background pass — BackgroundImage tinted by BackgroundColor, covering
//                      the *unfilled* portion. Can carry shadow/outline/glow
//                      like any other UiElement (see UiElement.h) — it's the
//                      stable, non-moving part of the bar.
//   Fill pass        — ProgressImage tinted by `color` (the inherited field,
//                      repurposed here as the fill tint, same as before).
//                      Covers the *filled* portion. Always drawn with
//                      useEffects=false, even if shadow/outline/glow are
//                      enabled on this element — its edge is wherever
//                      Progress currently cuts it off, not a stable
//                      silhouette, so a shadow tracking that moving edge
//                      would read as a rendering glitch, not a design
//                      element.
//
// Both passes go through UiElement::DrawSelfTextured (setting RectPosition/
// RectSize before each call), which submits only the visible sub-rect's
// geometry via UiRenderer::DrawTexturedRectRegion (a transient vertex
// buffer) rather than drawing the full quad and masking it in the fragment
// shader.
//
// NOTE ON COLOR: this fixes a pre-existing bug rather than preserving it —
// the old shader path set the fill's "u_Color" uniform from the raw `color`
// field AFTER the outer GetFinalColor()-based "u_Color" was already set,
// silently overwriting it, so parent-color inheritance never actually
// applied to the fill or background despite `inheritParentColor` existing.
// Both passes now correctly multiply by GetFinalColor(), same as every
// other element. If you were relying on the old (arguably accidental)
// behaviour, multiply inheritParentColor = false on affected instances.
//
// NOTE ON 9-SLICE: UiElement::NineSliceEnabled is not supported here — see
// UiElement.h, 9-slice and a partial rect (which is what the fill/background
// split is) aren't combinable in the same draw yet.
// ---------------------------------------------------------------------------

enum class UiProgressDirection { LeftToRight, RightToLeft, TopToBottom, BottomToTop };

class UiProgressBar : public UiElement
{
public:
    std::string BackgroundImage = "GameData/cat.png";
    std::string ProgressImage = "GameData/cat.png";
    float Progress = 0.f;
    vec4 BackgroundColor = vec4(1);

    UiProgressDirection Direction = UiProgressDirection::LeftToRight;

    void FinalizeChildren() override
    {
        UiElement::FinalizeChildren();
        progressFinal = std::clamp(Progress, 0.f, 1.f);
        directionFinal = Direction;
    }

    void Draw() override
    {
        Texture* bgTex = AssetRegistry::GetTextureFromFile(BackgroundImage);
        if (!bgTex->valid) bgTex = AssetRegistry::GetTextureFromFile("GameData/textures/generic/white.png");

        Texture* fgTex = AssetRegistry::GetTextureFromFile(ProgressImage);
        if (!fgTex->valid) fgTex = AssetRegistry::GetTextureFromFile("GameData/textures/generic/white.png");

        vec2 bgMin, bgSize, fillMin, fillSize;
        ComputeFillRects(bgMin, bgSize, fillMin, fillSize);

        RectPosition = bgMin;
        RectSize = bgSize;
        DrawSelfTextured(bgTex->getHandle(), BackgroundColor * GetFinalColor(),
            static_cast<float>(bgTex->width), static_cast<float>(bgTex->height));

        RectPosition = fillMin;
        RectSize = fillSize;
        DrawSelfTextured(fgTex->getHandle(), color * GetFinalColor(),
            static_cast<float>(fgTex->width), static_cast<float>(fgTex->height),
            /*useEffects=*/false);

        // Restore the default full rect so RectPosition/RectSize don't leak
        // as stale state to anything else that might read them off this
        // instance between frames.
        RectPosition = vec2(0.f, 0.f);
        RectSize = vec2(1.f, 1.f);

        UiElement::Draw();
    }

private:
    float progressFinal = 0.f;
    UiProgressDirection directionFinal = UiProgressDirection::LeftToRight;

    // Splits the element into [background-visible-rect] and
    // [fill-visible-rect], both in local [0,1] space (see
    // UiElement::RectPosition/RectSize) — together they exactly cover
    // [0,1], no overlap, no gap.
    void ComputeFillRects(vec2& bgMin, vec2& bgSize, vec2& fillMin, vec2& fillSize) const
    {
        const float p = progressFinal;

        switch (directionFinal)
        {
        default:
        case UiProgressDirection::LeftToRight:
            fillMin = vec2(0.f, 0.f);      fillSize = vec2(p, 1.f);
            bgMin   = vec2(p, 0.f);        bgSize   = vec2(1.f - p, 1.f);
            break;
        case UiProgressDirection::RightToLeft:
            fillMin = vec2(1.f - p, 0.f);  fillSize = vec2(p, 1.f);
            bgMin   = vec2(0.f, 0.f);      bgSize   = vec2(1.f - p, 1.f);
            break;
        case UiProgressDirection::TopToBottom:
            fillMin = vec2(0.f, 0.f);      fillSize = vec2(1.f, p);
            bgMin   = vec2(0.f, p);        bgSize   = vec2(1.f, 1.f - p);
            break;
        case UiProgressDirection::BottomToTop:
            fillMin = vec2(0.f, 1.f - p);  fillSize = vec2(1.f, p);
            bgMin   = vec2(0.f, 0.f);      bgSize   = vec2(1.f, 1.f - p);
            break;
        }
    }
};
