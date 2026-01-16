// Raycast.cpp
#include "Raycast.h"
#include <glm/gtx/component_wise.hpp> // For GLM functions
#include <glm/gtc/type_ptr.hpp> // Additional GLM support if needed
#include <random> // For random offsets in occlusion

#include "../../Physics.h"

#include "../Helpers.h"

#include "../../DebugDraw.hpp"

int GetMinAxis(const vec3& v) {
    int axis = 0;
    if (v.y < v[axis]) axis = 1;
    if (v.z < v[axis]) axis = 2;
    return axis;
}

RayHit CastRay(const FixedVoxelWorld* world, const vec3& start, const vec3& dir, float maxDistance) 
{
    
    auto physHit = Physics::LineTrace(VoxelToWorldPos(start), VoxelToWorldPos(start + normalize(dir) * maxDistance), BodyType::World);

    RayHit rh;

    rh.hit = physHit.hasHit;
    rh.distance = physHit.fraction * maxDistance;
    rh.material = 1;
    rh.normal = physHit.normal;
    rh.pos = WorldToVoxelPos(physHit.position);
    return rh;
    

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
    std::uniform_real_distribution<float> uniform(-1.5f, 1.5f); // Small offsets around source

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

ReverbStats ComputeReverb(
    const FixedVoxelWorld* world,
    const vec3& source,
    const vec3& listener,
    int numRays,
    int maxBounces,
    float maxRayDistance,
    const MaterialProps& props)
{
    ReverbStats stats{};

    // Weighted accumulators
    float weightedDistSum = 0.0f;
    float weightedBounceSum = 0.0f;
    float weightedEnergySum = 0.0f;
    float totalWeight = 0.0f;
    vec3  weightedDirectionSum = vec3(0.0f);

    int validRays = 0;
    float shortestTravelDist = std::numeric_limits<float>::max();

    // RNG (seeded once)
    std::default_random_engine generator;
    std::normal_distribution<float> distribution(0.0f, 1.0f);

    // Parameters for recursive branching:
    // - raysPerBounce: how many rays to spawn at each bounce (user requested 32)
    // - jitterStrength: how much child rays deviate from perfect reflection
    const int raysPerBounce = 1;
    const float jitterStrength = 0.75f; // tweak for roughness of scattering
    const float EPS = 0.001f;
    const float ENERGY_CUTOFF = 0.001f;

    // small reflect helper (in case you don't have glm::reflect)
    auto reflectVec = [](const vec3& v, const vec3& n) -> vec3 {
        return v - 2.0f * dot(v, n) * n;
        };

    // Recursive ray tracer for a single "ray path" (this function traces one ray,
    // then on hit spawns child rays recursively).
    std::function<void(const vec3&, const vec3&, float, int, float)> traceRay;
    traceRay = [&](const vec3& origin, const vec3& direction, float energy, int bounceCount, float traveledDistance) -> void {
        // Limit by remaining distance
        if (traveledDistance >= maxRayDistance) return;

        // Cast from origin along direction, up to remaining distance
        RayHit hit = CastRay(world, origin, direction, maxRayDistance - traveledDistance);
        if (!hit.hit) {
            // Misses the scene entirely
            return;
        }

        // Update travelled distance to the hit
        float newTraveled = traveledDistance + hit.distance;
        float reflectedEnergy = energy * props.GetReflectivity(hit.material);
        if (reflectedEnergy < ENERGY_CUTOFF) {
            // energy too small to matter
            return;
        }

        // small offset to avoid self-intersection on next casts
        vec3 hitPos = hit.pos + EPS * hit.normal;

        // Check LOS from this bounce point to listener (immediate arrival after this bounce)
        vec3 toListener = listener - hitPos;
        float losDist = length(toListener);
        if (losDist > 0.0f && (newTraveled + losDist) <= maxRayDistance) {
            vec3 losDir = toListener / losDist;
            RayHit losHit = CastRay(world, hitPos, losDir, losDist);
            if (!losHit.hit) {
                // Path reached listener
                float totalPathDist = newTraveled + losDist;
                float weight = reflectedEnergy / (totalPathDist * totalPathDist);

                // accumulate weighted stats
                weightedDistSum += totalPathDist * weight;
                weightedBounceSum += static_cast<float>(bounceCount) * weight;
                weightedEnergySum += reflectedEnergy * weight;
                weightedDirectionSum += losDir * weight;
                totalWeight += weight;

                shortestTravelDist = std::min(shortestTravelDist, totalPathDist);
                ++validRays;

                // Note: we DO NOT "return" here - we still allow spawning children if bounceCount < maxBounces.
                // But we do not trace further along this particular ray path to the "same" listener: child rays are additional paths.
            }
        }

        // If we can still bounce further, spawn child rays from the hit point
        if (bounceCount < maxBounces) {
            // Get nominal perfect reflection direction
            vec3 baseReflect = reflectVec(direction, hit.normal);
            // Spawn 'raysPerBounce' children, each with jittered direction
            for (int r = 0; r < raysPerBounce; ++r) {
                // jitter - normal-distributed small vector around 0
                vec3 jitter = vec3(
                    distribution(generator),
                    distribution(generator),
                    distribution(generator)
                );

                // bias jitter to lie mostly in reflection hemisphere:
                if (dot(jitter, hit.normal) < 0.0f) jitter = -jitter;

                vec3 childDir = normalize(baseReflect + jitter * jitterStrength);

                // Recurse: pass updated traveled distance and reflected energy
                traceRay(hitPos, childDir, reflectedEnergy, bounceCount + 1, newTraveled);
            }
        }
        };

    // ---------------------------
    // Direct sound (0 bounces)
    // ---------------------------
    {
        vec3 toL = listener - source;
        float directDist = length(toL);
        if (directDist > 0.0f && directDist <= maxRayDistance) {
            vec3 dir = toL / directDist;
            RayHit hit = CastRay(world, source, dir, directDist);
            if (!hit.hit) {
                float energy = 1.0f;
                float weight = energy / (directDist * directDist);

                weightedDistSum += directDist * weight;
                weightedBounceSum += 0.0f;
                weightedEnergySum += energy * weight;
                weightedDirectionSum += dir * weight;
                totalWeight += weight;

                shortestTravelDist = std::min(shortestTravelDist, directDist);
                ++validRays;
            }
        }
    }

    // ---------------------------
    // Initial sampled rays from source
    // ---------------------------
    for (int i = 0; i < numRays; ++i) {
        // sample roughly-uniform direction with normal distribution and normalize
        vec3 sampleDir = normalize(vec3(
            distribution(generator),
            distribution(generator),
            distribution(generator)
        ));

        // Start a full recursive trace for this ray
        traceRay(source, sampleDir, 1.0f, 1, 0.0f);
    }

    // ---------------------------
    // Final weighted averages
    // ---------------------------
    if (totalWeight > 0.0f) {
        stats.averageDistance = weightedDistSum / totalWeight;
        stats.averageBounces = weightedBounceSum / totalWeight;
        stats.averageEnergy = weightedEnergySum / totalWeight;

        // normalize direction properly (divide by weight first to get mean direction, then normalize)
        vec3 meanDir = weightedDirectionSum / totalWeight;
        if (length(meanDir) > 0.0f) stats.bounceDirection = normalize(meanDir);
        else stats.bounceDirection = vec3(0.0f);

        stats.averageTravelDistance = shortestTravelDist;
    }
    else {
        stats.averageDistance = 0.0f;
        stats.averageBounces = 0.0f;
        stats.averageEnergy = 0.0f;
        stats.bounceDirection = vec3(0.0f);
        stats.averageTravelDistance = std::numeric_limits<float>::infinity();
    }

    // Keep same semantics as before: fraction of initial primary rays that produced at least one path.
    // (If you prefer to use total spawned rays in denominator, I can change this.)
    stats.airspace = static_cast<float>(validRays) / static_cast<float>(numRays);

    return stats;
}



