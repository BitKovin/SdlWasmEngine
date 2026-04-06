#include <vector>
#include <mutex>
#include "../glm.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <iostream>
#include "Navigation.hpp"

#include "../DebugDraw.hpp"

#include "../Level.hpp"

#include "../Physics.h"

#include "../Time.hpp"

#include "../LoadingScreen/LoadingScreenSystem.h"
#include "Detour/DetourCommon.h"
#include "NavigationGenerationHelpers.hpp"
#include "NavigationFileHelper.h"

// Recast and Detour includes

bool NavigationSystem::DebugDrawNavMeshEnabled = false;

// Static member initialization
std::recursive_mutex NavigationSystem::mainLock;
std::vector<dtObstacleRef> NavigationSystem::obstacles;



void NavigationSystem::DestroyNavData()
{


	std::lock_guard<std::recursive_mutex> lock(mainLock);

    DestroyAllObstacles();

    if (tileCache)
    {
        tileCache = nullptr;
    }
    if (tileCache)
    {
        dtFreeTileCache(tileCache);
        tileCache = nullptr;
    }


	if (navMesh)
		dtFreeNavMesh(navMesh);


    if (talloc)
    {
        delete talloc;
        talloc = nullptr;
    }
		

    if (tcomp)
    {
        delete tcomp;
        tcomp = nullptr;
    }


    if (tmproc)
    {
        delete tmproc;
        tmproc = nullptr;
    }


    if (g_crowd)
    {
        dtFreeCrowd(g_crowd);
        g_crowd = nullptr;
    }

    
    
}



void NavigationSystem::Update()
{
    if (tileCache == nullptr) return;

    std::lock_guard<std::recursive_mutex> lock(mainLock);

    bool upToDate = false;
    auto status = tileCache->update(Time::DeltaTimeF, navMesh, &upToDate);

    if (dtStatusFailed(status))
    {
        std::printf("Tile cache update failed. Status: %u\n", status);
    }
    else if (!upToDate)
    {
        //std::printf("Tile cache still processing updates.\n");
    }

    g_crowd->update(Time::DeltaTimeF, nullptr);

}

// Set areas -> flags when the cache creates Detour data
struct MeshProcess : public dtTileCacheMeshProcess {
    unsigned short walkFlag = 1; // SAMPLE_POLYFLAGS_WALK
    void process(dtNavMeshCreateParams* params, unsigned char* polyAreas, unsigned short* polyFlags) override {
        for (int i = 0; i < params->polyCount; ++i) {
            if (polyAreas[i] == RC_WALKABLE_AREA) polyAreas[i] = 1; // SAMPLE_POLYAREA_GROUND
            polyFlags[i] = walkFlag;
        }
    }
};


