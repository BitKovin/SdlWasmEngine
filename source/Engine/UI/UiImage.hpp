#pragma once

#include "UiElement.h"
#include "UiRenderer.h"
#include "../Texture.hpp"
#include "../AssetRegistry.h"
#include "../Level.hpp"

class UiImage : public UiElement
{
public:
    string ImagePath = "GameData/textures/ui/white.png";

    UiImage() {}
    ~UiImage() {}

    void Draw() override
    {
        if (Level::ChangingLevel)
        {
            tex = AssetRegistry::GetTextureFromFile(ImagePath);
        }
        else
        {
            if (tex == nullptr)
                tex = AssetRegistry::GetTextureFromFile(ImagePath);
        }

        // tex may still be streaming in (lazy load outside a level load -
        // see AssetRegistry.h). Pick the fallback for THIS draw without
        // losing our reference to the real texture - once tex->valid flips,
        // the very next frame picks it up on its own, no extra bookkeeping.
        Texture* drawTex = tex->valid ? tex : AssetRegistry::GetTextureFromFile("GameData/textures/generic/white.png", AssetLoadTier::VisualImmediately);

        if (tex->valid)
        {
            // Effects vs PixelShader vs plain, partial-rect vs 9-slice vs full —
            // all handled by UiElement; see UiElement.h for what RectPosition/
            // RectSize/NineSliceEnabled do.
            DrawSelfTextured(drawTex->getHandle(), GetFinalColor(), static_cast<float>(drawTex->width), static_cast<float>(drawTex->height));

        }


        UiElement::Draw();
    }

private:
    Texture* tex = nullptr;
};
