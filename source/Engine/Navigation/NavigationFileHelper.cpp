#include "NavigationFileHelper.h"
#include "NavigationGenerationHelpers.hpp"
#include "../Logger.hpp"
#include "../FileSystem/FileSystem.h"
#include "../Helpers/ByteCompressor.h"

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

    std::vector<uint8_t> buffer;

    auto append = [&buffer](const void* data, size_t size) {
        size_t old_size = buffer.size();
        buffer.resize(old_size + size);
        memcpy(buffer.data() + old_size, data, size);
        };

    NavMeshFileHeader fileHeader{};
    fileHeader.magic = NAVMESH_FILE_MAGIC;
    fileHeader.version = version;

    // Reserve space for the root header
    size_t rootHeaderPos = 0;
    buffer.resize(sizeof(NavMeshFileHeader)); // Placeholder for header

    /* ================= NavMesh ================= */
    fileHeader.navMeshOffset = (uint32_t)buffer.size();
    NavMeshSetHeader nmHeader{};
    nmHeader.params = *navMesh->getParams();
    nmHeader.numTiles = 0;
    for (int i = 0; i < navMesh->getMaxTiles(); ++i)
    {
        const dtMeshTile* tile = navMesh->getTile(i);
        if (tile && tile->header && tile->dataSize)
            nmHeader.numTiles++;
    }
    append(&nmHeader, sizeof(nmHeader));
    for (int i = 0; i < navMesh->getMaxTiles(); ++i)
    {
        const dtMeshTile* tile = navMesh->getTile(i);
        if (!tile || !tile->header || !tile->dataSize)
            continue;
        NavMeshTileHeader th{};
        th.tileRef = navMesh->getTileRef(tile);
        th.dataSize = tile->dataSize;
        append(&th, sizeof(th));
        append(tile->data, tile->dataSize);
    }

    /* ================= TileCache ================= */
    fileHeader.tileCacheOffset = (uint32_t)buffer.size();
    TileCacheSetHeader tcHeader{};
    tcHeader.params = *tileCache->getParams();
    tcHeader.numTiles = 0;
    for (int i = 0; i < tileCache->getTileCount(); ++i)
    {
        const dtCompressedTile* tile = tileCache->getTile(i);
        if (tile && tile->dataSize)
            tcHeader.numTiles++;
    }
    append(&tcHeader, sizeof(tcHeader));
    for (int i = 0; i < tileCache->getTileCount(); ++i)
    {
        const dtCompressedTile* tile = tileCache->getTile(i);
        if (!tile || !tile->dataSize)
            continue;
        TileCacheTileHeader th{};
        th.tileRef = tileCache->getTileRef(tile);
        th.dataSize = tile->dataSize;
        append(&th, sizeof(th));
        append(tile->data, tile->dataSize);
    }

    /* ================= Finalize ================= */
    fileHeader.fileSize = (uint32_t)buffer.size();

    // Overwrite the header at the beginning
    memcpy(buffer.data() + rootHeaderPos, &fileHeader, sizeof(fileHeader));

    // Compress the buffer
    buffer = ByteCompressor::CompressData(buffer);

    // Write the compressed data to file (assuming FileSystemEngine has a WriteFileBinary method)
    return FileSystemEngine::WriteFileBinary(filePath, buffer);
}

/* ========================================================= */
/* ========================= LOAD =========================== */
/* ========================================================= */
bool NavigationFileHelper::Load(
    const char* filePath,
    dtNavMesh*& outNavMesh,
    dtTileCache*& outTileCache, uint32_t version)
{
    std::vector<uint8_t> compressed = FileSystemEngine::ReadFileBinary(filePath);
    if (compressed.empty())
        return false;

    std::vector<uint8_t> buffer = ByteCompressor::DecompressData(compressed);
    if (buffer.empty())
        return false;

    size_t pos = 0;

    auto read = [&buffer, &pos](void* dest, size_t size) {
        if (pos + size > buffer.size())
            return false; // Error: out of bounds
        memcpy(dest, buffer.data() + pos, size);
        pos += size;
        return true;
        };

    auto seek = [&pos](size_t offset) {
        pos = offset;
        };

    NavMeshFileHeader fileHeader{};
    if (!read(&fileHeader, sizeof(fileHeader)))
        return false;

    if (fileHeader.magic != NAVMESH_FILE_MAGIC ||
        fileHeader.version != version)
    {
        return false;
    }

    /* ===================== Load NavMesh ===================== */
    seek(fileHeader.navMeshOffset);
    NavMeshSetHeader nmHeader{};
    if (!read(&nmHeader, sizeof(nmHeader)))
        return false;

    outNavMesh = dtAllocNavMesh();
    if (!outNavMesh ||
        dtStatusFailed(outNavMesh->init(&nmHeader.params)))
    {
        return false;
    }

    for (uint32_t i = 0; i < nmHeader.numTiles; ++i)
    {
        NavMeshTileHeader th{};
        if (!read(&th, sizeof(th)))
            return false;

        unsigned char* data =
            (unsigned char*)dtAlloc(th.dataSize, DT_ALLOC_PERM);
        if (!data || !read(data, th.dataSize))
        {
            dtFree(data);
            return false;
        }

        outNavMesh->addTile(
            data,
            th.dataSize,
            DT_TILE_FREE_DATA,
            th.tileRef,
            nullptr);
    }

    /* ===================== Load TileCache ===================== */
    seek(fileHeader.tileCacheOffset);
    TileCacheSetHeader tcHeader{};
    if (!read(&tcHeader, sizeof(tcHeader)))
        return false;

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
                nullptr))) // optional mesh processor
    {
        delete talloc;
        delete tcomp;
        return false;
    }

    for (uint32_t i = 0; i < tcHeader.numTiles; ++i)
    {
        TileCacheTileHeader th{};
        if (!read(&th, sizeof(th)))
            return false;

        unsigned char* data =
            (unsigned char*)dtAlloc(th.dataSize, DT_ALLOC_PERM);
        if (!data || !read(data, th.dataSize))
        {
            dtFree(data);
            return false;
        }

        dtCompressedTileRef newRef = 0;
        outTileCache->addTile(
            data,
            th.dataSize,
            (unsigned char)DT_COMPRESSEDTILE_FREE_DATA,
            &newRef);
    }

    return true;
}