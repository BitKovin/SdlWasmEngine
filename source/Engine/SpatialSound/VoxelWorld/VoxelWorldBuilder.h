#pragma once
#include <glm/glm.hpp>
#include <cstdint>

class VoxelWorldBuilder
{
public:
    virtual ~VoxelWorldBuilder() = default;

    // worldPos is CENTER of voxel in world space
    // return density / traversal price [0..255]
    virtual uint8_t GetDensity(const glm::vec3& worldPos) const = 0;
};
