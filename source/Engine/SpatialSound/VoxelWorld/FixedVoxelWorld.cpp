// FixedVoxelWorld.cpp
#include "FixedVoxelWorld.h"
#include "VoxelWorldBuilder.h"

FixedVoxelWorld::FixedVoxelWorld(const ivec3& size, const VoxelWorldBuilder& builder)
    : m_size(size), m_data(size.x* size.y* size.z, 0) {
    for (int z = 0; z < size.z; ++z) {
        for (int y = 0; y < size.y; ++y) {
            for (int x = 0; x < size.x; ++x) {
                vec3 center(x + 0.5f, y + 0.5f, z + 0.5f);
                uint8_t mat = builder.GetValue(center);
                m_data[x + y * m_size.x + z * m_size.x * m_size.y] = mat;
            }
        }
    }
}

FixedVoxelWorld::FixedVoxelWorld(const ivec3& size, std::vector<uint8_t> data)
    : m_size(size), m_data(data)
{
}

bool FixedVoxelWorld::inBounds(const ivec3& pos) const {
    return all(greaterThanEqual(pos, ivec3(0))) && all(lessThan(pos, m_size));
}

uint8_t FixedVoxelWorld::GetValue(const vec3& worldPos) const {
    ivec3 pos = floor(worldPos);
    if (!inBounds(pos)) {
        return 0; // Air or out-of-bounds
    }
    return m_data[pos.x + pos.y * m_size.x + pos.z * m_size.x * m_size.y];
}

ivec3 FixedVoxelWorld::GetSize() const { return m_size; }