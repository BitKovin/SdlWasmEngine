#include "SpatialSoundManager.h"
#include "BspVoxelWorldBuilder.h"
#include "../Level.hpp"
#include "VoxelWorld/TiledVoxelWorld.h"
void SpatialSoundManager::BuildWorld()
{

	auto mesh = Level::Current->GetStaticNavObstaclesMesh();


	std::vector<glm::vec3> vertices = mesh.vertices;
	std::vector<uint32_t> indices = mesh.indices;
	if (vertices.size() < 3) return;

	glm::vec3 bmin = vertices[0], bmax = vertices[0];
	for (const auto& v : vertices) { bmin = glm::min(bmin, v); bmax = glm::max(bmax, v); }
	//bmin -= glm::vec3(0.2); bmax += glm::vec3(0.2);

	// Define world bounds and voxel size
	glm::vec3 minWorld = bmin;
	glm::vec3 maxWorld = bmax;
	float voxelSize = 1.0f;
	// Create voxel world
	voxelWorld = TiledVoxelWorld(minWorld, maxWorld, voxelSize);
	// Create a builder (e.g., BspVoxelWorldBuilder)
	BspVoxelWorldBuilder builder;
	// Build the voxel world using the builder
	voxelWorld.BuildStatic(builder, 2);
	// The voxelWorld is now built and can be used for spatial sound calculations
}
