#pragma once
#include <cstdint>
#include <cstdio>

#include "Detour/DetourNavMesh.h"
#include "Detour/DetourTileCache.h"
#include "Detour/DetourTileCacheBuilder.h"

class NavigationFileHelper
{
public:
    static const uint32_t NAVMESH_FILE_MAGIC = 'NMTC'; // NavMesh + TileCache

    /* ================= Root Header ================= */

    struct NavMeshFileHeader
    {
        uint32_t magic;
        uint32_t version;

        uint32_t navMeshOffset;
        uint32_t tileCacheOffset;

        uint32_t fileSize;
    };

    /* ================= NavMesh ================= */

    struct NavMeshSetHeader
    {
        uint32_t numTiles;
        dtNavMeshParams params;
    };

    struct NavMeshTileHeader
    {
        dtTileRef tileRef;
        uint32_t dataSize;
    };

    /* ================= TileCache ================= */

    struct TileCacheSetHeader
    {
        uint32_t numTiles;
        dtTileCacheParams params;
    };

    struct TileCacheTileHeader
    {
        dtCompressedTileRef tileRef;
        uint32_t dataSize;
    };

public:
    static bool Save(
        const char* filePath,
        dtNavMesh* navMesh,
        dtTileCache* tileCache, uint32_t version);

    static bool Load(
        const char* filePath,
        dtNavMesh*& outNavMesh,
        dtTileCache*& outTileCache, uint32_t version);
};
