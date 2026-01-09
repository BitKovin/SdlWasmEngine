#include "SoundPathfindingQuery.h"
#include <queue>
#include <limits>
#include <cmath>
#include <algorithm>
#include <unordered_set>
#include <random>
#include <numeric>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/vector_query.hpp>

// Precomputed sampling directions (hemisphere + cardinal)
const glm::vec3 SoundPathfindingQuery::SAMPLE_DIRECTIONS[32] = {
    // Cardinal directions
    {1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}, {0, 0, 1}, {0, 0, -1},

    // Diagonal directions (8 corners)
    {1, 1, 1}, {1, 1, -1}, {1, -1, 1}, {1, -1, -1},
    {-1, 1, 1}, {-1, 1, -1}, {-1, -1, 1}, {-1, -1, -1},

    // Edge centers (12 edges)
    {1, 1, 0}, {1, -1, 0}, {-1, 1, 0}, {-1, -1, 0},
    {1, 0, 1}, {1, 0, -1}, {-1, 0, 1}, {-1, 0, -1},
    {0, 1, 1}, {0, 1, -1}, {0, -1, 1}, {0, -1, -1},

    // Additional samples for better coverage
    {0.707f, 0.354f, 0}, {0.354f, 0.707f, 0}, {0, 0.707f, 0.354f},
    {0, 0.354f, 0.707f}, {0.707f, 0, 0.354f}, {0.354f, 0, 0.707f}
};

SoundPathfindingQuery::SoundPathfindingQuery(const TiledVoxelWorld& world)
    : m_World(world)
{
    // Initialize random seed
    srand(static_cast<unsigned int>(time(nullptr)));

    // Default material properties based on density
    // Air/Vacuum
    SetMaterialProperties(0, 30, { 0.001f, 343.0f, 0.01f, 0.1f, 0.9f, 0.0f });
    // Light materials (wood, fabric)
    SetMaterialProperties(31, 100, { 0.5f, 2000.0f, 0.2f, 0.6f, 0.4f, 0.3f });
    // Medium materials (concrete, brick)
    SetMaterialProperties(101, 180, { 2.4f, 3500.0f, 0.4f, 0.8f, 0.2f, 0.2f });
    // Dense materials (metal, stone)
    SetMaterialProperties(181, 255, { 7.8f, 5000.0f, 0.6f, 0.9f, 0.1f, 0.1f });
}

