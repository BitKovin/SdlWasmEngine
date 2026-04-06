#pragma once

#include "glm.h"

class RandomHelper
{
public:

    static float RandomFloat();
    static int RandomInt();

    static glm::vec3 RandomPosition(float radius);
};

