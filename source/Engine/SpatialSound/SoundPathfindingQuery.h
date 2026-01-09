#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <functional>
#include "VoxelWorld/TiledVoxelWorld.h"

// Sound physics constants
constexpr float SPEED_OF_SOUND = 343.0f; // m/s
constexpr float AIR_DENSITY = 1.225f;    // kg/m³

enum class SoundPathType {
    DIRECT,
    REFLECTED,
    DIFFRACTED,
    TRANSMITTED
};

struct SoundPath {
    std::vector<glm::vec3> waypoints;        // World-space positions
    float totalDistance = 0.0f;              // Total path length in meters
    float attenuation = 1.0f;                // 0-1 scale (1 = no attenuation)
    float intensity = 1.0f;                  // Normalized intensity
    SoundPathType type = SoundPathType::DIRECT;
    glm::vec3 arrivalDirection = glm::vec3(0); // Direction at listener
    float arrivalTime = 0.0f;                // Time delay in seconds
    bool occluded = false;                   // If path is blocked
};

struct SoundPathResult {
    std::vector<SoundPath> paths;
    float totalIntensity = 0.0f;
    float averageDelay = 0.0f;
    glm::vec3 dominantDirection = glm::vec3(0); // Perceived direction of sound
    glm::vec3 sourceDirection = glm::vec3(0);   // Direction to actual source
    float directionalCertainty = 0.0f;          // How certain we are of direction
    float spatialSpread = 0.0f;                 // How spread out the sound is
    float directToReverberantRatio = 0.0f;

    // Directional statistics
    std::vector<glm::vec3> arrivalDirections;
    std::vector<float> directionalWeights;
};

// Configuration for sound propagation
struct SoundPropagationConfig {
    float frequency = 1000.0f;           // Hz
    float sourceRadius = 1.0f;           // meters
    float maxSearchRadius = 50.0f;       // meters
    float soundIntensity = 1.0f;         // Base intensity
    float airAbsorptionCoeff = 0.01f;    // dB/m at 1kHz
    float materialAbsorptionScale = 1.0f;
    int maxPathCount = 50;               // Maximum paths to find
    int maxBounces = 5;                  // Maximum reflection bounces
    bool enableDiffraction = true;
    bool enableTransmission = false;     // Sound through materials
    float diffractionThreshold = 0.1f;   // Minimum edge sharpness
};

// Voxel material properties for acoustics
struct VoxelAcousticProperties {
    float density;           // g/cm³
    float speedOfSound;      // m/s
    float absorptionCoeff;   // 0-1
    float reflectionCoeff;   // 0-1
    float transmissionCoeff; // 0-1
    float roughness;         // 0-1
};

// Main class for sound propagation through voxel worlds
class SoundPathfindingQuery {
public:
    SoundPathfindingQuery(const TiledVoxelWorld& world);

    // Main method to find sound paths from source to listener
    SoundPathResult FindSoundPaths(
        const glm::vec3& sourceWorld,
        const glm::vec3& listenerWorld,
        const SoundPropagationConfig& config
    );

    // Quick check for line-of-sight with attenuation
    float CheckLineOfSight(
        const glm::vec3& sourceWorld,
        const glm::vec3& listenerWorld,
        const SoundPropagationConfig& config
    );

    // Batch processing for multiple listeners
    std::vector<SoundPathResult> FindSoundPathsToMultiple(
        const glm::vec3& sourceWorld,
        const std::vector<glm::vec3>& listenerWorlds,
        const SoundPropagationConfig& config
    );

    // Set custom material properties for specific density ranges
    void SetMaterialProperties(
        uint8_t minDensity,
        uint8_t maxDensity,
        const VoxelAcousticProperties& props
    );

    // Get acoustic properties for a voxel
    VoxelAcousticProperties GetVoxelProperties(const glm::ivec3& voxel) const;

private:
    // Path finding methods
    SoundPath FindDirectPath(
        const glm::vec3& source,
        const glm::vec3& listener,
        const SoundPropagationConfig& config
    );

    void AnalyzeDirectionClusters(SoundPathResult& result) const;