SoundPathResult SoundPathfindingQuery::FindSoundPaths(
    const glm::vec3& sourceWorld,
    const glm::vec3& listenerWorld,
    const SoundPropagationConfig& config)
{
    SoundPathResult result;

    // Calculate direction to actual source
    glm::vec3 toSource = sourceWorld - listenerWorld;
    if (glm::length(toSource) > 0.001f) {
        result.sourceDirection = glm::normalize(toSource);
    }

    // Check if listener is within sound radius
    float sourceDistance = glm::distance(sourceWorld, listenerWorld);
    if (sourceDistance > config.maxSearchRadius) {
        return result; // Empty result
    }

    // Clear cache for new query
    m_AttenuationCache.clear();
    m_RayCache.clear();

    // Find direct path
    SoundPath directPath = FindDirectPath(sourceWorld, listenerWorld, config);
    if (directPath.attenuation > 0.001f) {
        result.paths.push_back(directPath);
        // Store arrival direction with weight
        result.arrivalDirections.push_back(directPath.arrivalDirection);
        result.directionalWeights.push_back(directPath.attenuation);
    }

    // Find reflected paths if we have bounces
    if (config.maxBounces > 0) {
        std::vector<SoundPath> reflectedPaths = FindReflectedPaths(
            sourceWorld, listenerWorld, config);

        // Add only the best reflected paths
        int maxReflected = std::min((int)reflectedPaths.size(),
            config.maxPathCount / 2);
        for (int i = 0; i < maxReflected; i++) {
            result.paths.push_back(reflectedPaths[i]);
            // Store arrival direction with weight
            result.arrivalDirections.push_back(reflectedPaths[i].arrivalDirection);
            result.directionalWeights.push_back(reflectedPaths[i].attenuation);
        }
    }

    // Find diffracted paths if enabled
    if (config.enableDiffraction && config.maxPathCount > result.paths.size()) {
        std::vector<SoundPath> diffractedPaths = FindDiffractedPaths(
            sourceWorld, listenerWorld, config);

        int remaining = config.maxPathCount - result.paths.size();
        int maxDiffracted = std::min((int)diffractedPaths.size(), remaining);
        for (int i = 0; i < maxDiffracted; i++) {
            result.paths.push_back(diffractedPaths[i]);
            // Store arrival direction with weight
            result.arrivalDirections.push_back(diffractedPaths[i].arrivalDirection);
            result.directionalWeights.push_back(diffractedPaths[i].attenuation);
        }
    }

    // Sort paths by attenuation (best first)
    std::sort(result.paths.begin(), result.paths.end(),
        [](const SoundPath& a, const SoundPath& b) {
            return a.attenuation > b.attenuation;
        });

    // Limit to maxPathCount
    if (result.paths.size() > config.maxPathCount) {
        result.paths.resize(config.maxPathCount);
        result.arrivalDirections.resize(config.maxPathCount);
        result.directionalWeights.resize(config.maxPathCount);
    }

    // Calculate total result metrics
    float totalIntensity = 0.0f;
    float totalTimeWeighted = 0.0f;
    float timeWeightSum = 0.0f;
    float directIntensity = 0.0f;
    float reflectedIntensity = 0.0f;

    // Calculate weighted dominant direction based on ALL arrival directions
    glm::vec3 weightedDirection(0);
    float directionWeightSum = 0.0f;

    for (size_t i = 0; i < result.paths.size(); i++) {
        const auto& path = result.paths[i];
        float intensity = path.intensity * path.attenuation;
        totalIntensity += intensity;

        if (path.type == SoundPathType::DIRECT) {
            directIntensity += intensity;
        }
        else {
            reflectedIntensity += intensity;
        }

        // Weight direction by intensity AND arrival time
        // Early arrivals have more influence on perceived direction
        float timeWeight = std::exp(-path.arrivalTime * 2.0f); // Exponential decay for late arrivals
        float directionalWeight = intensity * timeWeight;

        if (glm::length(path.arrivalDirection) > 0.001f) {
            weightedDirection += path.arrivalDirection * directionalWeight;
            directionWeightSum += directionalWeight;
        }

        // Weight time by intensity
        totalTimeWeighted += path.arrivalTime * intensity;
        timeWeightSum += intensity;
    }

    result.totalIntensity = totalIntensity;
    result.averageDelay = timeWeightSum > 0 ? totalTimeWeighted / timeWeightSum : 0.0f;

    // Calculate spatial spread of sound directions
    result.spatialSpread = CalculateSpatialSpread(result.arrivalDirections, result.directionalWeights);

    // Normalize dominant direction if we have weights
    if (directionWeightSum > 0.001f) {
        weightedDirection /= directionWeightSum;
        if (glm::length(weightedDirection) > 0.001f) {
            result.dominantDirection = glm::normalize(weightedDirection);

            // Calculate directional certainty
            // Lower spread = more certainty
            result.directionalCertainty = glm::clamp(1.0f - result.spatialSpread, 0.0f, 1.0f);

            // Boost certainty if direct path is strong
            if (directIntensity > 0) {
                float directRatio = directIntensity / totalIntensity;
                result.directionalCertainty = glm::mix(
                    result.directionalCertainty,
                    1.0f, // High certainty for direct sound
                    glm::clamp(directRatio * 2.0f, 0.0f, 1.0f)
                );
            }
        }
    }

    // Calculate difference between perceived direction and actual source direction
    if (glm::length(result.dominantDirection) > 0.001f &&
        glm::length(result.sourceDirection) > 0.001f) {
        float directionError = glm::angle(result.dominantDirection, result.sourceDirection);
        // Convert to 0-1 scale (0° = 1.0, 180° = 0.0)
        result.directionalCertainty *= (1.0f - directionError / glm::pi<float>());
    }

    // Analyze direction clusters for better accuracy
    AnalyzeDirectionClusters(result);

    if (reflectedIntensity > 0) {
        result.directToReverberantRatio = directIntensity / reflectedIntensity;
    }
    else if (directIntensity > 0) {
        result.directToReverberantRatio = 100.0f; // Only direct sound
    }

    return result;
}

float SoundPathfindingQuery::CheckLineOfSight(
    const glm::vec3& sourceWorld,
    const glm::vec3& listenerWorld,
    const SoundPropagationConfig& config)
{
    RayCastResult ray = CastSoundRay(sourceWorld, listenerWorld, config, false);
    return ray.totalAttenuation;
}

std::vector<SoundPathResult> SoundPathfindingQuery::FindSoundPathsToMultiple(
    const glm::vec3& sourceWorld,
    const std::vector<glm::vec3>& listenerWorlds,
    const SoundPropagationConfig& config)
{
    std::vector<SoundPathResult> results;
    results.reserve(listenerWorlds.size());

    for (const auto& listener : listenerWorlds) {
        results.push_back(FindSoundPaths(sourceWorld, listener, config));
    }

    return results;
}

void SoundPathfindingQuery::SetMaterialProperties(
    uint8_t minDensity,
    uint8_t maxDensity,
    const VoxelAcousticProperties& props)
{
    m_MaterialMap.push_back({ {minDensity, maxDensity}, props });
}

VoxelAcousticProperties SoundPathfindingQuery::GetVoxelProperties(const glm::ivec3& voxel) const
{
    uint8_t density = m_World.GetVoxelDensityAt(voxel);

    // Find matching material range
    for (const auto& entry : m_MaterialMap) {
        if (density >= entry.first.first && density <= entry.first.second) {
            return entry.second;
        }
    }

    // Default properties for unknown density
    return { 1.0f, 343.0f, 0.5f, 0.5f, 0.5f, 0.5f };
}

// Private implementation

