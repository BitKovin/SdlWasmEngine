#include "TiledVoxelWorld.h"
#include <algorithm>
#include <cmath>
#include <cstring>

TiledVoxelWorld::TiledVoxelWorld(const glm::vec3& minWorld,
    const glm::vec3& maxWorld,
    float voxelSize)
    : m_Min(minWorld)
    , m_Max(maxWorld)
    , m_VoxelSize(voxelSize)
{
    glm::vec3 size = m_Max - m_Min;
    m_VoxelDim = glm::ivec3(
        int(std::ceil(size.x / voxelSize)),
        int(std::ceil(size.y / voxelSize)),
        int(std::ceil(size.z / voxelSize))
    );

    m_TileDim = (m_VoxelDim + TILE_SIZE - 1) / TILE_SIZE;
    m_Tiles.resize(m_TileDim.x * m_TileDim.y * m_TileDim.z);
}

static std::vector<glm::vec3> GenerateVoxelSampleOffsets(int subdivisions)
{
    std::vector<glm::vec3> offsets;
    offsets.reserve(subdivisions * subdivisions * subdivisions);

    float step = 1.0f / subdivisions;
    float halfStep = step * 0.5f; // center of each sub-voxel

    for (int z = 0; z < subdivisions; ++z)
        for (int y = 0; y < subdivisions; ++y)
            for (int x = 0; x < subdivisions; ++x)
            {
                offsets.emplace_back(
                    x * step + halfStep,
                    y * step + halfStep,
                    z * step + halfStep
                );
            }
    return offsets;
}

void TiledVoxelWorld::BuildStatic(const VoxelWorldBuilder& builder, int subdivisions)
{
    auto sampleOffsets = GenerateVoxelSampleOffsets(subdivisions);
    const int numSamples = int(sampleOffsets.size());

    for (int tz = 0; tz < m_TileDim.z; ++tz)
        for (int ty = 0; ty < m_TileDim.y; ++ty)
            for (int tx = 0; tx < m_TileDim.x; ++tx)
            {
                VoxelTile& tile = Tile({ tx, ty, tz });

                for (int z = 0; z < TILE_SIZE; ++z)
                    for (int y = 0; y < TILE_SIZE; ++y)
                        for (int x = 0; x < TILE_SIZE; ++x)
                        {
                            glm::ivec3 v(tx * TILE_SIZE + x,
                                ty * TILE_SIZE + y,
                                tz * TILE_SIZE + z);

                            if (!glm::all(glm::lessThan(v, m_VoxelDim)))
                            {
                                tile.Static(x, y, z).value = 255;
                                continue;
                            }

                            float sum = 0.0f;
                            for (const auto& offset : sampleOffsets)
                            {
                                glm::vec3 samplePos = m_Min
                                    + (glm::vec3(v) + offset) * m_VoxelSize;

                                sum += float(builder.GetDensity(samplePos));
                            }

                            uint8_t avgDensity = static_cast<uint8_t>(sum / numSamples + 0.5f);
                            tile.Static(x, y, z).value = avgDensity;
                        }

                tile.ResetToStatic();
            }
}


int TiledVoxelWorld::AddObstacle(const VoxelObstacle& obs)
{
    VoxelObstacle o = obs;
    o.id = m_NextObstacleId++;
    m_Obstacles[o.id] = o;
    MarkTilesDirty(o);
    return o.id;
}

void TiledVoxelWorld::RemoveObstacle(int obstacleId)
{
    auto it = m_Obstacles.find(obstacleId);
    if (it != m_Obstacles.end())
    {
        MarkTilesDirty(it->second);
        m_Obstacles.erase(it);
    }
}

void TiledVoxelWorld::MarkTilesDirty(const VoxelObstacle& obs)
{
    glm::ivec3 minV = WorldToVoxel(obs.min);
    glm::ivec3 maxV = WorldToVoxel(obs.max);

    minV = glm::clamp(minV, glm::ivec3(0), m_VoxelDim - 1);
    maxV = glm::clamp(maxV, glm::ivec3(0), m_VoxelDim - 1);

    for (int z = minV.z; z <= maxV.z; ++z)
        for (int y = minV.y; y <= maxV.y; ++y)
            for (int x = minV.x; x <= maxV.x; ++x)
            {
                glm::ivec3 tileIdx(x / TILE_SIZE, y / TILE_SIZE, z / TILE_SIZE);
                m_DirtyTiles.insert(tileIdx);
            }
}

void TiledVoxelWorld::Update()
{
    for (const auto& tileIdx : m_DirtyTiles)
    {
        VoxelTile& tile = Tile(tileIdx);
        tile.ResetToStatic();

        // rasterize only obstacles that intersect this tile
        for (const auto& [id, obs] : m_Obstacles)
        {
            glm::ivec3 tileMin = WorldToVoxel(obs.min) / TILE_SIZE;
            glm::ivec3 tileMax = WorldToVoxel(obs.max) / TILE_SIZE;

            if (glm::all(glm::lessThanEqual(tileMin, tileIdx)) &&
                glm::all(glm::greaterThanEqual(tileMax, tileIdx)))
            {
                RasterizeTile(tile, tileIdx, obs);
            }
        }
    }

    m_DirtyTiles.clear();
}


