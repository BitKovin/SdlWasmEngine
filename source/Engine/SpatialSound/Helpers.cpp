// Helpers.cpp
#include "Helpers.h"
#include "SpatialSoundManager.h"

vec3 WorldToVoxelPos(const vec3& worldPos) {
    return (worldPos - SpatialSoundManager::worldMin) / SpatialSoundManager::voxelSize;
}

vec3 VoxelToWorldPos(const vec3& voxelPos) {
    return voxelPos * SpatialSoundManager::voxelSize + SpatialSoundManager::worldMin;
}