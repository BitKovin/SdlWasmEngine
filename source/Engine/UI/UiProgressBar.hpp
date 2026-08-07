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
// NOTE ON COLOR: BackgroundColor and `color` (the fill tint) are independent
// — this was broken in an earlier pass of this rewrite, where both passes
// multiplied by GetFinalColor(), which itself bakes in this element's own
// `color` field. That meant the background silently picked up the fill
// tint too. Both passes now multiply by the PARENT chain only
// (parent->GetFinalColor() when inheritParentColor is set), never by each
// other's own color field — see the parentTint variable in Draw() below.
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

        // `color` (inherited from UiElement) is repurposed as the FILL tint
        // only — GetFinalColor() bakes `color` into itself, so using it for
        // the background pass too would tint the background by the fill
        // color as a side effect. Parent-chain tinting should still apply to
        // both passes; just not each other's own field.
        const vec4 parentTint = (inheritParentColor && parent) ? parent->GetFinalColor() : vec4(1.f);

        RectPosition = vec2(0);
        RectSize = vec2(1);//background casts effects. Keep it always visible
        DrawSelfTextured(bgTex->getHandle(), BackgroundColor * parentTint,
            static_cast<float>(bgTex->width), static_cast<float>(bgTex->height));

        RectPosition = fillMin;
        RectSize = fillSize;
        DrawSelfTextured(fgTex->getHandle(), color * parentTint,
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
