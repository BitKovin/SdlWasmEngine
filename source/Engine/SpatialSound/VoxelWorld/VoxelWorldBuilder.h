#pragma once

#include <glm/glm.hpp>

using namespace glm;

class VoxelWorldBuilder {
public:
    virtual ~VoxelWorldBuilder() = default;
    // worldPos is CENTER of voxel in world space
    // returns material ID
    virtual uint8_t GetValue(const vec3& worldPos) const = 0;
};