SoundPath SoundPathfindingQuery::FindDirectPath(
    const glm::vec3& source,
    const glm::vec3& listener,
    const SoundPropagationConfig& config)
{
    SoundPath path;
    path.type = SoundPathType::DIRECT;

    RayCastResult ray = CastSoundRay(source, listener, config, true);

    if (ray.blocked) {
        path.attenuation = 0.0f;
        path.occluded = true;
        return path;
    }

    path.waypoints = { source, listener };
    path.totalDistance = glm::distance(source, listener);
    path.attenuation = ray.totalAttenuation;
    path.arrivalTime = ray.travelTime;
    path.arrivalDirection = glm::normalize(listener - source);
    path.intensity = config.soundIntensity;

    // Apply source radius effect (sound spreading)
    if (config.sourceRadius > 0) {
        float spreading = 1.0f / (1.0f + path.totalDistance / config.sourceRadius);
        path.attenuation *= spreading;
    }

    return path;
}

std::vector<SoundPath> SoundPathfindingQuery::FindReflectedPaths(
    const glm::vec3& source,
    const glm::vec3& listener,
    const SoundPropagationConfig& config,
    int currentBounce)
{
    std::vector<SoundPath> reflectedPaths;

    if (currentBounce >= config.maxBounces) {
        return reflectedPaths;
    }

    // Find potential reflection surfaces
    std::vector<ReflectionSurface> surfaces = FindReflectionSurfaces(
        source, listener, config);

    for (const auto& surface : surfaces) {
        // Skip surfaces with poor reflection
        if (surface.reflectionCoeff < 0.1f) continue;

        // Calculate image source (virtual source behind the wall)
        glm::vec3 imageSource = CalculateImageSource(
            source, surface.point, surface.normal);

        // Path from source to reflection point
        RayCastResult toSurface = CastSoundRay(source, surface.point, config, false);
        if (toSurface.blocked || toSurface.totalAttenuation < 0.01f) continue;

        // Path from reflection point to listener
        RayCastResult fromSurface = CastSoundRay(surface.point, listener, config, false);
        if (fromSurface.blocked || fromSurface.totalAttenuation < 0.01f) continue;

        // Create reflected path
        SoundPath reflectedPath;
        reflectedPath.type = SoundPathType::REFLECTED;
        reflectedPath.waypoints = { source, surface.point, listener };
        reflectedPath.totalDistance = toSurface.travelTime * SPEED_OF_SOUND +
            fromSurface.travelTime * SPEED_OF_SOUND;

        // Combined attenuation with reflection coefficient
        reflectedPath.attenuation = toSurface.totalAttenuation *
            fromSurface.totalAttenuation *
            surface.reflectionCoeff;

        // Apply roughness scattering - spreads the direction
        float roughnessFactor = 1.0f - surface.roughness * 0.5f;
        reflectedPath.attenuation *= roughnessFactor;

        reflectedPath.arrivalTime = toSurface.travelTime + fromSurface.travelTime;

        // CRITICAL FIX: Use direction from image source to listener
        // This makes the sound appear to come from the wall, not the actual source
        glm::vec3 imageToListener = listener - imageSource;
        if (glm::length(imageToListener) > 0.001f) {
            reflectedPath.arrivalDirection = glm::normalize(imageToListener);
        }
        else {
            reflectedPath.arrivalDirection = glm::normalize(listener - surface.point);
        }

        // Alternative: For specular reflections, the arrival direction should be
        // the mirror of the source direction across the surface normal
        // glm::vec3 incidentDir = glm::normalize(surface.point - source);
        // reflectedPath.arrivalDirection = glm::reflect(incidentDir, surface.normal);

        reflectedPath.intensity = config.soundIntensity;

        // Apply directional spreading based on surface roughness
        if (surface.roughness > 0.1f) {
            // Add some random direction variation for rough surfaces
            glm::vec3 randomVariation(
                (rand() / (float)RAND_MAX - 0.5f) * surface.roughness,
                (rand() / (float)RAND_MAX - 0.5f) * surface.roughness,
                (rand() / (float)RAND_MAX - 0.5f) * surface.roughness
            );
            reflectedPath.arrivalDirection += randomVariation;
            if (glm::length(reflectedPath.arrivalDirection) > 0.001f) {
                reflectedPath.arrivalDirection = glm::normalize(reflectedPath.arrivalDirection);
            }
        }

        if (reflectedPath.attenuation > 0.01f) {
            reflectedPaths.push_back(reflectedPath);
        }

        // Limit number of reflections
        if (reflectedPaths.size() >= 5) break;
    }

    // Sort by best attenuation
    std::sort(reflectedPaths.begin(), reflectedPaths.end(),
        [](const SoundPath& a, const SoundPath& b) {
            return a.attenuation > b.attenuation;
        });

    return reflectedPaths;
}