void NavigationSystem::GenerateNavData()
{
    DestroyNavData();

    std::string navmeshFilePath = Level::Current->filePath + ".nav";
    uint32_t mapVersion = FileSystemEngine::GetFileModificationTime(Level::Current->filePath);

    bool loaded = NavigationFileHelper::Load(navmeshFilePath.c_str(), navMesh, tileCache, mapVersion);

    if (loaded)
    {
        InitCrowd(512);
        return;
    }
    else
    {
        Logger::Log("rebuilding navmesh to file");
    }

    auto mesh = Level::Current->GetStaticNavObstaclesMesh();

    std::lock_guard<std::recursive_mutex> lock(mainLock);

    std::vector<glm::vec3> vertices = mesh.vertices;
    std::vector<uint32_t> indices = mesh.indices;
    if (vertices.size() < 3) return;

    glm::vec3 bmin = vertices[0], bmax = vertices[0];
    for (const auto& v : vertices) { bmin = glm::min(bmin, v); bmax = glm::max(bmax, v); }
    bmin -= glm::vec3(0.2); bmax += glm::vec3(0.2);


    // ---- Recast config (similar to yours)
    rcConfig cfg{};
    cfg.cs = 0.1f;
    cfg.ch = 0.2f;
    cfg.walkableSlopeAngle = 40.0f;
    cfg.walkableHeight = static_cast<int>(ceilf(2.0f / cfg.ch));
    cfg.walkableClimb = static_cast<int>(ceilf(0.6f / cfg.ch));
    cfg.walkableRadius = static_cast<int>(ceilf(0.5f / cfg.cs));
    cfg.maxEdgeLen = static_cast<int>(12 / cfg.cs);
    cfg.maxSimplificationError = 0.03f;
    cfg.minRegionArea = 0;
    cfg.mergeRegionArea = 200 * 200;
    cfg.maxVertsPerPoly = 6;
    cfg.tileSize = 64;
    cfg.borderSize = static_cast<int>(ceilf(0.5f / cfg.cs)) + 3;
    cfg.width = cfg.tileSize + cfg.borderSize * 2;
    cfg.height = cfg.tileSize + cfg.borderSize * 2;
    cfg.detailSampleDist = 0.1f;
    cfg.detailSampleMaxError = 0.1f;

    const float tileWorld = cfg.tileSize * cfg.cs;
    const int ntilesX = static_cast<int>(ceilf((bmax.x - bmin.x) / tileWorld));
    const int ntilesZ = static_cast<int>(ceilf((bmax.z - bmin.z) / tileWorld));
    const int expectedLayersPerTile = 4; // conservative upper bound; tune if you know your data
    const int maxTiles = ntilesX * ntilesZ * expectedLayersPerTile;
    const float borderWorld = cfg.borderSize * cfg.cs;

    // ---- Init Detour navmesh
    dtNavMeshParams navParams{};
    navParams.orig[0] = bmin.x; navParams.orig[1] = bmin.y; navParams.orig[2] = bmin.z;
    navParams.tileWidth = tileWorld;
    navParams.tileHeight = tileWorld;
    navParams.maxTiles = ntilesX * ntilesZ * expectedLayersPerTile; // Detour needs enough for all layers
    navParams.maxPolys = 1 << 14;

    navMesh = dtAllocNavMesh();
    if (!navMesh || dtStatusFailed(navMesh->init(&navParams))) {
        dtFreeNavMesh(navMesh); navMesh = nullptr;
        std::cerr << "Failed to initialize dtNavMesh\n";
        return;
    }

    // ---- Init Tile Cache
    talloc = new dtTileCacheAlloc();     // 4–16 MB temp buffer is typical
    tcomp = new FastLZCompressor();         // or your own compressor
    tmproc = new MeshProcess();

    dtTileCacheParams tcParams{};
    tcParams.orig[0] = bmin.x; tcParams.orig[1] = bmin.y; tcParams.orig[2] = bmin.z;
    tcParams.cs = cfg.cs; tcParams.ch = cfg.ch;
    tcParams.width = cfg.tileSize;          // in cells (not world units)
    tcParams.height = cfg.tileSize;          // in cells
    tcParams.walkableHeight = 2.0f;          // same as your dtCreateNavMeshData params
    tcParams.walkableRadius = 0.2f;
    tcParams.walkableClimb = 0.6f;
    tcParams.maxTiles = maxTiles;
    tcParams.maxObstacles = 2048;            // tune for your runtime needs

    tileCache = dtAllocTileCache();
    if (!tileCache || dtStatusFailed(tileCache->init(&tcParams, talloc, tcomp, tmproc))) {
        if (tileCache) dtFreeTileCache(tileCache); tileCache = nullptr;
        std::cerr << "Failed to initialize dtTileCache\n";
        return;
    }

    // ---- Flatten your geometry for Recast
    std::vector<float> vertFloats; vertFloats.reserve(vertices.size() * 3);
    for (auto& v : vertices) { vertFloats.push_back(v.x); vertFloats.push_back(v.y); vertFloats.push_back(v.z); }
    std::vector<int> triInts; triInts.reserve(indices.size());
    for (auto i : indices) triInts.push_back((int)i);

    const int nverts = (int)vertices.size();
    const int ntris = (int)indices.size() / 3;

    // ---- Precompute expanded tri bounds & tile bins (kept from your code)
    std::vector<std::array<float, 4>> triBounds(ntris);
    for (int i = 0; i < ntris; ++i) {
        const int* tri = &triInts[i * 3];
        const float* v0 = &vertFloats[tri[0] * 3];
        const float* v1 = &vertFloats[tri[1] * 3];
        const float* v2 = &vertFloats[tri[2] * 3];
        float minx = std::min(v0[0], std::min(v1[0], v2[0]));
        float minz = std::min(v0[2], std::min(v1[2], v2[2]));
        float maxx = std::max(v0[0], std::max(v1[0], v2[0]));
        float maxz = std::max(v0[2], std::max(v1[2], v2[2]));
        triBounds[i] = { minx - borderWorld, minz - borderWorld, maxx + borderWorld, maxz + borderWorld };
    }
    std::vector<std::vector<int>> tileTriangles(ntilesX * ntilesZ);
    for (int i = 0; i < ntris; ++i) {
        const float minx = triBounds[i][0], minz = triBounds[i][1];
        const float maxx = triBounds[i][2], maxz = triBounds[i][3];
        int tx0 = (int)((minx - bmin.x) / tileWorld);
        int tx1 = (int)((maxx - bmin.x) / tileWorld);
        int tz0 = (int)((minz - bmin.z) / tileWorld);
        int tz1 = (int)((maxz - bmin.z) / tileWorld);
        tx0 = std::max(0, tx0); tx1 = std::min(ntilesX - 1, tx1);
        tz0 = std::max(0, tz0); tz1 = std::min(ntilesZ - 1, tz1);
        for (int tz = tz0; tz <= tz1; ++tz)
            for (int tx = tx0; tx <= tx1; ++tx)
                tileTriangles[tz * ntilesX + tx].push_back(i);
    }

    rcContext* ctx = new rcContext();
    std::vector<unsigned char> triareas(ntris, 0);
    rcMarkWalkableTriangles(ctx, cfg.walkableSlopeAngle,
        vertFloats.data(), nverts, triInts.data(), ntris, triareas.data());

    // ---- Build cache tiles
    for (int tz = 0; tz < ntilesZ; ++tz) {
        float progress = (float)tz / (float)std::max(1, ntilesZ - 1);
        LoadingScreenSystem::Update(0.6f + progress * 0.3f);

        for (int tx = 0; tx < ntilesX; ++tx) {
            // Tight world bounds for this tile (no border)
            float tbmin[3] = { bmin.x + tx * tileWorld, bmin.y, bmin.z + tz * tileWorld };
            float tbmax[3] = { bmin.x + (tx + 1) * tileWorld, bmax.y, bmin.z + (tz + 1) * tileWorld };

            // Expanded bounds used for rasterization (add border)
            float ebmin[3] = { tbmin[0] - borderWorld, tbmin[1], tbmin[2] - borderWorld };
            float ebmax[3] = { tbmax[0] + borderWorld, tbmax[1], tbmax[2] + borderWorld };

            // Build heightfield for this tile
            rcHeightfield* solid = rcAllocHeightfield();
            if (!solid || !rcCreateHeightfield(ctx, *solid, cfg.width, cfg.height, ebmin, ebmax, cfg.cs, cfg.ch)) {
                if (solid) rcFreeHeightField(solid);
                continue;
            }

            const auto& triList = tileTriangles[tz * ntilesX + tx];
            if (triList.empty()) { rcFreeHeightField(solid); continue; }

            std::vector<int>    ttris;      ttris.reserve(triList.size() * 3);
            std::vector<unsigned char> ta;  ta.reserve(triList.size());
            for (int i : triList) {
                ttris.push_back(triInts[i * 3 + 0]);
                ttris.push_back(triInts[i * 3 + 1]);
                ttris.push_back(triInts[i * 3 + 2]);
                ta.push_back(triareas[i]);
            }

            if (!rcRasterizeTriangles(ctx, vertFloats.data(), nverts,
                ttris.data(), ta.data(), (int)triList.size(),
                *solid, cfg.walkableClimb)) {
                rcFreeHeightField(solid);
                continue;
            }

            rcFilterLowHangingWalkableObstacles(ctx, cfg.walkableClimb, *solid);
            rcFilterLedgeSpans(ctx, cfg.walkableHeight, cfg.walkableClimb, *solid);
            rcFilterWalkableLowHeightSpans(ctx, cfg.walkableHeight, *solid);

            rcCompactHeightfield* chf = rcAllocCompactHeightfield();
            if (!chf || !rcBuildCompactHeightfield(ctx, cfg.walkableHeight, cfg.walkableClimb, *solid, *chf)) {
                if (chf) rcFreeCompactHeightfield(chf);
                rcFreeHeightField(solid);
                continue;
            }
            rcFreeHeightField(solid);

            if (!rcErodeWalkableArea(ctx, cfg.walkableRadius, *chf)) {
                rcFreeCompactHeightfield(chf);
                continue;
            }

            // Build heightfield *layers* for tile cache
            rcHeightfieldLayerSet* lset = rcAllocHeightfieldLayerSet();
            if (!lset || !rcBuildHeightfieldLayers(ctx, *chf, cfg.borderSize, cfg.walkableHeight, *lset)) {
                if (lset) rcFreeHeightfieldLayerSet(lset);
                rcFreeCompactHeightfield(chf);
                continue;
            }
            rcFreeCompactHeightfield(chf);

            // For each layer, build compressed cache data and add to tile cache
            for (int li = 0; li < lset->nlayers; ++li) {
                const rcHeightfieldLayer* layer = &lset->layers[li];
                dtTileCacheLayerHeader header{};
                header.magic = DT_TILECACHE_MAGIC;
                header.version = DT_TILECACHE_VERSION;
                header.tx = tx; header.ty = tz; header.tlayer = (unsigned char)li;
                // Copy layer spatial info (cells)
                rcVcopy(header.bmin, layer->bmin);
                rcVcopy(header.bmax, layer->bmax);
                header.width = (unsigned char)layer->width;
                header.height = (unsigned char)layer->height;
                header.minx = (unsigned char)layer->minx;
                header.maxx = (unsigned char)layer->maxx;
                header.miny = (unsigned char)layer->miny;
                header.maxy = (unsigned char)layer->maxy;
                header.hmin = (unsigned short)layer->hmin;
                header.hmax = (unsigned short)layer->hmax;

                unsigned char* compressed = nullptr;
                int compressedSize = 0;
                if (dtStatusFailed(dtBuildTileCacheLayer(tcomp, &header,
                    layer->heights, layer->areas, layer->cons,
                    &compressed, &compressedSize))) {
                    continue;
                }

                dtCompressedTileRef cref = 0;
                // DT_COMPRESSEDTILE_FREE_DATA: cache will own & free the memory. :contentReference[oaicite:2]{index=2}
                if (dtStatusFailed(tileCache->addTile(compressed, compressedSize, DT_COMPRESSEDTILE_FREE_DATA, &cref))) {
                    // If add fails, free ourselves:
                    dtFree(compressed);
                }
            }

            rcFreeHeightfieldLayerSet(lset);

            // Build all detour tiles at this grid coordinate from the cache
            (void)tileCache->buildNavMeshTilesAt(tx, tz, navMesh); // builds one or more layers. :contentReference[oaicite:3]{index=3}
        }
    }

    delete ctx;

    NavigationFileHelper::Save(navmeshFilePath.c_str(), navMesh, tileCache, mapVersion);

    InitCrowd(512);



}

void NavigationSystem::InitCrowd(int maxAgents, float maxAgentRadius)
{
    g_crowd = dtAllocCrowd();
    if (!g_crowd) { std::cerr << "Failed allocate crowd\n"; return; }
    if (!g_crowd->init(maxAgents, maxAgentRadius, navMesh))
    {
        std::cerr << "Failed to initialize crowd\n";
        dtFreeCrowd(g_crowd);
        g_crowd = nullptr;
        return;
    }
    // Set up default filter for queryFilterType 0
    dtQueryFilter* filter = g_crowd->getEditableFilter(0);
    if (filter)
    {
        filter->setIncludeFlags(0xFFFF); // Include all poly flags
        filter->setExcludeFlags(0);      // Exclude none
        filter->setAreaCost(0, 1.0f);    // Default cost for default area
        // Add costs for other area types if your navmesh uses them
    }
    // Existing obstacle avoidance params
    dtObstacleAvoidanceParams p;
    p.velBias = 0.4f;
    p.adaptiveDivs = 7;
    p.adaptiveRings = 2;
    p.adaptiveDepth = 5;
    p.weightDesVel = 2.0f;
    p.weightCurVel = 0.75f;
    p.weightSide = 0.75f;
    p.weightToi = 2.5f;
    p.horizTime = 2.2f;
    p.gridSize = 33;
    g_crowd->setObstacleAvoidanceParams(0, &p);
}


