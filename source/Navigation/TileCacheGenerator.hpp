#include <cstring>
#include <cstdlib>
#include "Detour/DetourNavMesh.h"
#include "Detour/DetourNavMeshBuilder.h"
#include "Detour/DetourTileCache.h"
#include "Detour/DetourTileCacheBuilder.h"
#include "Detour/DetourAlloc.h"
#include "NavMeshGenerator.hpp"

// Default helper classes for the tile cache.
struct DefaultTileCacheAlloc : public dtTileCacheAlloc {
    void* alloc(const size_t size) override { return std::malloc(size); }
    void free(void* ptr) override { std::free(ptr); }
};

struct DefaultTileCacheCompressor : public dtTileCacheCompressor {
    int maxCompressedSize(const int bufferSize) {
        return bufferSize * 2;
    }
    dtStatus compress(const unsigned char* buffer, const int bufferSize,
        unsigned char* compressed, const int maxCompressedSize, int* compSize) {
        if (maxCompressedSize < bufferSize) return DT_FAILURE;
        memcpy(compressed, buffer, bufferSize);
        *compSize = bufferSize;
        return DT_SUCCESS;
    }
    dtStatus decompress(const unsigned char* compressed, const int compSize,
        unsigned char* buffer, const int maxBufferSize, int* bufferSize) {
        if (maxBufferSize < compSize) return DT_FAILURE;
        memcpy(buffer, compressed, compSize);
        *bufferSize = compSize;
        return DT_SUCCESS;
    }
};

struct DefaultTileCacheMeshProcess : public dtTileCacheMeshProcess {
    void process(dtNavMeshCreateParams* params, unsigned char* polyAreas, unsigned short* polyFlags) {
        // No-op, override to customize area/flag generation
    }
};

class TileCacheGenerator {
public:
    TileCacheGenerator(const NavMeshGenerator::NavMeshConfig& config, dtNavMesh* navMesh)
        : m_navMesh(navMesh), m_config(config), m_tileCache(nullptr),
        m_allocator(nullptr), m_compressor(nullptr), m_meshProcess(nullptr)
    {
        m_allocator = new DefaultTileCacheAlloc();
        m_compressor = new DefaultTileCacheCompressor();
        m_meshProcess = new DefaultTileCacheMeshProcess();
    }

    ~TileCacheGenerator() {
        if (m_tileCache) dtFreeTileCache(m_tileCache);
        delete m_allocator;
        delete m_compressor;
        delete m_meshProcess;
    }

    bool init() {
        if (!m_navMesh) return false;

        dtTileCacheParams tcParams;
        memset(&tcParams, 0, sizeof(tcParams));

        const dtNavMeshParams* navParams = m_navMesh->getParams();
        if (navParams)
            memcpy(tcParams.orig, navParams->orig, sizeof(float) * 3);
        else
            memset(tcParams.orig, 0, sizeof(tcParams.orig));

        tcParams.cs = m_config.cellSize;
        tcParams.ch = m_config.cellHeight;
        tcParams.width = 48;   // width in voxels (not world units!)
        tcParams.height = 48;  // height in voxels
        tcParams.maxTiles = 256;
        tcParams.maxObstacles = 128;

        m_tileCache = dtAllocTileCache();
        if (!m_tileCache) return false;

        dtStatus status = m_tileCache->init(&tcParams, m_allocator, m_compressor, m_meshProcess);
        if (dtStatusFailed(status)) {
            dtFreeTileCache(m_tileCache);
            m_tileCache = nullptr;
            return false;
        }

        return true;
    }

    dtTileCache* getTileCache() const { return m_tileCache; }

private:
    dtNavMesh* m_navMesh;
    NavMeshGenerator::NavMeshConfig m_config;
    dtTileCache* m_tileCache;

    dtTileCacheAlloc* m_allocator;
    dtTileCacheCompressor* m_compressor;
    dtTileCacheMeshProcess* m_meshProcess;
};