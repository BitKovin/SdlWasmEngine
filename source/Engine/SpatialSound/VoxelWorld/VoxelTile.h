#pragma once
#include <cstdint>
#include <cstring>
#include "Voxel.h"

constexpr int TILE_SIZE = 32;
constexpr int TILE_VOLUME = TILE_SIZE * TILE_SIZE * TILE_SIZE;

struct VoxelTile
{
    Voxel staticDensity[TILE_VOLUME];
    Voxel currentDensity[TILE_VOLUME];

    bool dirty = false;

    inline static int Index(int x, int y, int z)
    {
        return x + TILE_SIZE * (y + TILE_SIZE * z);
    }

    inline Voxel& Static(int x, int y, int z)
    {
        return staticDensity[Index(x, y, z)];
    }

    inline Voxel& Current(int x, int y, int z)
    {
        return currentDensity[Index(x, y, z)];
    }

    inline void ResetToStatic()
    {
        std::memcpy(currentDensity, staticDensity, TILE_VOLUME);
        dirty = false;
    }
};