void NavigationSystem::DrawNavmesh()
{
    if (DebugDrawNavMeshEnabled == false) return;

    if (!navMesh)
        return;

    std::lock_guard<std::recursive_mutex> lock(mainLock);

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

                if (distance(Camera::position, p1) > 100)
                    continue;

                DebugDraw::Line(p0, p1, Time::DeltaTimeF / 2, 0.01); // longer duration for better visibility
            }
        }
    }
}

void NavigationSystem::RemoveObstacle(dtObstacleRef obstacleRef)
{
    if (!tileCache || obstacleRef == 0)
        return;

    std::lock_guard<std::recursive_mutex> lock(mainLock);



    // Remove the obstacle using its reference
    const dtStatus status = tileCache->removeObstacle(obstacleRef);
    if (dtStatusFailed(status))
    {
        std::printf("Failed to remove obstacle with ref: %u. Status: %u\n", obstacleRef, status);
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

dtObstacleRef NavigationSystem::CreateObstacleBox(const glm::vec3& min, const glm::vec3& max)
{
    if (!tileCache) // Ensure tileCache is valid
        return 0;

    glm::vec3 adjustedMin = min;
    glm::vec3 adjustedMax = max;

    // Ensure each axis (x, y, z) has a minimum length of 2.0 units
    for (int i = 0; i < 3; ++i)
    {
        const float currentLength = adjustedMax[i] - adjustedMin[i];
        if (currentLength < 0.2f)
        {
            const float delta = (0.2f - currentLength) * 0.5f;
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
        std::printf("Failed to add box obstacle. Status: %u\n", status);
        return 0;
    }

    obstacles.push_back(obstacleRef);

    //std::printf("Box obstacle added. Ref: %u\n", obstacleRef);
    return obstacleRef;
}

bool NavigationSystem::HasLineOfSight(const vec3& pointA, const vec3& pointB)
{
    return Physics::SphereTrace(pointA, pointB, 0.5, BodyType::World).hasHit == false;
}

// Custom filter to check polygon area instead of flags


// =====================================================================
    // FindSimplePath
    // Computes a simple path from a start to a target position. It returns the
    // computed path as a vector of 3D points (world coordinates). If no valid
    // path is found, an empty vector is returned.
    //
    // After computing the initial straight path, the function checks if the
    // second and second-from-last points are redundant. If there is a clear line
    // of sight skipping those points, then they are removed.
    // =====================================================================
static bool IsPointReallyOnPoly(dtNavMeshQuery* navQuery,
    dtPolyRef        polyRef,
    const float      pos[3],
    float            horizTolSq = 0.0025f)
{
    if (!polyRef) return false;
    float  closest[3];
    bool inside = 0;
    if (dtStatusFailed(navQuery->closestPointOnPoly(polyRef, pos, closest, &inside)))
        return false;
    if (!inside) return false;
    const float dx = pos[0] - closest[0];
    const float dz = pos[2] - closest[2];
    return (dx * dx + dz * dz) <= horizTolSq;
}

std::vector<glm::vec3> NavigationSystem::FindSimplePath(glm::vec3 start, glm::vec3 target, float acceptanceRadius, bool* outReached, bool allowPartialPath)
{
    if (outReached) *outReached = false;

    std::vector<glm::vec3> outPath;
    if (!navMesh) return outPath;
    std::lock_guard<std::recursive_mutex> _lock(mainLock);

    // Allocate & init query
    dtNavMeshQuery* navQuery = dtAllocNavMeshQuery();
    if (!navQuery) return outPath;
    if (dtStatusFailed(navQuery->init(navMesh, 2048)))
    {
        dtFreeNavMeshQuery(navQuery);
        return outPath;
    }

    auto hit = Physics::LineTrace(start, start - vec3(0, 5, 0), BodyType::World);
    if (hit.hasHit)
        start = hit.position + vec3(0, 0.3f, 0);

	vec3 targetInitial = target;

    hit = Physics::SphereTrace(target, target - vec3(0, 5, 0),0.4, BodyType::World);
    if (hit.hasHit)
        target = hit.position + vec3(0, 0.3f, 0);

    CustomFilter filter;
    filter.setIncludeFlags(0xffff);
    filter.setExcludeFlags(0);

    // --- 1) FindNearestPoly  (increment extents up to limit) ---
    const std::vector<glm::vec3> EXTENTS = {
        {0.70f, 1.5f, 0.70f},  // tight
        {0.60f, 1.5f, 0.60f},  // med
        {1.00f, 1.5f, 1.00f},  // wide
        {1.50f, 1.5f, 1.50f},  // wide
    };
    dtPolyRef sRef = 0, gRef = 0;
    float     sNearest[3], gNearest[3];
    float     sPos[3] = { start.x,  start.y,  start.z };
    float     gPos[3] = { target.x, target.y, target.z };

    auto findOnPoly = [&](const float pos[3], dtPolyRef& outRef, float outNearest[3]) {
        for (auto& e : EXTENTS)
        {
            float ext[3] = { e.x, e.y, e.z };
            if (dtStatusSucceed(navQuery->findNearestPoly(pos, ext, &filter, &outRef, outNearest)) && outRef > 0)
            {
                // Optional: Add a distance check for safety
                if (dtVdist(pos, outNearest) <= e.x)
                    return true;
            }
        }
        return false;
        };

    if (!findOnPoly(gPos, gRef, gNearest))
    {
        gPos[0] = targetInitial.x;
		gPos[1] = targetInitial.y;
        gPos[2] = targetInitial.z;
        if (!findOnPoly(gPos, gRef, gNearest))
        {
            dtFreeNavMeshQuery(navQuery);
            return outPath; // failed to localize goal
		}
    }

    // attempt to pick start & goal polys
    if (!findOnPoly(sPos, sRef, sNearest))
    {
        dtFreeNavMeshQuery(navQuery);
        return outPath; // failed to localize start or goal
    }

    // Snap positions to nearest on-mesh points
    dtVcopy(sPos, sNearest);
    dtVcopy(gPos, gNearest);

    // Check if projected positions are close enough
    float dist = dtVdist(sNearest, gNearest);
    if (dist <= acceptanceRadius)
    {
        if (outReached) *outReached = true;
        dtFreeNavMeshQuery(navQuery);
        return {  };
    }

    // --- 2) FindPath (A*) across linked polys ---
    const int MAX_POLYS = 512;
    dtPolyRef polyPath[MAX_POLYS];
    int       polyCount = 0;
    if (dtStatusFailed(navQuery->findPath(sRef, gRef, sPos, gPos,
        &filter,
        polyPath, &polyCount, MAX_POLYS)) ||
        polyCount == 0)
    {
        dtFreeNavMeshQuery(navQuery);
        return outPath;
    }

    // --- 3) Clamp goal if partial ---
    bool reached = (polyPath[polyCount - 1] == gRef);
    if (!reached)
    {
        navQuery->closestPointOnPoly(polyPath[polyCount - 1], gPos, gNearest, nullptr);

        float partialDist = dtVdist(sNearest, gNearest);  // use sNearest, not sPos
        if (partialDist <= acceptanceRadius)
        {
            if (outReached) *outReached = true;
            dtFreeNavMeshQuery(navQuery);
            return {};
        }
    }

    // --- 4) StringPull / StraightPath ---
    const int MAX_STRAIGHT = 512;
    float          straight[3 * MAX_STRAIGHT];
    unsigned char  flags[MAX_STRAIGHT];
    dtPolyRef      strPolys[MAX_STRAIGHT];
    int            strCount = 0;

    if (dtStatusFailed(navQuery->findStraightPath(
        sNearest,
        reached ? gPos : gNearest,
        polyPath, polyCount,
        straight, flags, strPolys,
        &strCount, MAX_STRAIGHT, DT_STRAIGHTPATH_ALL_CROSSINGS)))
    {
        if (allowPartialPath)
        {
            if (dtVdist(sNearest, gNearest) <= acceptanceRadius)
            {
                if (outReached) *outReached = true;
                dtFreeNavMeshQuery(navQuery);
                return {};
            }

            dtFreeNavMeshQuery(navQuery);
            return { glm::vec3(gNearest[0], gNearest[1], gNearest[2]) };
        }
    }

    // --- 5) Build glm path (skip the first point, it's the start) ---
    outPath.reserve(strCount);
    for (int i = 1; i < strCount; ++i)
    {
        outPath.emplace_back(
            straight[i * 3 + 0],
            straight[i * 3 + 1],
            straight[i * 3 + 2]
        );
    }
    // final target or clamped point
    outPath.emplace_back(
        reached ? target : glm::vec3(gNearest[0], gNearest[1], gNearest[2])
    );

    dtFreeNavMeshQuery(navQuery);

    //   // --- 6) Collision sanity check ---
    //   if (!CollisionCheckPath(start, outPath))
    //   {
    //   	outPath.clear();
    //   }
    //       

    return outPath;
}
// Returns a path (list of waypoints) that attempts to make the NPC run AWAY from player.
// npcPos: NPC world position. playerPos: Player world position.
// maxSearchRadius: how far we search for flee points (tuneable).
// npcSpeed / playerSpeed: used to convert distance -> time for scoring; tune to your agents.
// Prefer open spaces: uses dtNavMeshQuery::findDistanceToWall + findPolysAroundCircle.
// Assumes HasLineOfSight(vec3 A, vec3 B) exists and that navMesh/isPoint checks exist
std::vector<glm::vec3> NavigationSystem::FindFleePath(
    const glm::vec3& start,
    const glm::vec3& threatPos,
    float desiredDist /*= 12.0f*/,
    int maxCandidates /*= 8*/,
    float npcSpeed /*= 4.0f*/,
    float playerSpeed /*= 5.0f*/)
{
    std::vector<glm::vec3> outPath;
    if (!navMesh) return outPath;
    std::lock_guard<std::recursive_mutex> _lock(mainLock);

    // --- navQuery ---
    dtNavMeshQuery* navQuery = dtAllocNavMeshQuery();
    if (!navQuery) return outPath;
    if (dtStatusFailed(navQuery->init(navMesh, 2048)))
    {
        dtFreeNavMeshQuery(navQuery);
        return outPath;
    }

    // --- snap to ground like FindSimplePath ---
    auto hit = Physics::LineTrace(start, start - glm::vec3(0, 300, 0), BodyType::World);
    glm::vec3 startFixed = hit.hasHit ? hit.position + glm::vec3(0, 1, 0) : start;
    hit = Physics::LineTrace(threatPos, threatPos - glm::vec3(0, 300, 0), BodyType::World);
    glm::vec3 threatFixed = hit.hasHit ? hit.position + glm::vec3(0, 1, 0) : threatPos;

    CustomFilter filter;
    filter.setIncludeFlags(0xffff);
    filter.setExcludeFlags(0);

    // --- helper: findNearestPoly with progressive extents (same as your code) ---
    const glm::vec3 EXTENTS_LIST[] = {
        {0.50f, 1.5f, 0.50f},
        {0.60f, 1.5f, 0.60f},
        {1.00f, 1.5f, 1.00f},
        {1.50f, 1.5f, 1.50f},
    };
    auto findOnPoly = [&](const float pos[3], dtPolyRef& outRef, float outNearest[3]) -> bool {
        for (auto& e : EXTENTS_LIST)
        {
            float ext[3] = { e.x, e.y, e.z };
            if (dtStatusSucceed(navQuery->findNearestPoly(pos, ext, &filter, &outRef, outNearest)) && outRef)
            {
                if (IsPointReallyOnPoly(navQuery, outRef, pos))
                    return true;
            }
        }
        return false;
        };

    // --- localize start & threat ---
    dtPolyRef startRef = 0, threatRef = 0;
    float startNearest[3], threatNearest[3];
    float startF[3] = { startFixed.x, startFixed.y, startFixed.z };
    float threatF[3] = { threatFixed.x, threatFixed.y, threatFixed.z };

    if (!findOnPoly(startF, startRef, startNearest))
    {
        dtFreeNavMeshQuery(navQuery);
        return outPath;
    }
    findOnPoly(threatF, threatRef, threatNearest); // may fail; we handle below

    const int MAX_POLYS = 512;
    const int MAX_STRAIGHT = 512;

    // --- compute baseline: player -> current NPC (navmesh path length) ---
    float playerToStartLen = 0.0f;
    if (threatRef != 0)
    {
        dtPolyRef ppath[MAX_POLYS];
        int pcount = 0;
        if (dtStatusSucceed(navQuery->findPath(threatRef, startRef, threatNearest, startNearest, &filter, ppath, &pcount, MAX_POLYS)) && pcount > 0)
        {
            float straightP[3 * MAX_STRAIGHT];
            unsigned char flagsP[MAX_STRAIGHT];
            dtPolyRef spPolysP[MAX_STRAIGHT];
            int spCount = 0;
            if (dtStatusSucceed(navQuery->findStraightPath(threatNearest, startNearest, ppath, pcount, straightP, flagsP, spPolysP, &spCount, MAX_STRAIGHT, DT_STRAIGHTPATH_ALL_CROSSINGS)) && spCount > 0)
            {
                for (int s = 1; s < spCount; ++s)
                {
                    float dx = straightP[s * 3 + 0] - straightP[(s - 1) * 3 + 0];
                    float dy = straightP[s * 3 + 1] - straightP[(s - 1) * 3 + 1];
                    float dz = straightP[s * 3 + 2] - straightP[(s - 1) * 3 + 2];
                    playerToStartLen += sqrtf(dx * dx + dy * dy + dz * dz);
                }
            }
            else
            {
                // fallback euclidean
                playerToStartLen = glm::distance(threatFixed, startFixed);
            }
        }
        else
        {
            // fallback euclidean
            playerToStartLen = glm::distance(threatFixed, startFixed);
        }
    }
    else
    {
        // threat not localized on navmesh: use Euclidean distance as baseline
        playerToStartLen = glm::distance(threatFixed, startFixed);
    }

    // --- sampling: forward-biased full-circle (densities tuned) ---
    glm::vec3 fleeDir = glm::vec3(startFixed.x - threatFixed.x, 0.0f, startFixed.z - threatFixed.z);
    if (glm::length2(fleeDir) < 1e-6f)
    {
        float a = (float)(rand() % 314) / 100.0f;
        fleeDir = glm::vec3(std::cos(a), 0.0f, std::sin(a));
    }
    fleeDir = glm::normalize(fleeDir);
    float baseAng = atan2f(fleeDir.z, fleeDir.x);

    struct AngleSample { float ang; float weight; };
    std::vector<AngleSample> angleSamples;
    // front dense
    const int FRONT_SAMPLES = 12;
    const float FRONT_HALF = glm::radians(70.0f);
    for (int i = 0; i < FRONT_SAMPLES; ++i)
    {
        float t = FRONT_SAMPLES == 1 ? 0.0f : (float)i / float(FRONT_SAMPLES - 1);
        float a = baseAng - FRONT_HALF + t * (2.0f * FRONT_HALF);
        angleSamples.push_back({ a, 1.0f });
    }
    // sides
    const int SIDE_SAMPLES = 8;
    const float SIDE_MIN = glm::radians(75.0f), SIDE_MAX = glm::radians(135.0f);
    for (int i = 0; i < SIDE_SAMPLES / 2; ++i)
    {
        float t = (float)i / float(SIDE_SAMPLES / 2 - 1);
        float a = baseAng + SIDE_MIN + t * (SIDE_MAX - SIDE_MIN);
        angleSamples.push_back({ a, 0.6f });
    }
    for (int i = 0; i < SIDE_SAMPLES / 2; ++i)
    {
        float t = (float)i / float(SIDE_SAMPLES / 2 - 1);
        float a = baseAng - SIDE_MIN - t * (SIDE_MAX - SIDE_MIN);
        angleSamples.push_back({ a, 0.6f });
    }
    // back sparse
    const int BACK_SAMPLES = 4;
    const float BACK_HALF = glm::radians(25.0f);
    for (int i = 0; i < BACK_SAMPLES; ++i)
    {
        float t = BACK_SAMPLES == 1 ? 0.0f : (float)i / float(BACK_SAMPLES - 1);
        float a = baseAng + glm::pi<float>() - BACK_HALF + t * (2.0f * BACK_HALF);
        angleSamples.push_back({ a, 0.35f });
    }
    // global sparse coverage
    const int GLOBAL_SAMPLES = 6;
    for (int i = 0; i < GLOBAL_SAMPLES; ++i)
    {
        float a = (glm::two_pi<float>() / GLOBAL_SAMPLES) * i;
        angleSamples.push_back({ a, 0.25f });
    }

    std::vector<float> radii = { desiredDist * 0.6f, desiredDist, desiredDist * 1.5f, desiredDist * 2.0f };
    struct Candidate {
        glm::vec3 pos;
        float posF[3];
        dtPolyRef ref;
        float npcPathLen;
        float angleWeight;
        glm::vec3 dirFromThreat;
    };
    std::vector<Candidate> candidates;
    candidates.reserve(128);

    // small extent for findNearestPoly
    float extSample[3] = { 0.6f, 1.5f, 0.6f };

    auto dirFromAngle = [](float a)->glm::vec3 { return glm::vec3(cosf(a), 0.0f, sinf(a)); };

    // collect candidates: map samples to navmesh and compute NPC path-length
    for (const auto& as : angleSamples)
    {
        glm::vec3 d = dirFromAngle(as.ang);
        for (float r : radii)
        {
            if ((int)candidates.size() >= 128) break;
            glm::vec3 sampleWorld = glm::vec3(startNearest[0], startNearest[1], startNearest[2]) + d * r;
            float sampleF[3] = { sampleWorld.x, sampleWorld.y, sampleWorld.z };

            dtPolyRef candRef = 0;
            float candNearest[3];
            if (!dtStatusSucceed(navQuery->findNearestPoly(sampleF, extSample, &filter, &candRef, candNearest)) || candRef == 0)
                continue;

            // quick reachability: start -> cand
            dtPolyRef polyPath[MAX_POLYS];
            int polyCount = 0;
            if (!dtStatusSucceed(navQuery->findPath(startRef, candRef, startNearest, candNearest, &filter, polyPath, &polyCount, MAX_POLYS)))
                continue;
            if (polyCount == 0 && startRef != candRef) continue;

            // compute string-pulled length
            float straight[MAX_STRAIGHT * 3];
            unsigned char flags[MAX_STRAIGHT];
            dtPolyRef spPolys[MAX_STRAIGHT];
            int spCount = 0;
            float npcLen = 0.0f;
            if (dtStatusSucceed(navQuery->findStraightPath(startNearest, candNearest, polyPath, polyCount, straight, flags, spPolys, &spCount, MAX_STRAIGHT, DT_STRAIGHTPATH_ALL_CROSSINGS)) && spCount > 0)
            {
                for (int s = 1; s < spCount; ++s)
                {
                    float dx = straight[s * 3 + 0] - straight[(s - 1) * 3 + 0];
                    float dy = straight[s * 3 + 1] - straight[(s - 1) * 3 + 1];
                    float dz = straight[s * 3 + 2] - straight[(s - 1) * 3 + 2];
                    npcLen += sqrtf(dx * dx + dy * dy + dz * dz);
                }
            }
            else
            {
                float dx = candNearest[0] - startNearest[0];
                float dz = candNearest[2] - startNearest[2];
                npcLen = sqrtf(dx * dx + dz * dz);
            }

            Candidate c;
            c.pos = glm::vec3(candNearest[0], candNearest[1], candNearest[2]);
            c.posF[0] = candNearest[0]; c.posF[1] = candNearest[1]; c.posF[2] = candNearest[2];
            c.ref = candRef;
            c.npcPathLen = npcLen;
            c.angleWeight = as.weight;
            c.dirFromThreat = glm::normalize(glm::vec3(c.pos.x - threatFixed.x, 0.0f, c.pos.z - threatFixed.z));
            candidates.push_back(c);
        }
        if ((int)candidates.size() >= 128) break;
    }

    if (candidates.empty())
    {
        dtFreeNavMeshQuery(navQuery);
        return FindSimplePath(startFixed, startFixed + fleeDir * 3.0f);
    }

    // prefer forward candidates but keep coverage
    std::sort(candidates.begin(), candidates.end(), [](const Candidate& a, const Candidate& b) {
        return (a.angleWeight * a.npcPathLen) > (b.angleWeight * b.npcPathLen);
        });

    int evaluateTop = std::min<int>(16, (int)candidates.size());
    float bestScore = -1e9f;
    int bestIndex = -1;

    // scoring tuning constants (tweak to your game)
    const float AREA_RADIUS = 5.5f;
    const int MAX_AROUND = 256;
    const float WALL_CHECK_RADIUS = 6.0f;
    const float MIN_FREE_DIST = 1.6f;
    const float MIN_AREA_POLYS = 6.0f;

    for (int i = 0; i < evaluateTop; ++i)
    {
        Candidate& cand = candidates[i];

        // --- 1) player -> candidate path length (navmesh if possible; unreachable => huge) ---
        float playerToCandLen = 0.0f;
        if (threatRef == 0)
        {
            // threat not on mesh: fallback to Euclidean
            playerToCandLen = glm::distance(threatFixed, cand.pos);
        }
        else
        {
            dtPolyRef ppath[MAX_POLYS];
            int pcount = 0;
            if (!dtStatusSucceed(navQuery->findPath(threatRef, cand.ref, threatNearest, cand.posF, &filter, ppath, &pcount, MAX_POLYS)) || pcount == 0)
            {
                // unreachable from player -> great (assign huge)
                playerToCandLen = 1e6f;
            }
            else
            {
                float straightP[3 * MAX_STRAIGHT];
                unsigned char flagsP[MAX_STRAIGHT];
                dtPolyRef spPolysP[MAX_STRAIGHT];
                int spCount = 0;
                if (dtStatusSucceed(navQuery->findStraightPath(threatNearest, cand.posF, ppath, pcount, straightP, flagsP, spPolysP, &spCount, MAX_STRAIGHT, DT_STRAIGHTPATH_ALL_CROSSINGS)) && spCount > 0)
                {
                    for (int s = 1; s < spCount; ++s)
                    {
                        float dx = straightP[s * 3 + 0] - straightP[(s - 1) * 3 + 0];
                        float dy = straightP[s * 3 + 1] - straightP[(s - 1) * 3 + 1];
                        float dz = straightP[s * 3 + 2] - straightP[(s - 1) * 3 + 2];
                        playerToCandLen += sqrtf(dx * dx + dy * dy + dz * dz);
                    }
                }
                else
                {
                    // fallback euclidean
                    playerToCandLen = glm::distance(threatFixed, cand.pos);
                }
            }
        }

        // --- 2) compute intercept margin (time-based) ---
        float npcTime = cand.npcPathLen / std::max(0.0001f, npcSpeed);
        float playerTime = playerToCandLen / std::max(0.0001f, playerSpeed);
        float interceptMargin = playerTime - npcTime;

        // --- 3) compute distance-gain-per-meter (the requested priority) ---
        // playerToStartLen computed earlier, use candidate unreachable as huge gain
        float deltaDist = playerToCandLen - playerToStartLen; // positive -> candidate farther from player than current
        // normalized by npcPathLen so we prefer candidates that give more gain per meter of travel
        float gainPerMeter = deltaDist / std::max(0.01f, cand.npcPathLen);

        // if playerToCandLen is huge (unreachable), favor strongly
        if (playerToCandLen > 1e5f)
            gainPerMeter = 1e3f;

        // --- 4) local area (polys) ---
        dtPolyRef aroundRef[MAX_AROUND];
        dtPolyRef aroundParent[MAX_AROUND];
        float aroundCost[MAX_AROUND];
        int aroundCount = 0;
        if (!dtStatusSucceed(navQuery->findPolysAroundCircle(cand.ref, cand.posF, AREA_RADIUS, &filter, aroundRef, aroundParent, aroundCost, &aroundCount, MAX_AROUND)))
            aroundCount = 0;

        // --- 5) distance-to-wall sampling along path (minFreeDist) ---
        // compute straight path again and sample a few points (start/mid/end)
        float minFreeDist = WALL_CHECK_RADIUS;
        {
            dtPolyRef ppoly[MAX_POLYS];
            int pcount = 0;
            if (dtStatusSucceed(navQuery->findPath(startRef, cand.ref, startNearest, cand.posF, &filter, ppoly, &pcount, MAX_POLYS)) && pcount > 0)
            {
                float straightPath[3 * MAX_STRAIGHT];
                unsigned char flagsSP[MAX_STRAIGHT];
                dtPolyRef spPolysSP[MAX_STRAIGHT];
                int spCount = 0;
                if (dtStatusSucceed(navQuery->findStraightPath(startNearest, cand.posF, ppoly, pcount, straightPath, flagsSP, spPolysSP, &spCount, MAX_STRAIGHT, DT_STRAIGHTPATH_ALL_CROSSINGS)) && spCount > 0)
                {
                    // sample up to three points: start, mid, end
                    std::vector<int> sampleIdx;
                    sampleIdx.push_back(0);
                    sampleIdx.push_back(spCount / 2);
                    sampleIdx.push_back(spCount - 1);
                    for (int si : sampleIdx)
                    {
                        float sx = straightPath[si * 3 + 0];
                        float sy = straightPath[si * 3 + 1];
                        float sz = straightPath[si * 3 + 2];
                        float samplePos[3] = { sx, sy, sz };

                        // localize sample point (robustness)
                        dtPolyRef sampleRef = 0;
                        float sampleNearest[3];
                        if (!findOnPoly(samplePos, sampleRef, sampleNearest))
                        {
                            sampleRef = cand.ref;
                            sampleNearest[0] = cand.posF[0]; sampleNearest[1] = cand.posF[1]; sampleNearest[2] = cand.posF[2];
                        }

                        float hitDist = WALL_CHECK_RADIUS;
                        float hitPos[3]; float hitNormal[3];
                        if (dtStatusSucceed(navQuery->findDistanceToWall(sampleRef, sampleNearest, WALL_CHECK_RADIUS, &filter, &hitDist, hitPos, hitNormal)))
                        {
                            hitDist = std::max(0.0f, std::min(hitDist, WALL_CHECK_RADIUS));
                        }
                        else
                        {
                            hitDist = WALL_CHECK_RADIUS;
                        }
                        minFreeDist = std::min(minFreeDist, hitDist);
                    }
                }
            }
        }

        // --- 6) LoS check with eye height (1.6m) ---
        glm::vec3 threatEye = threatFixed + glm::vec3(0, 1.6f, 0);
        glm::vec3 candEye = cand.pos + glm::vec3(0, 1.6f, 0);
        bool visible = true;// HasLineOfSight(threatEye, candEye);

        // --- 7) final score: include gainPerMeter with heavy weight (user request) ---
        // weights: tune these if needed
        const float W_GAIN = 4.0f;            // weight for gain-per-meter (primary fix)
        const float W_INTERCEPT = 1.5f;       // still prefer positive intercept margins
        const float W_MINFREE = 2.2f;         // openness along the path
        const float W_AREA = 0.14f;           // local area
        const float W_ANGLE = 0.9f;           // forward bias multiplier (0..1)
        const float VISIBLE_PENALTY = -3.5f;
        const float HIDDEN_BONUS = 2.0f;
        const float PATHTIME_PENALTY = -0.06f;

        float score = 0.0f;
        score += W_GAIN * gainPerMeter;
        score += W_INTERCEPT * interceptMargin;
        score += W_MINFREE * minFreeDist;
        score += W_AREA * (float)aroundCount;
        score += W_ANGLE * cand.angleWeight;
        score += (visible ? VISIBLE_PENALTY : HIDDEN_BONUS);
        score += PATHTIME_PENALTY * npcTime;

        // strong penalties for truly bad candidates
        if (minFreeDist < MIN_FREE_DIST) score -= 4.0f;           // too narrow anywhere
        if (aroundCount < (int)MIN_AREA_POLYS) score -= 2.5f;     // tiny pocket
        if (gainPerMeter < -0.05f) score -= 3.0f;                 // moving closer per meter -> strongly bad
        if (interceptMargin < -2.0f) score -= 5.0f;              // player reaches much faster -> avoid

        // tiny jitter
        score += ((rand() % 100) / 100.0f) * 0.02f;

        if (score > bestScore)
        {
            bestScore = score;
            bestIndex = i;
        }
    }

    // lateral sidestep fallback (prefer safe lateral moves)
    if (bestIndex < 0 || bestScore < 0.6f)
    {
        glm::vec3 up(0, 1, 0);
        glm::vec3 perp = glm::normalize(glm::cross(up, fleeDir));
        std::vector<glm::vec3> latTargets = {
            glm::vec3(startNearest[0], startNearest[1], startNearest[2]) + perp * 2.5f,
            glm::vec3(startNearest[0], startNearest[1], startNearest[2]) - perp * 2.5f,
            glm::vec3(startNearest[0], startNearest[1], startNearest[2]) + perp * 4.0f,
            glm::vec3(startNearest[0], startNearest[1], startNearest[2]) - perp * 4.0f
        };
        for (auto& lt : latTargets)
        {
            float ltF[3] = { lt.x, lt.y, lt.z };
            dtPolyRef ltRef = 0;
            float ltNearest[3];
            if (!dtStatusSucceed(navQuery->findNearestPoly(ltF, extSample, &filter, &ltRef, ltNearest)) || ltRef == 0)
                continue;

            dtPolyRef polyPath[MAX_POLYS];
            int polyCount = 0;
            if (!dtStatusSucceed(navQuery->findPath(startRef, ltRef, startNearest, ltNearest, &filter, polyPath, &polyCount, MAX_POLYS)))
                continue;
            if (polyCount == 0 && startRef != ltRef) continue;

            // straight path length
            float straight[MAX_STRAIGHT * 3];
            unsigned char flags[MAX_STRAIGHT];
            dtPolyRef spPolys[MAX_STRAIGHT];
            int spCount = 0;
            float ltLen = 0.0f;
            if (dtStatusSucceed(navQuery->findStraightPath(startNearest, ltNearest, polyPath, polyCount, straight, flags, spPolys, &spCount, MAX_STRAIGHT, DT_STRAIGHTPATH_ALL_CROSSINGS)) && spCount > 0)
            {
                for (int s = 1; s < spCount; ++s)
                {
                    float dx = straight[s * 3 + 0] - straight[(s - 1) * 3 + 0];
                    float dy = straight[s * 3 + 1] - straight[(s - 1) * 3 + 1];
                    float dz = straight[s * 3 + 2] - straight[(s - 1) * 3 + 2];
                    ltLen += sqrtf(dx * dx + dy * dy + dz * dz);
                }
            }
            else
            {
                float dx = ltNearest[0] - startNearest[0];
                float dz = ltNearest[2] - startNearest[2];
                ltLen = sqrtf(dx * dx + dz * dz);
            }

            // compute player margin to lateral target (cheap fallback like earlier)
            float playerLen = 0.0f;
            if (threatRef == 0)
                playerLen = glm::distance(threatFixed, lt);
            else
            {
                dtPolyRef ppath[MAX_POLYS];
                int pcount = 0;
                if (!dtStatusSucceed(navQuery->findPath(threatRef, ltRef, threatNearest, ltNearest, &filter, ppath, &pcount, MAX_POLYS)) || pcount == 0)
                    playerLen = 1e6f;
                else
                {
                    float straightP[3 * MAX_STRAIGHT];
                    unsigned char flagsP[MAX_STRAIGHT];
                    dtPolyRef spPolysP[MAX_STRAIGHT];
                    int spCount = 0;
                    if (dtStatusSucceed(navQuery->findStraightPath(threatNearest, ltNearest, ppath, pcount, straightP, flagsP, spPolysP, &spCount, MAX_STRAIGHT, DT_STRAIGHTPATH_ALL_CROSSINGS)) && spCount > 0)
                    {
                        for (int s = 1; s < spCount; ++s)
                        {
                            float dx = straightP[s * 3 + 0] - straightP[(s - 1) * 3 + 0];
                            float dy = straightP[s * 3 + 1] - straightP[(s - 1) * 3 + 1];
                            float dz = straightP[s * 3 + 2] - straightP[(s - 1) * 3 + 2];
                            playerLen += sqrtf(dx * dx + dy * dy + dz * dz);
                        }
                    }
                    else
                    {
                        playerLen = glm::distance(threatFixed, lt);
                    }
                }
            }

            float ltNpcTime = ltLen / std::max(0.0001f, npcSpeed);
            float ltPlayerTime = playerLen / std::max(0.0001f, playerSpeed);
            float ltMargin = ltPlayerTime - ltNpcTime;
            if (ltMargin > -0.3f && ltLen > 0.6f)
            {
                // return straight path to ltNearest
                bool reached = (polyPath[polyCount - 1] == ltRef);
                float gNearest[3];
                if (!reached)
                    navQuery->closestPointOnPoly(polyPath[polyCount - 1], ltNearest, gNearest, nullptr);
                else { gNearest[0] = ltNearest[0]; gNearest[1] = ltNearest[1]; gNearest[2] = ltNearest[2]; }

                float straight2[MAX_STRAIGHT * 3];
                unsigned char flags2[MAX_STRAIGHT];
                dtPolyRef sp2[MAX_STRAIGHT];
                int sc = 0;
                if (dtStatusSucceed(navQuery->findStraightPath(startNearest, reached ? ltNearest : gNearest, polyPath, polyCount, straight2, flags2, sp2, &sc, MAX_STRAIGHT, DT_STRAIGHTPATH_ALL_CROSSINGS)) && sc > 0)
                {
                    outPath.reserve(sc);
                    for (int idx = 1; idx < sc; ++idx)
                        outPath.emplace_back(straight2[idx * 3 + 0], straight2[idx * 3 + 1], straight2[idx * 3 + 2]);
                    dtFreeNavMeshQuery(navQuery);
                    return outPath;
                }
            }
        }
    }

    // --- final: build path to best candidate if found ---
    if (bestIndex >= 0)
    {
        Candidate& best = candidates[bestIndex];
        dtPolyRef polyPath[MAX_POLYS];
        int polyCount = 0;
        if (dtStatusSucceed(navQuery->findPath(startRef, best.ref, startNearest, best.posF, &filter, polyPath, &polyCount, MAX_POLYS)) && polyCount > 0)
        {
            bool reached = (polyPath[polyCount - 1] == best.ref);
            float gNearest[3] = { 0,0,0 };
            if (!reached)
                navQuery->closestPointOnPoly(polyPath[polyCount - 1], best.posF, gNearest, nullptr);
            else { gNearest[0] = best.posF[0]; gNearest[1] = best.posF[1]; gNearest[2] = best.posF[2]; }

            float straight[MAX_STRAIGHT * 3];
            unsigned char flags[MAX_STRAIGHT];
            dtPolyRef spPolys[MAX_STRAIGHT];
            int spCount = 0;
            if (dtStatusSucceed(navQuery->findStraightPath(startNearest, reached ? best.posF : gNearest, polyPath, polyCount, straight, flags, spPolys, &spCount, MAX_STRAIGHT, DT_STRAIGHTPATH_ALL_CROSSINGS)) && spCount > 0)
            {
                outPath.reserve(spCount);
                for (int s = 1; s < spCount; ++s)
                    outPath.emplace_back(straight[s * 3 + 0], straight[s * 3 + 1], straight[s * 3 + 2]);
            }
            else
            {
                // fallback single point
                outPath.emplace_back(glm::vec3(best.posF[0], best.posF[1], best.posF[2]));
            }
        }
        else
        {
            outPath.emplace_back(glm::vec3(best.posF[0], best.posF[1], best.posF[2]));
        }
    }

    dtFreeNavMeshQuery(navQuery);
    return outPath;
}

std::vector<glm::vec3> NavigationSystem::FindFleePathSimple(
    const glm::vec3& start,
    const glm::vec3& threatPos,
    float desiredDist /*= 12.0f*/,
    int maxCandidates /*= 8*/,
    float npcSpeed /*= 4.0f*/,
    float playerSpeed /*= 5.0f*/)
{
    std::vector<glm::vec3> outPath;
    if (!navMesh) return outPath;
    std::lock_guard<std::recursive_mutex> _lock(mainLock);

    // --- navQuery ---
    dtNavMeshQuery* navQuery = dtAllocNavMeshQuery();
    if (!navQuery) return outPath;
    if (dtStatusFailed(navQuery->init(navMesh, 2048)))
    {
        dtFreeNavMeshQuery(navQuery);
        return outPath;
    }

    // --- snap to ground like FindSimplePath ---
    auto hit = Physics::LineTrace(start, start - glm::vec3(0, 300, 0), BodyType::World);
    glm::vec3 startFixed = hit.hasHit ? hit.position + glm::vec3(0, 1, 0) : start;
    hit = Physics::LineTrace(threatPos, threatPos - glm::vec3(0, 300, 0), BodyType::World);
    glm::vec3 threatFixed = hit.hasHit ? hit.position + glm::vec3(0, 1, 0) : threatPos;

    CustomFilter filter;
    filter.setIncludeFlags(0xffff);
    filter.setExcludeFlags(0);

    // --- helper: findNearestPoly with progressive extents (same as your code) ---
    const glm::vec3 EXTENTS_LIST[] = {
        {0.50f, 1.5f, 0.50f},
        {0.60f, 1.5f, 0.60f},
        {1.00f, 1.5f, 1.00f},
        {1.50f, 1.5f, 1.50f},
    };
    auto findOnPoly = [&](const float pos[3], dtPolyRef& outRef, float outNearest[3]) -> bool {
        for (auto& e : EXTENTS_LIST)
        {
            float ext[3] = { e.x, e.y, e.z };
            if (dtStatusSucceed(navQuery->findNearestPoly(pos, ext, &filter, &outRef, outNearest)) && outRef)
            {
                if (IsPointReallyOnPoly(navQuery, outRef, pos))
                    return true;
            }
        }
        return false;
        };

    // --- localize start & threat ---
    dtPolyRef startRef = 0, threatRef = 0;
    float startNearest[3], threatNearest[3];
    float startF[3] = { startFixed.x, startFixed.y, startFixed.z };
    float threatF[3] = { threatFixed.x, threatFixed.y, threatFixed.z };

    if (!findOnPoly(startF, startRef, startNearest))
    {
        dtFreeNavMeshQuery(navQuery);
        return outPath;
    }
    findOnPoly(threatF, threatRef, threatNearest); // may fail; we handle below

    const int MAX_POLYS = 256; // reduced from 512 for optimization
    const int MAX_STRAIGHT = 256; // reduced

    // --- compute baseline: player -> current NPC (navmesh path length) ---
    float playerToStartLen = 0.0f;
    if (threatRef != 0)
    {
        dtPolyRef ppath[MAX_POLYS];
        int pcount = 0;
        if (dtStatusSucceed(navQuery->findPath(threatRef, startRef, threatNearest, startNearest, &filter, ppath, &pcount, MAX_POLYS)) && pcount > 0)
        {
            float straightP[3 * MAX_STRAIGHT];
            unsigned char flagsP[MAX_STRAIGHT];
            dtPolyRef spPolysP[MAX_STRAIGHT];
            int spCount = 0;
            if (dtStatusSucceed(navQuery->findStraightPath(threatNearest, startNearest, ppath, pcount, straightP, flagsP, spPolysP, &spCount, MAX_STRAIGHT, DT_STRAIGHTPATH_ALL_CROSSINGS)) && spCount > 0)
            {
                for (int s = 1; s < spCount; ++s)
                {
                    float dx = straightP[s * 3 + 0] - straightP[(s - 1) * 3 + 0];
                    float dy = straightP[s * 3 + 1] - straightP[(s - 1) * 3 + 1];
                    float dz = straightP[s * 3 + 2] - straightP[(s - 1) * 3 + 2];
                    playerToStartLen += sqrtf(dx * dx + dy * dy + dz * dz);
                }
            }
            else
            {
                playerToStartLen = glm::distance(threatFixed, startFixed);
            }
        }
        else
        {
            playerToStartLen = glm::distance(threatFixed, startFixed);
        }
    }
    else
    {
        playerToStartLen = glm::distance(threatFixed, startFixed);
    }

    // --- sampling: forward-biased, reduced samples to optimize ---
    glm::vec3 fleeDir = glm::vec3(startFixed.x - threatFixed.x, 0.0f, startFixed.z - threatFixed.z);
    if (glm::length2(fleeDir) < 1e-6f)
    {
        float a = (float)(rand() % 314) / 100.0f;
        fleeDir = glm::vec3(std::cos(a), 0.0f, std::sin(a));
    }
    fleeDir = glm::normalize(fleeDir);
    float baseAng = atan2f(fleeDir.z, fleeDir.x);

    struct AngleSample { float ang; float weight; };
    std::vector<AngleSample> angleSamples;
    // front dense (reduced)
    const int FRONT_SAMPLES = 6;
    const float FRONT_HALF = glm::radians(60.0f); // tightened to focus away
    for (int i = 0; i < FRONT_SAMPLES; ++i)
    {
        float t = FRONT_SAMPLES == 1 ? 0.0f : (float)i / float(FRONT_SAMPLES - 1);
        float a = baseAng - FRONT_HALF + t * (2.0f * FRONT_HALF);
        angleSamples.push_back({ a, 1.0f });
    }
    // sides (reduced, no back to avoid towards threat)
    const int SIDE_SAMPLES = 6;
    const float SIDE_MIN = glm::radians(65.0f), SIDE_MAX = glm::radians(110.0f);
    for (int i = 0; i < SIDE_SAMPLES / 2; ++i)
    {
        float t = (float)i / float(SIDE_SAMPLES / 2 - 1);
        float a = baseAng + SIDE_MIN + t * (SIDE_MAX - SIDE_MIN);
        angleSamples.push_back({ a, 0.7f }); // higher weight than original
    }
    for (int i = 0; i < SIDE_SAMPLES / 2; ++i)
    {
        float t = (float)i / float(SIDE_SAMPLES / 2 - 1);
        float a = baseAng - SIDE_MIN - t * (SIDE_MAX - SIDE_MIN);
        angleSamples.push_back({ a, 0.7f });
    }

    std::vector<float> radii = { desiredDist * 0.8f, desiredDist, desiredDist * 1.2f }; // reduced

    struct Candidate {
        glm::vec3 pos;
        float posF[3];
        dtPolyRef ref;
        float npcPathLen;
        float angleWeight;
    };
    std::vector<Candidate> candidates;
    candidates.reserve(64); // cap lower

    float extSample[3] = { 0.6f, 1.5f, 0.6f };

    auto dirFromAngle = [](float a)->glm::vec3 { return glm::vec3(cosf(a), 0.0f, sinf(a)); };

    // collect candidates
    for (const auto& as : angleSamples)
    {
        glm::vec3 d = dirFromAngle(as.ang);
        for (float r : radii)
        {
            if ((int)candidates.size() >= 64) break;
            glm::vec3 sampleWorld = glm::vec3(startNearest[0], startNearest[1], startNearest[2]) + d * r;
            float sampleF[3] = { sampleWorld.x, sampleWorld.y, sampleWorld.z };

            dtPolyRef candRef = 0;
            float candNearest[3];
            if (!dtStatusSucceed(navQuery->findNearestPoly(sampleF, extSample, &filter, &candRef, candNearest)) || candRef == 0)
                continue;

            dtPolyRef polyPath[MAX_POLYS];
            int polyCount = 0;
            if (!dtStatusSucceed(navQuery->findPath(startRef, candRef, startNearest, candNearest, &filter, polyPath, &polyCount, MAX_POLYS)))
                continue;
            if (polyCount == 0 && startRef != candRef) continue;

            float straight[MAX_STRAIGHT * 3];
            unsigned char flags[MAX_STRAIGHT];
            dtPolyRef spPolys[MAX_STRAIGHT];
            int spCount = 0;
            float npcLen = 0.0f;
            if (dtStatusSucceed(navQuery->findStraightPath(startNearest, candNearest, polyPath, polyCount, straight, flags, spPolys, &spCount, MAX_STRAIGHT, DT_STRAIGHTPATH_ALL_CROSSINGS)) && spCount > 0)
            {
                for (int s = 1; s < spCount; ++s)
                {
                    float dx = straight[s * 3 + 0] - straight[(s - 1) * 3 + 0];
                    float dy = straight[s * 3 + 1] - straight[(s - 1) * 3 + 1];
                    float dz = straight[s * 3 + 2] - straight[(s - 1) * 3 + 2];
                    npcLen += sqrtf(dx * dx + dy * dy + dz * dz);
                }
            }
            else
            {
                float dx = candNearest[0] - startNearest[0];
                float dz = candNearest[2] - startNearest[2];
                npcLen = sqrtf(dx * dx + dz * dz);
            }

            Candidate c;
            c.pos = glm::vec3(candNearest[0], candNearest[1], candNearest[2]);
            c.posF[0] = candNearest[0]; c.posF[1] = candNearest[1]; c.posF[2] = candNearest[2];
            c.ref = candRef;
            c.npcPathLen = npcLen;
            c.angleWeight = as.weight;
            candidates.push_back(c);
        }
        if ((int)candidates.size() >= 64) break;
    }

    if (candidates.empty())
    {
        dtFreeNavMeshQuery(navQuery);
        return FindSimplePath(startFixed, startFixed + fleeDir * 3.0f);
    }

    // sort preferring forward and longer
    std::sort(candidates.begin(), candidates.end(), [](const Candidate& a, const Candidate& b) {
        return (a.angleWeight * a.npcPathLen) > (b.angleWeight * b.npcPathLen);
        });

    int evaluateTop = std::min<int>(maxCandidates, (int)candidates.size());
    float bestScore = -1e9f;
    int bestIndex = -1;

    // simplified scoring constants
    const float W_GAIN = 4.0f;
    const float W_INTERCEPT = 1.5f;

    for (int i = 0; i < evaluateTop; ++i)
    {
        Candidate& cand = candidates[i];

        // 1) player -> candidate path length
        float playerToCandLen = 0.0f;
        if (threatRef == 0)
        {
            playerToCandLen = glm::distance(threatFixed, cand.pos);
        }
        else
        {
            dtPolyRef ppath[MAX_POLYS];
            int pcount = 0;
            if (!dtStatusSucceed(navQuery->findPath(threatRef, cand.ref, threatNearest, cand.posF, &filter, ppath, &pcount, MAX_POLYS)) || pcount == 0)
            {
                playerToCandLen = 1e6f; // unreachable: good
            }
            else
            {
                float straightP[3 * MAX_STRAIGHT];
                unsigned char flagsP[MAX_STRAIGHT];
                dtPolyRef spPolysP[MAX_STRAIGHT];
                int spCount = 0;
                if (dtStatusSucceed(navQuery->findStraightPath(threatNearest, cand.posF, ppath, pcount, straightP, flagsP, spPolysP, &spCount, MAX_STRAIGHT, DT_STRAIGHTPATH_ALL_CROSSINGS)) && spCount > 0)
                {
                    for (int s = 1; s < spCount; ++s)
                    {
                        float dx = straightP[s * 3 + 0] - straightP[(s - 1) * 3 + 0];
                        float dy = straightP[s * 3 + 1] - straightP[(s - 1) * 3 + 1];
                        float dz = straightP[s * 3 + 2] - straightP[(s - 1) * 3 + 2];
                        playerToCandLen += sqrtf(dx * dx + dy * dy + dz * dz);
                    }
                }
                else
                {
                    playerToCandLen = glm::distance(threatFixed, cand.pos);
                }
            }
        }

        // 2) intercept margin
        float npcTime = cand.npcPathLen / std::max(0.0001f, npcSpeed);
        float playerTime = playerToCandLen / std::max(0.0001f, playerSpeed);
        float interceptMargin = playerTime - npcTime;

        // 3) distance-gain-per-meter
        float deltaDist = playerToCandLen - playerToStartLen;
        float gainPerMeter = deltaDist / std::max(0.01f, cand.npcPathLen);
        if (playerToCandLen > 1e5f)
            gainPerMeter = 1e3f;

        // simplified score
        float score = W_GAIN * gainPerMeter + W_INTERCEPT * interceptMargin;

        // penalties
        if (gainPerMeter < -0.05f) score -= 3.0f;
        if (interceptMargin < -2.0f) score -= 5.0f;

        if (score > bestScore)
        {
            bestScore = score;
            bestIndex = i;
        }
    }

    // fallback: short path in flee direction
    if (bestIndex < 0)
    {
        dtFreeNavMeshQuery(navQuery);
        return FindSimplePath(startFixed, startFixed + fleeDir * desiredDist * 0.5f);
    }

    // build path to best
    Candidate& best = candidates[bestIndex];
    dtPolyRef polyPath[MAX_POLYS];
    int polyCount = 0;
    if (dtStatusSucceed(navQuery->findPath(startRef, best.ref, startNearest, best.posF, &filter, polyPath, &polyCount, MAX_POLYS)) && polyCount > 0)
    {
        bool reached = (polyPath[polyCount - 1] == best.ref);
        float gNearest[3] = { 0,0,0 };
        if (!reached)
            navQuery->closestPointOnPoly(polyPath[polyCount - 1], best.posF, gNearest, nullptr);
        else { gNearest[0] = best.posF[0]; gNearest[1] = best.posF[1]; gNearest[2] = best.posF[2]; }

        float straight[MAX_STRAIGHT * 3];
        unsigned char flags[MAX_STRAIGHT];
        dtPolyRef spPolys[MAX_STRAIGHT];
        int spCount = 0;
        if (dtStatusSucceed(navQuery->findStraightPath(startNearest, reached ? best.posF : gNearest, polyPath, polyCount, straight, flags, spPolys, &spCount, MAX_STRAIGHT, DT_STRAIGHTPATH_ALL_CROSSINGS)) && spCount > 0)
        {
            outPath.reserve(spCount);
            for (int s = 1; s < spCount; ++s)
                outPath.emplace_back(straight[s * 3 + 0], straight[s * 3 + 1], straight[s * 3 + 2]);
        }
        else
        {
            outPath.emplace_back(glm::vec3(best.posF[0], best.posF[1], best.posF[2]));
        }
    }
    else
    {
        outPath.emplace_back(glm::vec3(best.posF[0], best.posF[1], best.posF[2]));
    }

    dtFreeNavMeshQuery(navQuery);
    return outPath;
}