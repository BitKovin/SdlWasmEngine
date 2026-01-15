#include "SpatialSoundManager.h"
#include "BspVoxelWorldBuilder.h"
#include "../Level.hpp"
#include "VoxelWorld/FixedVoxelWorld.h"
#include "Helpers.h"
#include "../FileSystem/FileSystem.h"

void SpatialSoundManager::BuildWorld() 
{


    props.absorption[1] = 0.1f;
    props.reflectivity[1] = 0.9f;

    props.absorption[2] = 100.0f;
    props.reflectivity[2] = 0.0f;




    if (voxelWorld != nullptr) {
        delete voxelWorld;
        voxelWorld = nullptr;
    }

    auto mesh = Level::Current->GetStaticNavObstaclesMesh();
    std::vector<glm::vec3> vertices = mesh.vertices;
    std::vector<uint32_t> indices = mesh.indices;
    if (vertices.size() < 3) return;

    glm::vec3 bmin = vertices[0], bmax = vertices[0];
    for (const auto& v : vertices) {
        bmin = glm::min(bmin, v);
        bmax = glm::max(bmax, v);
    }
    bmin -= glm::vec3(1.0f);
    bmax += glm::vec3(1.0f);

    // Define world bounds and voxel size
    glm::vec3 minWorld = bmin;
    glm::vec3 maxWorld = bmax;
    worldMin = minWorld;

    // Corrected size: Use ceil to fully cover the range
    glm::vec3 extent = (maxWorld - minWorld) / voxelSize;
    glm::ivec3 size = glm::ivec3(glm::ceil(extent));
    
    std::string filePath = Level::Current->filePath + ".svd";

    auto fileData = FileSystemEngine::ReadFileBinary(filePath);

    if (fileData.empty() == false)
    {
        voxelWorld = new FixedVoxelWorld(size, fileData);
        return;
    }

    BspVoxelWorldBuilder builder;
    
    voxelWorld = new FixedVoxelWorld(size, builder);

    FileSystemEngine::WriteFileBinary(filePath, voxelWorld->m_data);


}

AudioSourceSpatialResult SpatialSoundManager::ComputeAudioSource(glm::vec3 position, float maxDistance) {
    // Transform source to voxel space
    glm::vec3 source_voxel = WorldToVoxelPos(position);

    // Assume listener is in world space; transform to voxel space
    // (Adjust if listener is already voxel space)
    glm::vec3 listener_voxel = WorldToVoxelPos(listener);

    // Max distance in voxel units
    float maxDist_voxel = maxDistance / voxelSize;

    AudioSourceSpatialResult result;
    result.occlusionGain = ComputeOcclusionGain(voxelWorld, source_voxel, listener_voxel, props);
    result.reverb = ComputeReverb(voxelWorld, source_voxel, listener_voxel, 1024, 30, maxDist_voxel, props);

    // Scale back to world units
    result.reverb.averageDistance *= voxelSize;

    return result;
}

uint8_t SpatialSoundManager::GetVoxelValueAt(vec3 pos)
{
    glm::vec3 source_voxel = WorldToVoxelPos(pos);

    return voxelWorld->GetValue(source_voxel);

}