std::vector<SoundPath> SoundPathfindingQuery::FindDiffractedPaths(
    const glm::vec3& source,
    const glm::vec3& listener,
    const SoundPropagationConfig& config)
{
    std::vector<SoundPath> diffractedPaths;

    if (!config.enableDiffraction) {
        return diffractedPaths;
    }

    std::vector<DiffractionEdge> edges = FindDiffractionEdges(
        source, listener, config);

    for (const auto& edge : edges) {
        if (edge.sharpness < config.diffractionThreshold) continue;

        // Calculate diffraction attenuation
        float diffAtten = CalculateDiffractionAttenuation(
            edge, source, listener, config);

        if (diffAtten < 0.01f) continue;

        // Path from source to edge
        RayCastResult toEdge = CastSoundRay(source, edge.midpoint, config, false);
        if (toEdge.blocked || toEdge.totalAttenuation < 0.01f) continue;

        // Path from edge to listener
        RayCastResult fromEdge = CastSoundRay(edge.midpoint, listener, config, false);
        if (fromEdge.blocked || fromEdge.totalAttenuation < 0.01f) continue;

        // Create diffracted path
        SoundPath diffractedPath;
        diffractedPath.type = SoundPathType::DIFFRACTED;
        diffractedPath.waypoints = { source, edge.midpoint, listener };
        diffractedPath.totalDistance = toEdge.travelTime * SPEED_OF_SOUND +
            fromEdge.travelTime * SPEED_OF_SOUND;

        diffractedPath.attenuation = toEdge.totalAttenuation *
            fromEdge.totalAttenuation *
            diffAtten;

        // Additional high-frequency loss for diffraction
        if (config.frequency > 2000.0f) {
            float hfLoss = 1.0f - (config.frequency - 2000.0f) / 10000.0f;
            diffractedPath.attenuation *= glm::clamp(hfLoss, 0.1f, 1.0f);
        }

        diffractedPath.arrivalTime = toEdge.travelTime + fromEdge.travelTime;

        // For diffraction, sound appears to come from the edge
        // But we can bias it toward the source direction for small angles
        glm::vec3 sourceToEdge = edge.midpoint - source;
        glm::vec3 edgeToListener = listener - edge.midpoint;

        float sourceAngle = glm::angle(glm::normalize(sourceToEdge),
            glm::normalize(edge.normal));
        float listenerAngle = glm::angle(glm::normalize(-edgeToListener),
            glm::normalize(edge.normal));

        // Blend between edge direction and source direction
        // When angles are small, sound appears more from the source direction
        float blendFactor = glm::clamp(1.0f - (sourceAngle + listenerAngle) / glm::pi<float>(), 0.0f, 1.0f);

        if (blendFactor > 0.5f) {
            // More like direct sound from source
            diffractedPath.arrivalDirection = glm::normalize(listener - source);
        }
        else {
            // More like sound from edge
            diffractedPath.arrivalDirection = glm::normalize(edgeToListener);
        }

        // Add some spread based on edge sharpness
        float spread = 1.0f - edge.sharpness;
        glm::vec3 spreadOffset(
            (rand() / (float)RAND_MAX - 0.5f) * spread * 0.2f,
            (rand() / (float)RAND_MAX - 0.5f) * spread * 0.2f,
            (rand() / (float)RAND_MAX - 0.5f) * spread * 0.2f
        );
        diffractedPath.arrivalDirection += spreadOffset;
        if (glm::length(diffractedPath.arrivalDirection) > 0.001f) {
            diffractedPath.arrivalDirection = glm::normalize(diffractedPath.arrivalDirection);
        }

        diffractedPath.intensity = config.soundIntensity;

        if (diffractedPath.attenuation > 0.01f) {
            diffractedPaths.push_back(diffractedPath);
        }

        if (diffractedPaths.size() >= 3) break;
    }

    return diffractedPaths;
}

SoundPathfindingQuery::RayCastResult SoundPathfindingQuery::CastSoundRay(
    const glm::vec3& start,
    const glm::vec3& end,
    const SoundPropagationConfig& config,
    bool collectDetails)
{
    // Check cache first
    uint64_t cacheKey = GetCacheKey(start, end, config.frequency);
    auto cached = m_RayCache.find(cacheKey);
    if (cached != m_RayCache.end()) {
        return cached->second;
    }

    RayCastResult result;
    float totalDistance = glm::distance(start, end);

    // If distance exceeds max search radius, ray is effectively blocked
    if (totalDistance > config.maxSearchRadius) {
        result.blocked = true;
        result.totalAttenuation = 0.0f;
        m_RayCache[cacheKey] = result;
        return result;
    }

    result.travelTime = totalDistance / SPEED_OF_SOUND;

    // Sample along ray at voxel-sized intervals
    glm::vec3 direction = glm::normalize(end - start);
    float stepSize = m_World.GetVoxelSize() * 0.25f; // Quarter voxel for accuracy
    int steps = static_cast<int>(totalDistance / stepSize) + 1;

    glm::vec3 current = start;
    glm::ivec3 lastVoxel;
    bool first = true;
    float accumulatedAttenuation = 1.0f;
    std::vector<glm::ivec3> traversedVoxels;

    for (int i = 0; i <= steps; i++) {
        glm::ivec3 voxel;
        if (!m_World.WorldToVoxelSafe(current, voxel)) {
            // Out of bounds - treat as blocked
            result.blocked = true;
            break;
        }

        // Check occlusion
        if (CheckVoxelOcclusion(voxel, config)) {
            result.blocked = true;
            break;
        }

        if (collectDetails && (!first || i == 0)) {
            if (first || voxel != lastVoxel) {
                result.samplePoints.push_back(current);
                traversedVoxels.push_back(voxel);

                // Calculate attenuation for this segment
                VoxelAcousticProperties props = GetVoxelProperties(voxel);
                float materialAtten = std::exp(-props.absorptionCoeff * stepSize);
                accumulatedAttenuation *= materialAtten;

                if (collectDetails) {
                    result.attenuationPerSegment.push_back(materialAtten);
                }
            }
        }

        lastVoxel = voxel;
        first = false;
        current += direction * stepSize;

        // Check if we've reached the end
        if (glm::distance(start, current) >= totalDistance) {
            if (collectDetails) {
                result.samplePoints.push_back(end);
            }
            result.exitPoint = end;
            break;
        }
    }

    if (!result.blocked) {
        // Apply distance attenuation
        float distAtten = CalculateDistanceAttenuation(totalDistance, config);

        // Apply air absorption
        float airAtten = CalculateAirAbsorption(totalDistance, config);

        result.totalAttenuation = accumulatedAttenuation * distAtten * airAtten;
        result.traversedVoxels = traversedVoxels;
    }
    else {
        result.totalAttenuation = 0.0f;
    }

    // Cache the result
    m_RayCache[cacheKey] = result;

    return result;
}

