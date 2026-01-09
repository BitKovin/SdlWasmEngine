#pragma once
#include <glm/glm.hpp>
#include <vector>

class TiledVoxelWorld;

struct PathResult
{
    std::vector<glm::vec3> path;      // world-space voxel centers, empty == failure
    float traveledDistance = 0.0f;    // raw Euclidean distance sum
    float traveledDensityCost = 0.0f; // density-based cost (densityWeight * densNorm * voxelVolume summed)
    float totalCost = 0.0f;          // distanceWeight * traveledDistance + traveledDensityCost

};

class SimplePathfindingQuery
{
public:
    // distanceWeight: multiplier for geometric distance component
    // densityWeight: multiplier for density component (densNorm * voxelVolume)
    // maxVisited: safety cap on number of popped nodes from open set
    SimplePathfindingQuery(const TiledVoxelWorld& world,
        float distanceWeight = 1.0f,
        float densityWeight = 1.0f,
        int maxVisited = 200000);

    // Finds path from startWorld to endWorld. If path empty -> no path found.
    // PathResult contains path and cost breakdowns.
    PathResult FindPath(const glm::vec3& startWorld, const glm::vec3& endWorld) const;


    const TiledVoxelWorld& m_World;
    float m_DistanceWeight;
    float m_DensityWeight;
    int   m_MaxVisited;

private:
    // 26 neighbors (all combos except (0,0,0))
    static const glm::ivec3 NEI[26];
};
