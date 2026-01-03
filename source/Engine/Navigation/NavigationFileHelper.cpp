#include "NavigationFileHelper.h"
#include "NavigationGenerationHelpers.hpp"
#include "../Logger.hpp"

/* ========================================================= */
/* ========================= SAVE =========================== */
/* ========================================================= */

bool NavigationFileHelper::Save(
    const char* filePath,
    dtNavMesh* navMesh,
    dtTileCache* tileCache, uint32_t version)
{
    if (!navMesh || !tileCache)
        return false;

#ifdef __EMSCRIPTEN__

    Logger::Log("can't save file on this platform");

    return false;

#endif // __EMSCRIPTEN__


    FILE* fp = std::fopen(filePath, "wb");
    if (!fp)
        return false;

    NavMeshFileHeader fileHeader{};
    fileHeader.magic = NAVMESH_FILE_MAGIC;
    fileHeader.version = version;

    const long rootHeaderPos = ftell(fp);
    fwrite(&fileHeader, sizeof(fileHeader), 1, fp);

    /* ================= NavMesh ================= */

    fileHeader.navMeshOffset = (uint32_t)ftell(fp);

    NavMeshSetHeader nmHeader{};
    nmHeader.params = *navMesh->getParams();
    nmHeader.numTiles = 0;

    for (int i = 0; i < navMesh->getMaxTiles(); ++i)
    {
        const dtMeshTile* tile = navMesh->getTile(i);
        if (tile && tile->header && tile->dataSize)
            nmHeader.numTiles++;
    }

    fwrite(&nmHeader, sizeof(nmHeader), 1, fp);

    for (int i = 0; i < navMesh->getMaxTiles(); ++i)
    {
        const dtMeshTile* tile = navMesh->getTile(i);
        if (!tile || !tile->header || !tile->dataSize)
            continue;

        NavMeshTileHeader th{};
        th.tileRef = navMesh->getTileRef(tile);
        th.dataSize = tile->dataSize;

        fwrite(&th, sizeof(th), 1, fp);
        fwrite(tile->data, tile->dataSize, 1, fp);
    }

    /* ================= TileCache ================= */

    fileHeader.tileCacheOffset = (uint32_t)ftell(fp);

    TileCacheSetHeader tcHeader{};
    tcHeader.params = *tileCache->getParams();
    tcHeader.numTiles = 0;

    for (int i = 0; i < tileCache->getTileCount(); ++i)
    {
        const dtCompressedTile* tile = tileCache->getTile(i);
        if (tile && tile->dataSize)
            tcHeader.numTiles++;
    }

    fwrite(&tcHeader, sizeof(tcHeader), 1, fp);

    for (int i = 0; i < tileCache->getTileCount(); ++i)
    {
        const dtCompressedTile* tile = tileCache->getTile(i);
        if (!tile || !tile->dataSize)
            continue;

        TileCacheTileHeader th{};
        th.tileRef = tileCache->getTileRef(tile);
        th.dataSize = tile->dataSize;

        fwrite(&th, sizeof(th), 1, fp);
        fwrite(tile->data, tile->dataSize, 1, fp);
    }

    /* ================= Finalize ================= */

    fileHeader.fileSize = (uint32_t)ftell(fp);

    fseek(fp, rootHeaderPos, SEEK_SET);
    fwrite(&fileHeader, sizeof(fileHeader), 1, fp);

    fclose(fp);
    return true;
}

/* ========================================================= */
/* ========================= LOAD =========================== */
/* ========================================================= */

bool NavigationFileHelper::Load(
    const char* filePath,
    dtNavMesh*& outNavMesh,
    dtTileCache*& outTileCache, uint32_t version)
{
    FILE* fp = std::fopen(filePath, "rb");
    if (!fp)
        return false;

    NavMeshFileHeader fileHeader{};
    fread(&fileHeader, sizeof(fileHeader), 1, fp);

    if (fileHeader.magic != NAVMESH_FILE_MAGIC ||
        fileHeader.version != version)
    {
        fclose(fp);
        return false;
    }

    /* ===================== Load NavMesh ===================== */

    fseek(fp, fileHeader.navMeshOffset, SEEK_SET);

    NavMeshSetHeader nmHeader{};
    fread(&nmHeader, sizeof(nmHeader), 1, fp);

    outNavMesh = dtAllocNavMesh();
    if (!outNavMesh ||
        dtStatusFailed(outNavMesh->init(&nmHeader.params)))
    {
        fclose(fp);
        return false;
    }

    for (uint32_t i = 0; i < nmHeader.numTiles; ++i)
    {
        NavMeshTileHeader th{};
        fread(&th, sizeof(th), 1, fp);

        unsigned char* data =
            (unsigned char*)dtAlloc(th.dataSize, DT_ALLOC_PERM);
        fread(data, th.dataSize, 1, fp);

        outNavMesh->addTile(
            data,
            th.dataSize,
            DT_TILE_FREE_DATA,
            th.tileRef,
            nullptr);
    }

    /* ===================== Load TileCache ===================== */

    fseek(fp, fileHeader.tileCacheOffset, SEEK_SET);

    TileCacheSetHeader tcHeader{};
    fread(&tcHeader, sizeof(tcHeader), 1, fp);

    // Create alloc & compressor
    dtTileCacheAlloc* talloc = new dtTileCacheAlloc();
    dtTileCacheCompressor* tcomp = new FastLZCompressor();

    outTileCache = dtAllocTileCache();
    if (!outTileCache ||
        dtStatusFailed(
            outTileCache->init(
                &tcHeader.params,
                talloc,
                tcomp,
                nullptr)))   // optional mesh processor
    {
        delete talloc;
        delete tcomp;
        fclose(fp);
        return false;
    }

    for (uint32_t i = 0; i < tcHeader.numTiles; ++i)
    {
        TileCacheTileHeader th{};
        fread(&th, sizeof(th), 1, fp);

        unsigned char* data =
            (unsigned char*)dtAlloc(th.dataSize, DT_ALLOC_PERM);
        fread(data, th.dataSize, 1, fp);

        dtCompressedTileRef newRef = 0;
        outTileCache->addTile(
            data,
            th.dataSize,
            (unsigned char)DT_COMPRESSEDTILE_FREE_DATA,
            &newRef);
    }

    fclose(fp);
    return true;
}