    float CalculateSpatialSpread(
        const std::vector<glm::vec3>& directions,
        const std::vector<float>& weights) const;
    

    std::vector<SoundPath> FindReflectedPaths(
        const glm::vec3& source,
        const glm::vec3& listener,
        const SoundPropagationConfig& config,
        int currentBounce = 0
    );

    std::vector<SoundPath> FindDiffractedPaths(
        const glm::vec3& source,
        const glm::vec3& listener,
        const SoundPropagationConfig& config
    );

    // Ray casting for sound propagation
    struct RayCastResult {
        std::vector<glm::vec3> samplePoints;
        std::vector<float> attenuationPerSegment;
        std::vector<glm::ivec3> traversedVoxels;
        float totalAttenuation = 1.0f;
        bool blocked = false;
        glm::vec3 exitPoint;
        float travelTime = 0.0f;
    };

    RayCastResult CastSoundRay(
        const glm::vec3& start,
        const glm::vec3& end,
        const SoundPropagationConfig& config,
        bool collectDetails = true
    );

    // Physics calculations
    float CalculateDistanceAttenuation(float distance, const SoundPropagationConfig& config);
    float CalculateMaterialAttenuation(const std::vector<glm::ivec3>& voxels, const SoundPropagationConfig& config);
    float CalculateAirAbsorption(float distance, const SoundPropagationConfig& config);
    float CalculateSpreadingAttenuation(const glm::vec3& sourceDir, const glm::vec3& pathDir);

    // Reflection calculations
    struct ReflectionSurface {
        glm::vec3 point;
        glm::vec3 normal;
        glm::ivec3 voxel;
        float roughness;
        float reflectionCoeff;
    };

    std::vector<ReflectionSurface> FindReflectionSurfaces(
        const glm::vec3& source,
        const glm::vec3& listener,
        const SoundPropagationConfig& config
    );

    glm::vec3 CalculateImageSource(
        const glm::vec3& source,
        const glm::vec3& surfacePoint,
        const glm::vec3& surfaceNormal
    );

    // Diffraction calculations
    struct DiffractionEdge {
        glm::vec3 point1;
        glm::vec3 point2;
        glm::vec3 midpoint;
        glm::vec3 normal;
        float sharpness;  // 0-1 how sharp the edge is
    };

    std::vector<DiffractionEdge> FindDiffractionEdges(
        const glm::vec3& source,
        const glm::vec3& listener,
        const SoundPropagationConfig& config
    );

    float CalculateDiffractionAttenuation(
        const DiffractionEdge& edge,
        const glm::vec3& source,
        const glm::vec3& listener,
        const SoundPropagationConfig& config
    );

    // Helper functions
    std::vector<glm::vec3> GetVoxelSurfacePoints(const glm::ivec3& voxel) const;
    glm::vec3 GetVoxelSurfaceNormal(const glm::ivec3& voxel, const glm::vec3& direction) const;
    bool IsInSoundRadius(const glm::vec3& point, const glm::vec3& source, float radius) const;
    float CalculatePathLength(const std::vector<glm::vec3>& waypoints) const;

    // Voxel grid traversal
    std::vector<glm::ivec3> GetVoxelsAlongRay(const glm::vec3& start, const glm::vec3& end) const;
    bool CheckVoxelOcclusion(const glm::ivec3& voxel, const SoundPropagationConfig& config) const;

private:
    const TiledVoxelWorld& m_World;

    // Material property mapping
    std::vector<std::pair<std::pair<uint8_t, uint8_t>, VoxelAcousticProperties>> m_MaterialMap;

    // Precomputed directions for sampling (26 directions + 6 cardinal)
    static const glm::vec3 SAMPLE_DIRECTIONS[32];

    // Cache for performance
    mutable std::unordered_map<uint64_t, float> m_AttenuationCache;
    mutable std::unordered_map<uint64_t, RayCastResult> m_RayCache;

    // Helper for cache keys
    uint64_t GetCacheKey(const glm::vec3& a, const glm::vec3& b, float frequency) const;
};