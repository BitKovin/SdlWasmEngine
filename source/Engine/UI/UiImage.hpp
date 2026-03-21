#pragma once

#include "UiElement.h"

#include "UiRenderer.h"

#include "../Texture.hpp"
#include "../AssetRegistry.h"

#include "../Level.hpp"

class UiImage : public UiElement
{

private:

public:

	string ImagePath = "GameData/textures/ui/white.png";

	UiImage()
	{ 
		//tex = AssetRegistry::GetTextureFromFile("GameData/cat.png");
	}
	~UiImage()
	{

	}

	void Draw()
	{

		vec2 pos = finalizedPosition + finalizedOffset;

		if (Level::ChangingLevel)
		{
			tex = AssetRegistry::GetTextureFromFile(ImagePath);
		}
		else
		{
			if (tex == nullptr)
			{
				tex = AssetRegistry::GetTextureFromFile(ImagePath);
				if (tex->valid == false)
					tex = AssetRegistry::GetTextureFromFile("GameData/textures/generic/white.png");
			}
		}

		if (PixelShader.empty())
		{
			UiRenderer::DrawTexturedRect(pos, finalizedSize,rotation, pivot, tex->getHandle(), GetFinalColor());
		}
		else
		{
			UiRenderer::DrawTexturedRectShader(pos, finalizedSize, rotation, pivot, tex->getHandle(), GetFinalColor(), PixelShader);
		}


		UiElement::Draw();
	}

private:

	Texture* tex = nullptr;

};