float SoundPathfindingQuery::CalculateDistanceAttenuation(
    float distance, const SoundPropagationConfig& config)
{
    // Inverse square law with near-field correction
    float nearField = config.sourceRadius * 2.0f;
    if (distance < nearField) {
        // In near field, attenuation is less severe
        return 1.0f / (1.0f + (distance * distance) / (nearField * nearField));
    }
    else {
        // Far field: inverse square law
        return 1.0f / (1.0f + distance * distance);
    }
}

float SoundPathfindingQuery::CalculateMaterialAttenuation(
    const std::vector<glm::ivec3>& voxels,
    const SoundPropagationConfig& config)
{
    if (voxels.empty()) return 1.0f;

    float totalAttenuation = 1.0f;
    float voxelSize = m_World.GetVoxelSize();

    for (const auto& voxel : voxels) {
        VoxelAcousticProperties props = GetVoxelProperties(voxel);
        float atten = std::exp(-props.absorptionCoeff * voxelSize *
            config.materialAbsorptionScale);
        totalAttenuation *= atten;
    }

    return totalAttenuation;
}

float SoundPathfindingQuery::CalculateAirAbsorption(
    float distance, const SoundPropagationConfig& config)
{
    // Frequency-dependent air absorption
    // Simplified model: higher frequencies absorb more
    float freqFactor = config.frequency / 1000.0f;
    float absorptionDB = config.airAbsorptionCoeff * distance * freqFactor;

    // Convert from dB to linear scale
    // dB = 20 * log10(amplitude_ratio)
    return std::pow(10.0f, -absorptionDB / 20.0f);
}

float SoundPathfindingQuery::CalculateSpreadingAttenuation(
    const glm::vec3& sourceDir,
    const glm::vec3& pathDir)
{
    if (glm::length(sourceDir) < 0.001f) {
        return 1.0f; // Omnidirectional source
    }

    float dot = glm::dot(glm::normalize(sourceDir), glm::normalize(pathDir));
    // Map from [-1, 1] to [0, 1] with bias toward front
    return 0.3f + 0.7f * (dot + 1.0f) * 0.5f;
}

std::vector<SoundPathfindingQuery::ReflectionSurface>
SoundPathfindingQuery::FindReflectionSurfaces(
    const glm::vec3& source,
    const glm::vec3& listener,
    const SoundPropagationConfig& config)
{
    std::vector<ReflectionSurface> surfaces;

    // Search for surfaces along the line between source and listener
    glm::vec3 midpoint = (source + listener) * 0.5f;
    float searchRadius = glm::distance(source, listener) * 0.5f;

    // Convert to voxel coordinates
    glm::ivec3 centerVoxel;
    if (!m_World.WorldToVoxelSafe(midpoint, centerVoxel)) {
        return surfaces;
    }

    int searchVoxels = static_cast<int>(searchRadius / m_World.GetVoxelSize()) + 2;

    const glm::ivec3 dims = m_World.VoxelDimensions();

    // Check voxels in search area
    for (int dx = -searchVoxels; dx <= searchVoxels; dx++) {
        for (int dy = -searchVoxels; dy <= searchVoxels; dy++) {
            for (int dz = -searchVoxels; dz <= searchVoxels; dz++) {
                glm::ivec3 voxel = centerVoxel + glm::ivec3(dx, dy, dz);

                // Check bounds
                if (voxel.x < 0 || voxel.y < 0 || voxel.z < 0 ||
                    voxel.x >= dims.x || voxel.y >= dims.y || voxel.z >= dims.z) {
                    continue;
                }

                uint8_t density = m_World.GetVoxelDensityAt(voxel);
                if (density < 100) continue; // Skip low-density materials

                // Check if this is a surface (has at least one empty neighbor)
                bool isSurface = false;
                glm::vec3 normal(0);
                int emptyNeighbors = 0;

                // Check 6-connected neighbors
                glm::ivec3 neighbors[6] = {
                    {1, 0, 0}, {-1, 0, 0},
                    {0, 1, 0}, {0, -1, 0},
                    {0, 0, 1}, {0, 0, -1}
                };

                for (int i = 0; i < 6; i++) {
                    glm::ivec3 nb = voxel + neighbors[i];
                    if (nb.x >= 0 && nb.y >= 0 && nb.z >= 0 &&
                        nb.x < dims.x && nb.y < dims.y && nb.z < dims.z) {
                        uint8_t nbDensity = m_World.GetVoxelDensityAt(nb);
                        if (nbDensity < 50) { // Empty or very low density
                            isSurface = true;
                            normal.x -= neighbors[i].x;
                            normal.y -= neighbors[i].y;
                            normal.z -= neighbors[i].z;
                            emptyNeighbors++;
                        }
                    }
                }

                if (isSurface && emptyNeighbors > 0) {
                    normal = glm::normalize(normal);
                    glm::vec3 surfacePoint = m_World.VoxelCenterWorldPublic(voxel);

                    // Adjust surface point to be on the surface in direction of normal
                    surfacePoint += normal * m_World.GetVoxelSize() * 0.5f;

                    VoxelAcousticProperties props = GetVoxelProperties(voxel);

                    ReflectionSurface surface;
                    surface.point = surfacePoint;
                    surface.normal = normal;
                    surface.voxel = voxel;
                    surface.roughness = props.roughness;
                    surface.reflectionCoeff = props.reflectionCoeff;

                    surfaces.push_back(surface);

                    if (surfaces.size() >= 10) {
                        return surfaces; // Limit for performance
                    }
                }
            }
        }
    }

    return surfaces;
}

