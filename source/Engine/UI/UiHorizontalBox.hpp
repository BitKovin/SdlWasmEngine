#pragma once
#include "UiElement.h"

#include "UiContentBox.hpp"

class UiHorizontalBox : public UiContentBox
{
public:
	
	float ContentDistance = 5;

    void Update()
    {
        float xOffset = 0.0f;

        for (auto elem : children)
        {
            elem->position = vec2(xOffset, 0.0f);
            xOffset += elem->GetSize().x + ContentDistance;
        }

        UiElement::Update();
    }


private:

};