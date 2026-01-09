#pragma once
#include "../../glm.h"
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include "VoxelTile.h"
#include "VoxelObstacle.h"
#include "VoxelWorldBuilder.h"

class TiledVoxelWorld
{
public:

    TiledVoxelWorld(){}

    TiledVoxelWorld(const glm::vec3& minWorld,
        const glm::vec3& maxWorld,
        float voxelSize);

    void BuildStatic(const VoxelWorldBuilder& builder, int subdivisions = 1);

    int AddObstacle(const VoxelObstacle& obs);      // returns unique obstacle ID
    void RemoveObstacle(int obstacleId);

    void Update();  // rebuilds dirty tiles

    Voxel SampleWorld(const glm::vec3& worldPos) const;

    size_t GetVoxelWorldMemoryBytes();

    struct TileAddress
    {
        glm::ivec3 tile;
        glm::ivec3 local;
    };

    TileAddress AddressFromVoxel(const glm::ivec3& v) const;
    glm::ivec3 WorldToVoxel(const glm::vec3& p) const;
    glm::vec3  VoxelCenterWorld(const glm::ivec3& v) const;

    VoxelTile& Tile(const glm::ivec3& t);
    const VoxelTile& Tile(const glm::ivec3& t) const;

    void MarkTilesDirty(const VoxelObstacle& obs);
    void RasterizeTile(VoxelTile& tile, const glm::ivec3& tileIndex, const VoxelObstacle& obs);

    bool WorldToVoxelSafe(const glm::vec3& worldPos, glm::ivec3& outVoxel) const;
    glm::vec3 VoxelCenterWorldPublic(const glm::ivec3& v) const;
    uint8_t GetVoxelDensityAt(const glm::ivec3& v) const;
    size_t VoxelCount() const;
    int FlattenIndex(const glm::ivec3& v) const;
    glm::ivec3 UnflattenIndex(int idx) const;

	glm::ivec3 VoxelDimensions() const { return m_VoxelDim; }
	float GetVoxelSize() const { return m_VoxelSize; }

private:
    glm::vec3 m_Min;
    glm::vec3 m_Max;
    float m_VoxelSize;

    glm::ivec3 m_VoxelDim;
    glm::ivec3 m_TileDim;

    std::vector<VoxelTile> m_Tiles;

    std::unordered_map<int, VoxelObstacle> m_Obstacles;
    std::unordered_set<glm::ivec3, std::hash<glm::ivec3>> m_DirtyTiles;

    int m_NextObstacleId = 1;
};
