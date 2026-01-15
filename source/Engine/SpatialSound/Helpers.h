#pragma once

#include <glm/glm.hpp>

using namespace glm;

vec3 WorldToVoxelPos(const vec3& worldPos);

vec3 VoxelToWorldPos(const vec3& voxelPos);