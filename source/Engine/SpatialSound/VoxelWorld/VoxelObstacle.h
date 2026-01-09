#pragma once
#include "../../glm.h"
#include <cstdint>

struct VoxelObstacle
{
    int id;                // unique id
    glm::vec3 min;
    glm::vec3 max;
    uint8_t density = 255;
    bool dynamic = true;
};
