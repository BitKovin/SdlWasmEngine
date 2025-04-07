#pragma once
#include "UiElement.h"

class UiContentBox : public UiElement
{
public:

    glm::vec2 GetSize()
    {
        if (children.empty())
            return glm::vec2(0.0f);

        glm::vec2 topLeft(FLT_MAX, FLT_MAX);
        glm::vec2 bottomRight(-FLT_MAX, -FLT_MAX);

        for (const auto elem : children)
        {
            if (elem->topLeft.x < topLeft.x)
                topLeft.x = elem->topLeft.x;

            if (elem->topLeft.y < topLeft.y)
                topLeft.y = elem->topLeft.y;

            if (elem->bottomRight.x > bottomRight.x)
                bottomRight.x = elem->bottomRight.x;

            if (elem->bottomRight.y > bottomRight.y)
                bottomRight.y = elem->bottomRight.y;
        }

        return bottomRight - topLeft;
    }



private:

};