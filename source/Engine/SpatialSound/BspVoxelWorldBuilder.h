#pragma once
#include "VoxelWorld/VoxelWorldBuilder.h"

class BspVoxelWorldBuilder : public VoxelWorldBuilder
{

	public:

		float voxelSize = 1.0f;
		vec3 worldMin = vec3();

		BspVoxelWorldBuilder()
		{
		}
		// worldPos is CENTER of voxel in world space
		// return density / traversal price [0..255]
		virtual uint8_t GetValue(const glm::vec3& worldPos) const override;

};