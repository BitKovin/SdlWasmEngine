#pragma once

#include <glm/glm.hpp>
#include <vector>

using namespace glm;

class VoxelWorldBuilder; // Forward declaration

class FixedVoxelWorld {
private:
    ivec3 m_size;


public:

    std::vector<uint8_t> m_data; // Flat array: x + y*size.x + z*size.x*size.y

    bool inBounds(const ivec3& pos) const;

    FixedVoxelWorld(const ivec3& size, const VoxelWorldBuilder& builder);
    FixedVoxelWorld(const ivec3& size, std::vector<uint8_t> data);

    uint8_t GetValue(const vec3& worldPos) const;

    ivec3 GetSize() const;
};
