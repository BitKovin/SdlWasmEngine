#pragma once
#include "VoxelWorld/VoxelWorldBuilder.h"

class BspVoxelWorldBuilder : public VoxelWorldBuilder
{

	public:
		BspVoxelWorldBuilder()
		{
		}
		// worldPos is CENTER of voxel in world space
		// return density / traversal price [0..255]
		virtual uint8_t GetDensity(const glm::vec3& worldPos) const override;

};