glm::vec3 SoundPathfindingQuery::CalculateImageSource(
    const glm::vec3& source,
    const glm::vec3& surfacePoint,
    const glm::vec3& surfaceNormal)
{
    // Reflect source across the plane defined by surface point and normal
    glm::vec3 v = source - surfacePoint;
    float d = glm::dot(v, surfaceNormal);
    return source - 2.0f * d * surfaceNormal;
}

std::vector<SoundPathfindingQuery::DiffractionEdge>
SoundPathfindingQuery::FindDiffractionEdges(
    const glm::vec3& source,
    const glm::vec3& listener,
    const SoundPropagationConfig& config)
{
    std::vector<DiffractionEdge> edges;

    // Cast a ray and look for density transitions
    std::vector<glm::ivec3> voxels = GetVoxelsAlongRay(source, listener);

    if (voxels.size() < 2) return edges;

    for (size_t i = 1; i < voxels.size(); i++) {
        glm::ivec3 prevVoxel = voxels[i - 1];
        glm::ivec3 currVoxel = voxels[i];

        uint8_t prevDensity = m_World.GetVoxelDensityAt(prevVoxel);
        uint8_t currDensity = m_World.GetVoxelDensityAt(currVoxel);

        // Look for strong density transitions
        float densityDiff = std::abs(static_cast<float>(currDensity) -
            static_cast<float>(prevDensity)) / 255.0f;

        if (densityDiff > 0.3f) { // Significant transition
            glm::vec3 prevPos = m_World.VoxelCenterWorldPublic(prevVoxel);
            glm::vec3 currPos = m_World.VoxelCenterWorldPublic(currVoxel);

            // Estimate edge normal (perpendicular to ray direction)
            glm::vec3 rayDir = glm::normalize(listener - source);
            glm::vec3 edgeNormal;

            // Find a direction perpendicular to rayDir
            if (std::abs(rayDir.x) > std::abs(rayDir.y)) {
                edgeNormal = glm::normalize(glm::vec3(-rayDir.z, 0, rayDir.x));
            }
            else {
                edgeNormal = glm::normalize(glm::vec3(0, -rayDir.z, rayDir.y));
            }

            DiffractionEdge edge;
            edge.point1 = prevPos;
            edge.point2 = currPos;
            edge.midpoint = (prevPos + currPos) * 0.5f;
            edge.normal = edgeNormal;
            edge.sharpness = densityDiff;

            edges.push_back(edge);

            if (edges.size() >= 5) break; // Limit
        }
    }

    return edges;
}

float SoundPathfindingQuery::CalculateDiffractionAttenuation(
    const DiffractionEdge& edge,
    const glm::vec3& source,
    const glm::vec3& listener,
    const SoundPropagationConfig& config)
{
    // Simplified diffraction model
    glm::vec3 toEdge = edge.midpoint - source;
    glm::vec3 fromEdge = listener - edge.midpoint;

    float angleIn = glm::angle(glm::normalize(-toEdge), edge.normal);
    float angleOut = glm::angle(glm::normalize(fromEdge), edge.normal);

    // Diffraction is strongest when edge is sharp and path bends around it
    float bendAngle = std::abs(angleIn + angleOut - glm::pi<float>());
    float bendFactor = 1.0f - glm::clamp(bendAngle / glm::pi<float>(), 0.0f, 1.0f);

    // Frequency dependence: higher frequencies diffract less
    float freqFactor = 1.0f / (1.0f + config.frequency / 4000.0f);

    // Combined diffraction coefficient
    float diffAtten = edge.sharpness * bendFactor * freqFactor * 0.5f;

    return glm::clamp(diffAtten, 0.0f, 1.0f);
}

