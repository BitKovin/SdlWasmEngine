#pragma once
#include "UiElement.h"

#include "UiContentBox.hpp"

class UiVerticalBox : public UiContentBox
{
public:
	
	float ContentDistance = 5;

    void Update()
    {
        float yOffset = 0.0f;

        for (auto& elem : children)
        {
            elem->position = vec2(0.0f, yOffset);
            yOffset += elem->GetSize().y + ContentDistance;
        }

        UiElement::Update();
    }

private:

};