// Raycast.cpp
#include "Raycast.h"
#include <glm/gtx/component_wise.hpp> // For GLM functions
#include <glm/gtc/type_ptr.hpp> // Additional GLM support if needed
#include <random> // For random offsets in occlusion

#include "../Helpers.h"

#include "../../DebugDraw.hpp"

int GetMinAxis(const vec3& v) {
    int axis = 0;
    if (v.y < v[axis]) axis = 1;
    if (v.z < v[axis]) axis = 2;
    return axis;
}

RayHit CastRay(const FixedVoxelWorld* world, const vec3& start, const vec3& dir, float maxDistance) {
    if (length(dir) < 0.001f) return {}; // Invalid dir
    vec3 rayDir = normalize(dir);
    float t = 0.0f;
    ivec3 voxel = floor(start);
    vec3 invDir = 1.0f / rayDir;
    vec3 delta = abs(invDir);
    ivec3 step = sign(rayDir);
    // Initial tMax to next boundaries (fixed for negative directions)
    vec3 floorStart = floor(start);
    vec3 boundary = glm::mix(floorStart, floorStart + vec3(1.0f), glm::greaterThan(step, ivec3(0)));
    vec3 tMax = (boundary - start) * invDir;
    // Handle NaN/Inf if dir component zero (set large tMax)
    if (std::abs(rayDir.x) < 0.0001f) tMax.x = std::numeric_limits<float>::max();
    if (std::abs(rayDir.y) < 0.0001f) tMax.y = std::numeric_limits<float>::max();
    if (std::abs(rayDir.z) < 0.0001f) tMax.z = std::numeric_limits<float>::max();
    while (t < maxDistance) {
        float tMin = std::min(tMax.x, std::min(tMax.y, tMax.z));
        if (tMin > maxDistance) break;
        int axis = GetMinAxis(tMax);
        t = tMin;
        voxel[axis] += step[axis];
        tMax[axis] += delta[axis];
        // Check if solid
        vec3 center = vec3(voxel) + vec3(0.5f);
        uint8_t mat = world->GetValue(center);
        if (mat != 0) { // Solid
            vec3 hitPos = start + t * rayDir;
            vec3 normal(0.0f);
            normal[axis] = -float(step[axis]); // Axis-aligned normal (outward from previous)
            return { true, hitPos, normal, mat, t };
        }
    }
    return {}; // No hit
}

float ComputeOcclusionGain(const FixedVoxelWorld* world, const vec3& source, const vec3& listener, const MaterialProps& props) {
    vec3 deltaPos = listener - source;
    float totalDist = length(deltaPos);
    if (totalDist < 0.001f) return 1.0f; // Same position, no occlusion

    // Multi-ray occlusion (e.g., 9 rays like mod for robustness)
    const int numOcclusionRays = 9;
    float maxGain = 0.0f; // Take the maximum gain (least occluded path)
    std::default_random_engine generator;
    std::uniform_real_distribution<float> uniform(-0.5f, 0.5f); // Small offsets around source

    for (int i = 0; i < numOcclusionRays; ++i) {
        vec3 offset = (i == 0) ? vec3(0.0f) : vec3(uniform(generator), uniform(generator), uniform(generator)) * 0.1f; // Central + random small offsets
        vec3 offsetSource = source + offset;
        vec3 offsetDelta = listener - offsetSource;
        float offsetDist = length(offsetDelta);
        vec3 rayDir = offsetDelta / offsetDist;
        float occlusionAccum = 0.0f;
        float t = 0.0f;
        ivec3 voxel = floor(offsetSource);
        vec3 invDir = 1.0f / rayDir;
        vec3 delta = abs(invDir);
        ivec3 step = sign(rayDir);
        vec3 floorSource = floor(offsetSource);
        vec3 boundary = glm::mix(floorSource, floorSource + vec3(1.0f), glm::greaterThan(step, ivec3(0)));
        vec3 tMax = (boundary - offsetSource) * invDir;
        if (std::abs(rayDir.x) < 0.0001f) tMax.x = std::numeric_limits<float>::max();
        if (std::abs(rayDir.y) < 0.0001f) tMax.y = std::numeric_limits<float>::max();
        if (std::abs(rayDir.z) < 0.0001f) tMax.z = std::numeric_limits<float>::max();
        float prevT = 0.0f;
        while (t < offsetDist) {
            float tMin = std::min(tMax.x, std::min(tMax.y, tMax.z));
            if (tMin > offsetDist) break;
            int axis = GetMinAxis(tMax);
            t = tMin;
            voxel[axis] += step[axis];
            tMax[axis] += delta[axis];
            // If out of bounds, full occlusion for this ray (if treating bounds as absorbing)
            if (!world->inBounds(voxel)) {
                occlusionAccum = 10.0f; // High accum to zero gain
                break;
            }
            vec3 center = vec3(voxel) + vec3(0.5f);
            uint8_t mat = world->GetValue(center);
            if (mat != 0) {
                float segmentLength = t - prevT;
                occlusionAccum += props.GetAbsorption(mat) * segmentLength;
                if (occlusionAccum > 10.0f) break;
            }
            prevT = t;
        }
        float gain = std::exp(-occlusionAccum);
        if (gain > maxGain) maxGain = gain;
    }
    return maxGain;
}