std::vector<glm::vec3> SoundPathfindingQuery::GetVoxelSurfacePoints(const glm::ivec3& voxel) const
{
    std::vector<glm::vec3> points;
    glm::vec3 center = m_World.VoxelCenterWorldPublic(voxel);
    float halfSize = m_World.GetVoxelSize() * 0.5f;

    // 6 face centers
    points.push_back(center + glm::vec3(halfSize, 0, 0));
    points.push_back(center + glm::vec3(-halfSize, 0, 0));
    points.push_back(center + glm::vec3(0, halfSize, 0));
    points.push_back(center + glm::vec3(0, -halfSize, 0));
    points.push_back(center + glm::vec3(0, 0, halfSize));
    points.push_back(center + glm::vec3(0, 0, -halfSize));

    return points;
}

glm::vec3 SoundPathfindingQuery::GetVoxelSurfaceNormal(
    const glm::ivec3& voxel, const glm::vec3& direction) const
{
    // Find which face the direction is coming from
    glm::vec3 absDir = glm::abs(direction);

    if (absDir.x >= absDir.y && absDir.x >= absDir.z) {
        return glm::vec3(glm::sign(direction.x), 0, 0);
    }
    else if (absDir.y >= absDir.x && absDir.y >= absDir.z) {
        return glm::vec3(0, glm::sign(direction.y), 0);
    }
    else {
        return glm::vec3(0, 0, glm::sign(direction.z));
    }
}

bool SoundPathfindingQuery::IsInSoundRadius(
    const glm::vec3& point, const glm::vec3& source, float radius) const
{
    return glm::distance(point, source) <= radius;
}

float SoundPathfindingQuery::CalculatePathLength(const std::vector<glm::vec3>& waypoints) const
{
    if (waypoints.size() < 2) return 0.0f;

    float length = 0.0f;
    for (size_t i = 1; i < waypoints.size(); i++) {
        length += glm::distance(waypoints[i], waypoints[i - 1]);
    }
    return length;
}

std::vector<glm::ivec3> SoundPathfindingQuery::GetVoxelsAlongRay(
    const glm::vec3& start, const glm::vec3& end) const
{
    std::vector<glm::ivec3> voxels;

    glm::vec3 direction = end - start;
    float length = glm::length(direction);
    if (length < 0.001f) return voxels;

    direction /= length;

    // Use 3D DDA algorithm for voxel traversal
    glm::ivec3 currentVoxel;
    if (!m_World.WorldToVoxelSafe(start, currentVoxel)) {
        return voxels;
    }

    glm::vec3 step = glm::sign(direction);
    glm::vec3 tDelta = glm::abs(glm::vec3(
        direction.x != 0 ? m_World.GetVoxelSize() / std::abs(direction.x) : 1e10f,
        direction.y != 0 ? m_World.GetVoxelSize() / std::abs(direction.y) : 1e10f,
        direction.z != 0 ? m_World.GetVoxelSize() / std::abs(direction.z) : 1e10f
    ));

    glm::vec3 distToNext = glm::vec3(
        direction.x > 0 ? (m_World.VoxelCenterWorldPublic(currentVoxel).x + m_World.GetVoxelSize() * 0.5f - start.x) / direction.x
        : (start.x - (m_World.VoxelCenterWorldPublic(currentVoxel).x - m_World.GetVoxelSize() * 0.5f)) / -direction.x,
        direction.y > 0 ? (m_World.VoxelCenterWorldPublic(currentVoxel).y + m_World.GetVoxelSize() * 0.5f - start.y) / direction.y
        : (start.y - (m_World.VoxelCenterWorldPublic(currentVoxel).y - m_World.GetVoxelSize() * 0.5f)) / -direction.y,
        direction.z > 0 ? (m_World.VoxelCenterWorldPublic(currentVoxel).z + m_World.GetVoxelSize() * 0.5f - start.z) / direction.z
        : (start.z - (m_World.VoxelCenterWorldPublic(currentVoxel).z - m_World.GetVoxelSize() * 0.5f)) / -direction.z
    );

    const glm::ivec3 dims = m_World.VoxelDimensions();
    float traveled = 0.0f;

    voxels.push_back(currentVoxel);

    while (traveled < length) {
        // Find which axis to step next
        if (distToNext.x < distToNext.y && distToNext.x < distToNext.z) {
            traveled = distToNext.x;
            distToNext.x += tDelta.x;
            currentVoxel.x += step.x;
        }
        else if (distToNext.y < distToNext.z) {
            traveled = distToNext.y;
            distToNext.y += tDelta.y;
            currentVoxel.y += step.y;
        }
        else {
            traveled = distToNext.z;
            distToNext.z += tDelta.z;
            currentVoxel.z += step.z;
        }

        // Check bounds
        if (currentVoxel.x < 0 || currentVoxel.y < 0 || currentVoxel.z < 0 ||
            currentVoxel.x >= dims.x || currentVoxel.y >= dims.y || currentVoxel.z >= dims.z) {
            break;
        }

        voxels.push_back(currentVoxel);
    }

    return voxels;
}

