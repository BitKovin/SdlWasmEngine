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

        if (PixelShader.empty())
            UiRenderer::DrawTexturedRect(finalizedMatrix, finalizedSize, tex->getHandle(), GetFinalColor());
        else
            UiRenderer::DrawTexturedRectShader(finalizedMatrix, finalizedSize, tex->getHandle(), GetFinalColor(), PixelShader);

        UiElement::Draw();
    }

private:
    Texture* tex = nullptr;
};
