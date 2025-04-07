#pragma once

#include "UiElement.h"
#include "../Camera.h"

class UiViewport : public UiElement
{
public:
	UiViewport()
	{

	}
	~UiViewport()
	{

	}

	void Update()
	{
		size = GetSize();

		parentTopLeft = vec2();
		parentBottomRight = size;


		UiElement::Update();

	}

	vec2 GetSize()
	{
		return vec2(1080 * Camera::AspectRatio, 1080);
	}

private:

};