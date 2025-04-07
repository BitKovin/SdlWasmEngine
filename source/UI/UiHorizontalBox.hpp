#pragma once
#include "UiElement.h"

#include "UiContentBox.hpp"

class UiHorizontalBox : public UiContentBox
{
public:
	
	float ContentDistance = 5;

	void Update()
	{
		int i = -1;

		for (auto elem : children)
		{

			i++;

			elem->position = vec2(elem->size.x + ContentDistance,0) * (float)i;

		}

		UiElement::Update();
	}

private:

};