ReverbStats ComputeReverb(const FixedVoxelWorld* world, const vec3& source, const vec3& listener, int numRays, int maxBounces, float maxRayDistance, const MaterialProps& props) {
    ReverbStats stats;
    float totalDist = 0.0f;
    float totalEnergy = 0.0f;
    float totalBounces = 0.0f;
    vec3 totalDirections = vec3();
    int validRays = 0;
    std::default_random_engine generator;
    std::normal_distribution<float> distribution(0.0f, 1.0f);

    float traveledDistance = 0;

    auto hit = CastRay(world, source, listener, maxRayDistance);

    if (!hit.hit)
    {
        totalDirections += (listener - source) / hit.distance * maxRayDistance;
    }

    for (int i = 0; i < numRays; ++i) {
        // Random direction (approximate uniform on sphere)
        vec3 rayDir = normalize(vec3(distribution(generator), distribution(generator), distribution(generator)));
        float energy = 1.0f;
        float rayDist = 0.0f;
        vec3 currentPos = source;
        int bounceCount = 0;
        bool hadBounce = false;
        for (int b = 0; b < maxBounces; ++b) {
            RayHit hit = CastRay(world, currentPos, rayDir, maxRayDistance - rayDist);
            if (!hit.hit) break; // Miss: break without *= (path to infinity, not absorbed)
            energy *= props.GetReflectivity(hit.material);
            if (energy < 0.001f) break; // Low energy: break (absorbed)
            rayDist += hit.distance;
            currentPos = hit.pos + 0.001f * hit.normal; // Epsilon offset
            // Pseudo-reflect (flip component, like mod)
            int axis = 0;
            if (std::abs(hit.normal.y) > 0.5f) axis = 1;
            else if (std::abs(hit.normal.z) > 0.5f) axis = 2;
            rayDir[axis] = -rayDir[axis];
            ++bounceCount;
            hadBounce = true;
        }
        if (hadBounce && energy >= 0.001f) {
            // Final LOS check to listener (approximate original's shared airspace)
            vec3 losDir = listener - currentPos;
            float losDist = length(losDir);
            RayHit losHit = CastRay(world, currentPos, losDir, losDist);
            if (!losHit.hit) { // Clear path to listener
                totalDist += rayDist + losDist; // Fixed: include the final segment distance in the total path length
                totalEnergy += energy;
                totalBounces += static_cast<float>(bounceCount);

                //DebugDraw::Line(VoxelToWorldPos(currentPos), VoxelToWorldPos(source), 0.002f, 0.01f);
                //DebugDraw::Line(VoxelToWorldPos(currentPos), VoxelToWorldPos(listener - vec3(0,2.5f,0)), 0.002f);

                totalDirections += normalize(losDir) / (rayDist * rayDist * rayDist) / ((float)(bounceCount* bounceCount* bounceCount));
                ++validRays;
            }
        }
    }
    if (validRays > 0) {
        stats.averageDistance = totalDist / static_cast<float>(validRays);
        stats.averageEnergy = totalEnergy / static_cast<float>(validRays);
        stats.averageBounces = totalBounces / static_cast<float>(validRays);
        stats.bounceDirection = normalize(totalDirections / static_cast<float>(validRays));
    }
    stats.airspace = static_cast<float>(validRays) / static_cast<float>(numRays);
    return stats;
}