void TiledVoxelWorld::RasterizeTile(VoxelTile& tile, const glm::ivec3& tileIndex, const VoxelObstacle& obs)
{
    glm::ivec3 tileOrigin = tileIndex * TILE_SIZE;

    // compute intersection of tile with obstacle bounds in voxel space
    glm::ivec3 tileMin = tileOrigin;
    glm::ivec3 tileMax = tileOrigin + glm::ivec3(TILE_SIZE) - glm::ivec3(1);

    glm::ivec3 obsMin = WorldToVoxel(obs.min);
    glm::ivec3 obsMax = WorldToVoxel(obs.max);

    glm::ivec3 start = glm::max(tileMin, obsMin);
    glm::ivec3 end = glm::min(tileMax, obsMax);

    for (int z = start.z; z <= end.z; ++z)
        for (int y = start.y; y <= end.y; ++y)
            for (int x = start.x; x <= end.x; ++x)
            {
                glm::ivec3 local = glm::ivec3(x, y, z) - tileOrigin;
                Voxel& dst = tile.Current(local.x, local.y, local.z);
                dst.value = std::max((int)dst.value, (int)obs.density);
                tile.dirty = true;
            }
}


Voxel TiledVoxelWorld::SampleWorld(const glm::vec3& worldPos) const
{
    glm::ivec3 v = WorldToVoxel(worldPos);
    if (glm::any(glm::lessThan(v, glm::ivec3(0))) ||
        glm::any(glm::greaterThanEqual(v, m_VoxelDim)))
        return Voxel{0};

    TileAddress a = AddressFromVoxel(v);
    const VoxelTile& tile = Tile(a.tile);
    return tile.currentDensity[VoxelTile::Index(a.local.x, a.local.y, a.local.z)];
}

size_t TiledVoxelWorld::GetVoxelWorldMemoryBytes()
{
    size_t bytes = sizeof(this); // base object size

    // Add vector memory
    bytes += m_Tiles.capacity() * sizeof(VoxelTile);

    // Add dynamic obstacle memory
    bytes += m_Obstacles.size() * sizeof(VoxelObstacle);

    // Add dirty tile set memory (rough estimate)
    bytes += m_DirtyTiles.size() * (sizeof(glm::ivec3) + sizeof(void*));

    return bytes;
}

TiledVoxelWorld::TileAddress TiledVoxelWorld::AddressFromVoxel(const glm::ivec3& v) const
{
    return { v / TILE_SIZE, v % TILE_SIZE };
}

glm::ivec3 TiledVoxelWorld::WorldToVoxel(const glm::vec3& p) const
{
    return glm::ivec3((p - m_Min) / m_VoxelSize);
}

glm::vec3 TiledVoxelWorld::VoxelCenterWorld(const glm::ivec3& v) const
{
    return m_Min + (glm::vec3(v) + 0.5f) * m_VoxelSize;
}

VoxelTile& TiledVoxelWorld::Tile(const glm::ivec3& t)
{
    int idx = t.x + m_TileDim.x * (t.y + m_TileDim.y * t.z);
    return m_Tiles[idx];
}

const VoxelTile& TiledVoxelWorld::Tile(const glm::ivec3& t) const
{
    int idx = t.x + m_TileDim.x * (t.y + m_TileDim.y * t.z);
    return m_Tiles[idx];
}


// --- public helpers used by pathfinding ---
// Converts world pos -> voxel index (safe) with bounds checking
bool TiledVoxelWorld::WorldToVoxelSafe(const glm::vec3& worldPos, glm::ivec3& outVoxel) const
{
    glm::vec3 local = (worldPos - m_Min) / m_VoxelSize;
    // floor to integer voxel indices
    outVoxel = glm::ivec3(static_cast<int>(std::floor(local.x)),
        static_cast<int>(std::floor(local.y)),
        static_cast<int>(std::floor(local.z)));
    // bounds check
    if (glm::any(glm::lessThan(outVoxel, glm::ivec3(0))) ||
        glm::any(glm::greaterThanEqual(outVoxel, m_VoxelDim)))
        return false;
    return true;
}

// Returns world-space center of voxel index v
glm::vec3 TiledVoxelWorld::VoxelCenterWorldPublic(const glm::ivec3& v) const
{
    // assume caller provides valid v inside bounds
    return m_Min + (glm::vec3(v) + 0.5f) * m_VoxelSize;
}

// Get density stored in currentDensity for voxel index v
uint8_t TiledVoxelWorld::GetVoxelDensityAt(const glm::ivec3& v) const
{
    // bounds check, return solid if outside
    if (glm::any(glm::lessThan(v, glm::ivec3(0))) ||
        glm::any(glm::greaterThanEqual(v, m_VoxelDim)))
        return 255;
    TileAddress a = AddressFromVoxel(v);
    const VoxelTile& tile = Tile(a.tile);
    return tile.currentDensity[VoxelTile::Index(a.local.x, a.local.y, a.local.z)].value;
}

size_t TiledVoxelWorld::VoxelCount() const
{
    return static_cast<size_t>(m_VoxelDim.x) * m_VoxelDim.y * m_VoxelDim.z;
}

int TiledVoxelWorld::FlattenIndex(const glm::ivec3& v) const
{
    return v.x + m_VoxelDim.x * (v.y + m_VoxelDim.y * v.z);
}

glm::ivec3 TiledVoxelWorld::UnflattenIndex(int idx) const
{
    int x = idx % m_VoxelDim.x;
    int yz = idx / m_VoxelDim.x;
    int y = yz % m_VoxelDim.y;
    int z = yz / m_VoxelDim.y;
    return glm::ivec3(x, y, z);
}
