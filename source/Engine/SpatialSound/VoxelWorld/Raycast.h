#pragma once

#include "../BVH/BVH.h"
#include "MaterialProps.h"
#include "../../glm.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <random>

using namespace glm;


// Reverb stats (aggregated from rays, similar to mod's average energy decay for EFX params)
struct ReverbStats {
    float averageDistance = 0.0f;
    float averageEnergy = 0.0f;
    float averageBounces = 0;
    float airspace = 0;
    vec3 bounceDirection = {};
};

// Helper to get argmin axis
int GetMinAxis(const vec3& v);

// Ray casting function (DDA voxel traversal, stops at first solid hit)
RayHit CastRay(BVH* world, const vec3& start, const vec3& dir, float maxDistance);

// Compute direct path occlusion gain
float ComputeOcclusionGain(BVH* world, const vec3& source, const vec3& listener, const MaterialProps& props);

// Compute reverb stats
ReverbStats ComputeReverb(BVH* world, const vec3& source, const vec3& listener, int numRays, int maxBounces, float maxRayDistance, const MaterialProps& props);