#pragma once
#include "UiElement.h"

#include "UiContentBox.hpp"

class UiVerticalBox : public UiContentBox
{
public:
	
	float ContentDistance = 5;

	void Update()
	{
		int i = -1;

		for (auto elem : children)
		{

			i++;

			elem->position = vec2(0, elem->size.y + ContentDistance) * (float)i;

		}

		UiElement::Update();
	}

private:

};