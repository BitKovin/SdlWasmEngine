#include "SpatialSoundManager.h"
#include "BspVoxelWorldBuilder.h"
#include "../Level.hpp"
#include "VoxelWorld/FixedVoxelWorld.h"
#include "Helpers.h"
#include "../FileSystem/FileSystem.h"

#include "../MeshUtils.hpp"

void SpatialSoundManager::BuildWorld() 
{


    props.absorption[1] = 0.1f;
    props.reflectivity[1] = 0.9f;

    props.absorption[2] = 100.0f;
    props.reflectivity[2] = 0.0f;


    auto mesh = Level::Current->GetStaticNavObstaclesMesh();

    //mesh = MeshUtils::Subdivide(mesh, 3);

    worldBVH = new BVH(mesh.vertices, mesh.indices);

    std::string filePath = Level::Current->filePath + ".svd";

    //FileSystemEngine::WriteFileBinary(filePath, voxelWorld->m_data);


}

AudioSourceSpatialResult SpatialSoundManager::ComputeAudioSource(glm::vec3 position, float maxDistance) 
{


    // Transform source to voxel space
    glm::vec3 source_voxel = position;

    // Assume listener is in world space; transform to voxel space
    // (Adjust if listener is already voxel space)
    glm::vec3 listener_voxel = listener;

    // Max distance in voxel units
    float maxDist_voxel = maxDistance / voxelSize;

    AudioSourceSpatialResult result;
    result.occlusionGain = ComputeOcclusionGain(worldBVH, source_voxel, listener_voxel, props);
    result.reverb = ComputeReverb(worldBVH, source_voxel, listener_voxel, 128, 30, maxDist_voxel, props);


    return result;
}

uint8_t SpatialSoundManager::GetVoxelValueAt(vec3 pos)
{
    glm::vec3 source_voxel = WorldToVoxelPos(pos);

    return 0;

    //return voxelWorld->GetValue(source_voxel);

}