bool SoundPathfindingQuery::CheckVoxelOcclusion(
    const glm::ivec3& voxel, const SoundPropagationConfig& config) const
{
    uint8_t density = m_World.GetVoxelDensityAt(voxel);

    // Very dense materials block sound
    if (density > 200) return true;

    // For lower frequencies, some materials may be transparent
    if (config.frequency < 500.0f && density < 150) {
        return false; // Low frequency passes through
    }

    // Check material properties
    VoxelAcousticProperties props = GetVoxelProperties(voxel);
    return props.transmissionCoeff < 0.1f; // Low transmission = occluding
}

uint64_t SoundPathfindingQuery::GetCacheKey(
    const glm::vec3& a, const glm::vec3& b, float frequency) const
{
    // Simple hash combining positions and frequency
    uint64_t hash = 0;

    // Combine coordinates (using integer representation)
    const int32_t* aInt = reinterpret_cast<const int32_t*>(&a);
    const int32_t* bInt = reinterpret_cast<const int32_t*>(&b);
    int32_t freqInt = *reinterpret_cast<const int32_t*>(&frequency);

    for (int i = 0; i < 3; i++) {
        hash ^= static_cast<uint64_t>(aInt[i]) << (i * 21);
        hash ^= static_cast<uint64_t>(bInt[i]) << (i * 21 + 1);
    }
    hash ^= static_cast<uint64_t>(freqInt) << 63;

    return hash;
}

float SoundPathfindingQuery::CalculateSpatialSpread(
    const std::vector<glm::vec3>& directions,
    const std::vector<float>& weights) const
{
    if (directions.empty() || weights.empty()) return 1.0f;

    glm::vec3 meanDirection(0);
    float totalWeight = 0.0f;

    // Calculate weighted mean direction
    for (size_t i = 0; i < directions.size(); i++) {
        if (glm::length(directions[i]) > 0.001f) {
            meanDirection += directions[i] * weights[i];
            totalWeight += weights[i];
        }
    }

    if (totalWeight < 0.001f) return 1.0f;

    meanDirection /= totalWeight;
    if (glm::length(meanDirection) < 0.001f) return 1.0f;

    meanDirection = glm::normalize(meanDirection);

    // Calculate variance (spread)
    float variance = 0.0f;
    for (size_t i = 0; i < directions.size(); i++) {
        if (glm::length(directions[i]) > 0.001f) {
            float dot = glm::dot(glm::normalize(directions[i]), meanDirection);
            // Convert dot product to angular difference (0-1)
            float angularDiff = (1.0f - dot) * 0.5f;
            variance += angularDiff * angularDiff * weights[i];
        }
    }

    variance /= totalWeight;

    // Normalize variance to 0-1 range
    return glm::clamp(variance * 2.0f, 0.0f, 1.0f);
}

void SoundPathfindingQuery::AnalyzeDirectionClusters(SoundPathResult& result) const
{
    if (result.arrivalDirections.empty()) return;

    // Simple clustering by angular separation
    const float CLUSTER_THRESHOLD = glm::radians(30.0f); // 30 degrees

    std::vector<std::vector<int>> clusters;
    std::vector<bool> assigned(result.arrivalDirections.size(), false);

    for (size_t i = 0; i < result.arrivalDirections.size(); i++) {
        if (assigned[i]) continue;

        std::vector<int> cluster;
        cluster.push_back(i);
        assigned[i] = true;

        for (size_t j = i + 1; j < result.arrivalDirections.size(); j++) {
            if (assigned[j]) continue;

            float angle = glm::angle(
                glm::normalize(result.arrivalDirections[i]),
                glm::normalize(result.arrivalDirections[j])
            );

            if (angle < CLUSTER_THRESHOLD) {
                cluster.push_back(j);
                assigned[j] = true;
            }
        }

        if (!cluster.empty()) {
            clusters.push_back(cluster);
        }
    }

    // Calculate cluster strengths
    std::vector<float> clusterStrengths(clusters.size(), 0.0f);
    std::vector<glm::vec3> clusterDirections(clusters.size(), glm::vec3(0));

    for (size_t i = 0; i < clusters.size(); i++) {
        glm::vec3 weightedDir(0);
        float totalWeight = 0.0f;

        for (int idx : clusters[i]) {
            float weight = result.directionalWeights[idx];
            clusterStrengths[i] += weight;
            weightedDir += result.arrivalDirections[idx] * weight;
            totalWeight += weight;
        }

        if (totalWeight > 0 && glm::length(weightedDir) > 0.001f) {
            clusterDirections[i] = glm::normalize(weightedDir);
        }
    }

    // Find strongest cluster
    if (!clusterStrengths.empty()) {
        auto strongestIt = std::max_element(clusterStrengths.begin(), clusterStrengths.end());
        size_t strongestIdx = std::distance(clusterStrengths.begin(), strongestIt);

        // If strongest cluster is significantly stronger than others, use its direction
        float strongestStrength = clusterStrengths[strongestIdx];
        float totalStrength = std::accumulate(clusterStrengths.begin(), clusterStrengths.end(), 0.0f);

        if (strongestStrength / totalStrength > 0.6f) { // 60% threshold
            glm::vec3 clusterDir = clusterDirections[strongestIdx];
            if (glm::length(clusterDir) > 0.001f) {
                result.dominantDirection = clusterDir;
                result.directionalCertainty = glm::clamp(strongestStrength / totalStrength, 0.0f, 1.0f);
            }
        }
    }
}