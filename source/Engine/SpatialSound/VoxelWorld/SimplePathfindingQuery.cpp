#include "SimplePathfindingQuery.h"
#include "TiledVoxelWorld.h"
#include <queue>
#include <limits>
#include <cmath>
#include <algorithm>
#include <cstdint>

const glm::ivec3 SimplePathfindingQuery::NEI[26] = {
    { -1,-1,-1}, { 0,-1,-1}, { 1,-1,-1},
    { -1, 0,-1}, { 0, 0,-1}, { 1, 0,-1},
    { -1, 1,-1}, { 0, 1,-1}, { 1, 1,-1},

    { -1,-1, 0}, { 0,-1, 0}, { 1,-1, 0},
    { -1, 0, 0},             { 1, 0, 0},
    { -1, 1, 0}, { 0, 1, 0}, { 1, 1, 0},

    { -1,-1, 1}, { 0,-1, 1}, { 1,-1, 1},
    { -1, 0, 1}, { 0, 0, 1}, { 1, 0, 1},
    { -1, 1, 1}, { 0, 1, 1}, { 1, 1, 1}
};

SimplePathfindingQuery::SimplePathfindingQuery(const TiledVoxelWorld& world,
    float distanceWeight,
    float densityWeight,
    int maxVisited)
    : m_World(world)
    , m_DistanceWeight(distanceWeight)
    , m_DensityWeight(densityWeight)
    , m_MaxVisited(maxVisited)
{
}

PathResult SimplePathfindingQuery::FindPath(const glm::vec3& startWorld, const glm::vec3& endWorld) const
{
    PathResult result;

    // Convert start/end to voxel indices (safe)
    glm::ivec3 startV, endV;
    if (!m_World.WorldToVoxelSafe(startWorld, startV)) return result;
    if (!m_World.WorldToVoxelSafe(endWorld, endV))   return result;

    // Quick rejection if start or end are blocked
    uint8_t startD = m_World.GetVoxelDensityAt(startV);
    uint8_t endD = m_World.GetVoxelDensityAt(endV);
    //if (startD == BLOCKED_DENSITY || endD == BLOCKED_DENSITY) return result;

    // trivial case
    if (startV == endV)
    {
        result.path.push_back(m_World.VoxelCenterWorldPublic(startV));
        result.traveledDistance = 0.0f;
        result.traveledDensityCost = 0.0f;
        result.totalCost = 0.0f;
        return result;
    }

    const glm::ivec3 dims = m_World.VoxelDimensions();
    const int vx = dims.x, vy = dims.y, vz = dims.z;
    const size_t totalVox = m_World.VoxelCount();
    const float INF = std::numeric_limits<float>::infinity();

    // A* arrays
    std::vector<float> gCost(totalVox, INF);
    std::vector<int> parent(totalVox, -1);
    std::vector<char> closed(totalVox, 0);

    auto idxOf = [&](const glm::ivec3& v)->int { return m_World.FlattenIndex(v); };

    // Heuristic: only geometric distance scaled by distanceWeight (admissible)
    auto heuristic = [&](const glm::ivec3& a, const glm::ivec3& b)->float {
        glm::vec3 ca = m_World.VoxelCenterWorldPublic(a);
        glm::vec3 cb = m_World.VoxelCenterWorldPublic(b);
        return m_DistanceWeight * glm::length(cb - ca);
        };

    struct Node { float f; float g; int idx; };
    struct Cmp { bool operator()(Node const& a, Node const& b) const { return a.f > b.f; } };
    std::priority_queue<Node, std::vector<Node>, Cmp> open;

    int startIdx = idxOf(startV);
    int endIdx = idxOf(endV);

    gCost[startIdx] = 0.0f;
    open.push({ heuristic(startV, endV), 0.0f, startIdx });

    int visited = 0;

    while (!open.empty())
    {
        Node cur = open.top(); open.pop();

        if (closed[cur.idx]) continue; // stale entry
        closed[cur.idx] = 1;

        ++visited;
        if (visited > m_MaxVisited) return result; // safety

        if (cur.idx == endIdx)
        {
            // reconstruct path as voxel indices
            std::vector<int> revIdx;
            int it = cur.idx;
            while (it != -1)
            {
                revIdx.push_back(it);
                it = parent[it];
            }
            std::reverse(revIdx.begin(), revIdx.end());

            // convert to world centers and compute traveledDistance & traveledDensity
            result.path.reserve(revIdx.size());
            float traveledDist = 0.0f;
            float traveledDens = 0.0f;

            for (size_t i = 0; i < revIdx.size(); ++i)
            {
                glm::ivec3 v = m_World.UnflattenIndex(revIdx[i]);
                glm::vec3 center = m_World.VoxelCenterWorldPublic(v);
                result.path.push_back(center);

                if (i >= 1)
                {
                    // geometric distance between consecutive centers
                    glm::vec3 prevCenter = result.path[i - 1];
                    traveledDist += glm::length(center - prevCenter);

                    // density contribution: average of the two voxel densities (in meters)
                    glm::ivec3 prevV = m_World.UnflattenIndex(revIdx[i - 1]);
                    uint8_t dens0 = m_World.GetVoxelDensityAt(prevV);
                    uint8_t dens1 = m_World.GetVoxelDensityAt(v);
                    // densities are already in meters (per your spec)
                    float avgDensityMeters = (float(dens0) + float(dens1)) * 0.5f;
                    traveledDens += avgDensityMeters;
                }
            }

            result.traveledDistance = traveledDist;
            result.traveledDensityCost = traveledDens;
            result.totalCost = m_DistanceWeight * traveledDist + m_DensityWeight * traveledDens;
            return result;
        }

        glm::ivec3 curV = m_World.UnflattenIndex(cur.idx);
        float gcur = cur.g;
        glm::vec3 curCenter = m_World.VoxelCenterWorldPublic(curV);

        // explore 26 neighbors
        for (int ni = 0; ni < 26; ++ni)
        {
            glm::ivec3 nbV = curV + NEI[ni];

            // bounds
            if (nbV.x < 0 || nbV.y < 0 || nbV.z < 0 ||
                nbV.x >= vx || nbV.y >= vy || nbV.z >= vz) continue;

            int nbIdx = idxOf(nbV);
            if (closed[nbIdx]) continue;

            // skip impassable
            uint8_t nbDensity = m_World.GetVoxelDensityAt(nbV);
            //if (nbDensity == BLOCKED_DENSITY) continue;

            glm::vec3 nbCenter = m_World.VoxelCenterWorldPublic(nbV);
            float geomDist = glm::length(nbCenter - curCenter);
            float distanceComponent = m_DistanceWeight * geomDist;

            // density component (densities are meters)
            uint8_t densCur = m_World.GetVoxelDensityAt(curV);
            uint8_t densNb = nbDensity;
            float avgDensityMeters = (float(densCur) + float(densNb)) * 0.5f;
            float densityComponent = m_DensityWeight * avgDensityMeters;

            float edgeCost = distanceComponent + densityComponent;
            float tentativeG = gcur + edgeCost;

            if (tentativeG < gCost[nbIdx])
            {
                gCost[nbIdx] = tentativeG;
                parent[nbIdx] = cur.idx;
                float f = tentativeG + heuristic(nbV, endV);
                open.push({ f, tentativeG, nbIdx });
            }
        }
    }

    // no path found
    return result;
}
