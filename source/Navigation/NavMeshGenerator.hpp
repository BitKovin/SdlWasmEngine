#include <vector>
#include <cstdint>
#include <cmath>
#include <limits>
#include <cstring>
#include <glm/glm.hpp>

// Include Recast/Detour headers
#include "Recast/Recast.h"
#include "Detour/DetourNavMesh.h"
#include "Detour/DetourNavMeshBuilder.h"

// A simple wrapper class to generate a navmesh
class NavMeshGenerator {
public:

    struct NavMeshConfig {
        float cellSize = 0.3f;
        float cellHeight = 0.2f;
        float agentHeight = 2.0f;
        float agentRadius = 0.6f;
        float agentMaxClimb = 0.9f;
        float agentMaxSlope = 45.0f;
        float regionMinSize = 8.0f;
        float regionMergeSize = 20.0f;
        float edgeMaxLen = 12.0f;
        float edgeMaxError = 1.3f;
        float vertsPerPoly = 6.0f;
        float detailSampleDist = 6.0f;
        float detailSampleMaxError = 1.0f;
    };

    dtNavMesh* CreateNavMesh(const std::vector<glm::vec3>& vertices, const std::vector<uint32_t>& indices, const NavMeshConfig& config) {
        if (vertices.empty() || indices.empty() || indices.size() % 3 != 0)
            return nullptr;

        rcContext* context = new rcContext();

        // Compute AABB
        glm::vec3 bmin = vertices[0];
        glm::vec3 bmax = vertices[0];
        for (const auto& v : vertices) {
            bmin.x = std::min(bmin.x, v.x);
            bmin.y = std::min(bmin.y, v.y);
            bmin.z = std::min(bmin.z, v.z);
            bmax.x = std::max(bmax.x, v.x);
            bmax.y = std::max(bmax.y, v.y);
            bmax.z = std::max(bmax.z, v.z);
        }

        // Configure Recast
        rcConfig rcCfg;
        memset(&rcCfg, 0, sizeof(rcCfg));
        rcCfg.cs = config.cellSize;
        rcCfg.ch = config.cellHeight;
        rcCfg.walkableSlopeAngle = config.agentMaxSlope;
        rcCfg.walkableHeight = static_cast<int>(ceilf(config.agentHeight / rcCfg.ch));
        rcCfg.walkableClimb = static_cast<int>(floorf(config.agentMaxClimb / rcCfg.ch));
        rcCfg.walkableRadius = static_cast<int>(ceilf(config.agentRadius / rcCfg.cs));
        rcCfg.maxEdgeLen = static_cast<int>(config.edgeMaxLen / rcCfg.cs);
        rcCfg.maxSimplificationError = config.edgeMaxError;
        rcCfg.minRegionArea = static_cast<int>(rcSqr(config.regionMinSize));
        rcCfg.mergeRegionArea = static_cast<int>(rcSqr(config.regionMergeSize));
        rcCfg.maxVertsPerPoly = static_cast<int>(config.vertsPerPoly);
        rcCfg.detailSampleDist = config.detailSampleDist * rcCfg.cs;
        rcCfg.detailSampleMaxError = config.detailSampleMaxError * rcCfg.ch;
        rcVcopy(rcCfg.bmin, &bmin.x);
        rcVcopy(rcCfg.bmax, &bmax.x);
        rcCalcGridSize(rcCfg.bmin, rcCfg.bmax, rcCfg.cs, &rcCfg.width, &rcCfg.height);

        // Build heightfield
        rcHeightfield* heightfield = rcAllocHeightfield();
        if (!rcCreateHeightfield(context, *heightfield, rcCfg.width, rcCfg.height, rcCfg.bmin, rcCfg.bmax, rcCfg.cs, rcCfg.ch)) {
            rcFreeHeightField(heightfield);
            delete context;
            return nullptr;
        }

        // Rasterize triangles
        const int ntris = indices.size() / 3;
        std::vector<unsigned char> areas(ntris, RC_WALKABLE_AREA);
        if (!rcRasterizeTriangles(context, &vertices[0].x, static_cast<int>(vertices.size()),
            reinterpret_cast<const int*>(indices.data()), areas.data(), ntris, *heightfield, rcCfg.walkableClimb)) {
            rcFreeHeightField(heightfield);
            delete context;
            return nullptr;
        }

        // Filter walkable surfaces
        rcFilterLowHangingWalkableObstacles(context, rcCfg.walkableClimb, *heightfield);
        rcFilterLedgeSpans(context, rcCfg.walkableHeight, rcCfg.walkableClimb, *heightfield);
        rcFilterWalkableLowHeightSpans(context, rcCfg.walkableHeight, *heightfield);

        // Build compact heightfield
        rcCompactHeightfield* compactHeightfield = rcAllocCompactHeightfield();
        if (!rcBuildCompactHeightfield(context, rcCfg.walkableHeight, rcCfg.walkableClimb, *heightfield, *compactHeightfield)) {
            rcFreeHeightField(heightfield);
            rcFreeCompactHeightfield(compactHeightfield);
            delete context;
            return nullptr;
        }
        rcFreeHeightField(heightfield);

        // Erode and partition
        if (!rcErodeWalkableArea(context, rcCfg.walkableRadius, *compactHeightfield) ||
            !rcBuildRegions(context, *compactHeightfield, rcCfg.borderSize, rcCfg.minRegionArea, rcCfg.mergeRegionArea)) {
            rcFreeCompactHeightfield(compactHeightfield);
            delete context;
            return nullptr;
        }

        // Extract contours and build poly mesh
        rcContourSet* contourSet = rcAllocContourSet();
        rcPolyMesh* polyMesh = rcAllocPolyMesh();
        rcPolyMeshDetail* detailMesh = rcAllocPolyMeshDetail();
        if (!rcBuildContours(context, *compactHeightfield, rcCfg.maxSimplificationError, rcCfg.maxEdgeLen, *contourSet) ||
            !rcBuildPolyMesh(context, *contourSet, rcCfg.maxVertsPerPoly, *polyMesh) ||
            !rcBuildPolyMeshDetail(context, *polyMesh, *compactHeightfield, rcCfg.detailSampleDist, rcCfg.detailSampleMaxError, *detailMesh)) {
            // Cleanup allocated resources
            rcFreeCompactHeightfield(compactHeightfield);
            rcFreeContourSet(contourSet);
            rcFreePolyMesh(polyMesh);
            rcFreePolyMeshDetail(detailMesh);
            delete context;
            return nullptr;
        }

        // Prepare Detour navmesh
        dtNavMeshCreateParams params;
        memset(&params, 0, sizeof(params));
        params.verts = polyMesh->verts;
        params.vertCount = polyMesh->nverts;
        params.polys = polyMesh->polys;
        params.polyAreas = polyMesh->areas;
        params.polyFlags = polyMesh->flags;
        params.polyCount = polyMesh->npolys;
        params.nvp = polyMesh->nvp;
        params.detailMeshes = detailMesh->meshes;
        params.detailVerts = detailMesh->verts;
        params.detailVertsCount = detailMesh->nverts;
        params.detailTris = detailMesh->tris;
        params.detailTriCount = detailMesh->ntris;
        params.walkableHeight = config.agentHeight;
        params.walkableRadius = config.agentRadius;
        params.walkableClimb = config.agentMaxClimb;
        rcVcopy(params.bmin, polyMesh->bmin);
        rcVcopy(params.bmax, polyMesh->bmax);
        params.cs = rcCfg.cs;
        params.ch = rcCfg.ch;
        params.buildBvTree = true;

        unsigned char* navData = nullptr;
        int navDataSize = 0;
        if (!dtCreateNavMeshData(&params, &navData, &navDataSize)) {
            // Cleanup
            rcFreeCompactHeightfield(compactHeightfield);
            rcFreeContourSet(contourSet);
            rcFreePolyMesh(polyMesh);
            rcFreePolyMeshDetail(detailMesh);
            delete context;
            return nullptr;
        }

        dtNavMesh* navMesh = dtAllocNavMesh();
        dtStatus status = navMesh->init(navData, navDataSize, DT_TILE_FREE_DATA);
        if (dtStatusFailed(status)) {
            dtFree(navData);
            dtFreeNavMesh(navMesh);
            navMesh = nullptr;
        }

        // Cleanup Recast resources
        rcFreeCompactHeightfield(compactHeightfield);
        rcFreeContourSet(contourSet);
        rcFreePolyMesh(polyMesh);
        rcFreePolyMeshDetail(detailMesh);
        delete context;

        return navMesh;
    }

};
