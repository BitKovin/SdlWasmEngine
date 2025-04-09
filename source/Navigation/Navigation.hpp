#pragma once

#include "Recast/Recast.h"
#include "Detour/DetourNavMesh.h"
#include "Detour/DetourNavMeshBuilder.h"
#include "Detour/DetourTileCache.h"
#include "Detour/DetourTileCacheBuilder.h"

#include <mutex>

#include "../Time.hpp"

#include "../DebugDraw.hpp"


class NavigationSystem
{

private:

	static dtNavMesh* navMesh;
	static dtTileCache* tileCache;

    static mutex mainLock;

    static vector<dtObstacleRef> obstacles;

public:

    static void DestroyNavData();

    static void Update()
    {
        std::lock_guard<std::mutex> lock(mainLock);

        bool upToDate = false;
        auto status = tileCache->update(Time::DeltaTimeF, navMesh, &upToDate);

        if (dtStatusFailed(status))
        {
            std::printf("Tile cache update failed. Status: %u\n", status);
        }
        else if (!upToDate)
        {
            std::printf("Tile cache still processing updates.\n");
        }
    }

    static void GenerateNavData();

    // DrawNavmesh renders every edge in the navigation mesh using DebugDraw::Line.
// It uses the dtNavMesh's internal tile storage to iterate over all polygons.
    static void DrawNavmesh()
    {
        if (!navMesh)
            return;

        std::lock_guard<std::mutex> lock(mainLock);

        const int maxTiles = navMesh->getMaxTiles();
        for (int i = 0; i < maxTiles; ++i)
        {
            const dtMeshTile* tile = navMesh->getTile(i);
            if (!tile || !tile->header)
                continue;

            for (int j = 0; j < tile->header->polyCount; ++j)
            {
                const dtPoly* poly = &tile->polys[j];

                // Only draw standard ground polys (avoid off-mesh connections, etc.)
                if (poly->getType() != DT_POLYTYPE_GROUND)
                    continue;

                const int vertCount = poly->vertCount;
                for (int k = 0; k < vertCount; ++k)
                {
                    const int currentIdx = poly->verts[k];
                    const int nextIdx = poly->verts[(k + 1) % vertCount];

                    if (currentIdx >= tile->header->vertCount || nextIdx >= tile->header->vertCount)
                        continue; // Skip invalid indices

                    const float* v0 = &tile->verts[currentIdx * 3];
                    const float* v1 = &tile->verts[nextIdx * 3];

                    glm::vec3 p0(v0[0], v0[1], v0[2]);
                    glm::vec3 p1(v1[0], v1[1], v1[2]);

                    // Skip degenerate edges
                    if (glm::distance(p0, p1) < 0.001f)
                        continue;

                    DebugDraw::Line(p0, p1, 0.2f, 0.03f); // longer duration for better visibility
                }
            }
        }
    }

    static void RemoveObstacle(dtObstacleRef obstacleRef)
    {
        if (!tileCache || obstacleRef == 0)
            return;

        std::lock_guard<std::mutex> lock(mainLock);



        // Remove the obstacle using its reference
        const dtStatus status = tileCache->removeObstacle(obstacleRef);
        if (dtStatusFailed(status))
        {
            //std::printf("Failed to remove obstacle with ref: %u. Status: %u\n", obstacleRef, status);
        }
        else
        {
            //std::printf("Obstacle with ref: %u removed successfully.\n", obstacleRef);

        }

        // Remove from tracking vector if used
        auto it = std::find(obstacles.begin(), obstacles.end(), obstacleRef);
        if (it != obstacles.end())
            obstacles.erase(it);

    }

    static dtObstacleRef CreateObstacleBox(const glm::vec3& min, const glm::vec3& max)
    {
        if (!tileCache) // Ensure tileCache is valid
            return 0;

        glm::vec3 adjustedMin = min;
        glm::vec3 adjustedMax = max;

        // Ensure each axis (x, y, z) has a minimum length of 2.0 units
        for (int i = 0; i < 3; ++i)
        {
            const float currentLength = adjustedMax[i] - adjustedMin[i];
            if (currentLength < 2.0f)
            {
                const float delta = (2.0f - currentLength) * 0.5f;
                adjustedMin[i] -= delta;
                adjustedMax[i] += delta;
            }
        }

        dtObstacleRef obstacleRef = 0;

        // Add the adjusted box obstacle
        dtStatus status = tileCache->addBoxObstacle(
            &adjustedMin.x,     // Adjusted min coordinates
            &adjustedMax.x,     // Adjusted max coordinates
            &obstacleRef
        );

        if (dtStatusFailed(status))
        {
            //std::printf("Failed to add box obstacle. Status: %u\n", status);
            return 0;
        }

        obstacles.push_back(obstacleRef);

        //std::printf("Box obstacle added. Ref: %u\n", obstacleRef);
        return obstacleRef;
    }

};