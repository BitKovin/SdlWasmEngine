#pragma once

#include "UiElement.h"

#include "UiRenderer.h"

#include "../Texture.hpp"
#include "../AssetRegisty.h"

#include "../Input.h"

class UiButton : public UiElement
{

private:

	bool pendingClick = false;

public:

	Texture* tex;

	std::function<void()>* onClick = nullptr;

	UiButton()
	{ 
		tex = AssetRegistry::GetTextureFromFile("GameData/cat.png");
	}
	~UiButton()
	{

	}

	void Update()
	{

		UiElement::Update();


		if (hovering) {
			if (Input::GetAction("click")->Pressed())
			{
				if (onClick)
				{
					(*onClick)();
				}
			}
		}
	}

	bool HasPendingClick()
	{
		if (pendingClick)
		{
			pendingClick = false;
			return true;
		}

		return false;
	}

	void Draw()
	{

		vec2 pos = position + offset;

		UiRenderer::DrawTexturedRect(pos, size, tex->getID());

		UiElement::Draw();
	}

};