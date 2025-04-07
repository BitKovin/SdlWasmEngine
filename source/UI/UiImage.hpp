#pragma once

#include "UiElement.h"

#include "UiRenderer.h"

#include "../Texture.hpp"
#include "../AssetRegisty.h"

class UiImage : public UiElement
{
public:

	Texture* tex;

	UiImage()
	{ 
		tex = AssetRegistry::GetTextureFromFile("GameData/cat.png");
	}
	~UiImage()
	{

	}

	void Draw()
	{

		vec2 pos = position + offset;

		UiRenderer::DrawTexturedRect(pos, size, tex->getID());

		UiElement::Draw();
	}

private:

};