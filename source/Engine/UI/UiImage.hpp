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
            {
                tex = AssetRegistry::GetTextureFromFile(ImagePath);
                if (!tex->valid)
                    tex = AssetRegistry::GetTextureFromFile("GameData/textures/generic/white.png");
            }
        }

        // Effects vs PixelShader vs plain, partial-rect vs 9-slice vs full —
        // all handled by UiElement; see UiElement.h for what RectPosition/
        // RectSize/NineSliceEnabled do.
        DrawSelfTextured(tex->getHandle(), GetFinalColor(), static_cast<float>(tex->width), static_cast<float>(tex->height));

        UiElement::Draw();
    }

private:
    Texture* tex = nullptr;
};
