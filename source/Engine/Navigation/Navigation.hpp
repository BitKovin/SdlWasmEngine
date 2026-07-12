#pragma once

#define RC_MAX_LAYERS_DEF 255

#include "Recast/Recast.h"
#include "Detour/DetourNavMesh.h"
#include "Detour/DetourNavMeshBuilder.h"
#include "Detour/DetourTileCache.h"
#include "Detour/DetourTileCacheBuilder.h"
#include "Detour/DetourCrowd.h"

#include "CrowdAgent.h"

#include <mutex>


#include <vector>

#include "Detour/DetourNavMeshQuery.h"

using namespace std;

class CustomFilter : public dtQueryFilter
{
    bool passFilter(const dtPolyRef /*ref*/, const dtMeshTile* /*tile*/, const dtPoly* poly) const override
    {
        return true;// poly->getArea() == DT_TILECACHE_WALKABLE_AREA; // Match area set during navmesh build
    }
};

class NavigationSystem
{

private:


    static inline dtTileCache* tileCache = nullptr;
    static inline dtTileCacheAlloc* talloc = nullptr;
    static inline dtTileCacheCompressor* tcomp = nullptr;
    static inline dtTileCacheMeshProcess* tmproc = nullptr;

    static std::recursive_mutex mainLock;

    static vector<dtObstacleRef> obstacles;

    static bool HasLineOfSight(const vec3& pointA, const vec3& pointB);

public:

	static inline vec3 WorldMin = vec3(0, 0, 0);
	static inline vec3 WorldMax = vec3(0, 0, 0);

    static inline dtNavMesh* navMesh = nullptr;
    static inline dtCrowd* g_crowd = nullptr;

    static void DestroyNavData();

    static bool DebugDrawNavMeshEnabled;

    static void DestroyAllObstacles()
    {
        for (auto obstacle : obstacles)
        {
            RemoveObstacle(obstacle);
        }
    }

    static void Update();

    static void GenerateNavData();

    static void InitCrowd(int maxAgents = 128, float maxAgentRadius = 0.6f);

  
    // DrawNavmesh renders every edge in the navigation mesh using DebugDraw::Line.
// It uses the dtNavMesh's internal tile storage to iterate over all polygons.
    static void DrawNavmesh();

    static void RemoveObstacle(dtObstacleRef obstacleRef);

    static dtObstacleRef CreateObstacleBox(const glm::vec3& min, const glm::vec3& max);

    static std::vector<glm::vec3> FindSimplePath(glm::vec3 start, glm::vec3 target, float acceptanceRadius = 0.0f, bool* outReached = nullptr, bool allowPartialPath = false);

    static std::vector<glm::vec3> FindFleePath(
        const glm::vec3& start,
        const glm::vec3& threatPos,
        float desiredDist = 25,
        int maxCandidates = 20,
        float npcSpeed = 4.0f,
        float playerSpeed = 5.0f);

    static std::vector<glm::vec3> FindFleePathSimple(
        const glm::vec3& start,
        const glm::vec3& threatPos,
        float desiredDist = 25,
        int maxCandidates = 20,
        float npcSpeed = 4.0f,
        float playerSpeed = 5.0f);

    static bool ProjectPointToNavMesh(const glm::vec3& sample, const glm::vec3& navSnapTolerance, glm::vec3& navPos);


};

