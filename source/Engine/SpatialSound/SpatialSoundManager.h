#pragma once

#include "VoxelWorld/FixedVoxelWorld.h"
#include "VoxelWorld/VoxelWorldBuilder.h"
#include "VoxelWorld/MaterialProps.h"
#include "VoxelWorld/Raycast.h"

struct AudioSourceSpatialResult
{
	ReverbStats reverb;
	float occlusionGain = 0;
};

class SpatialSoundManager
{
public:
	
	static void BuildWorld();

	static AudioSourceSpatialResult ComputeAudioSource(vec3 position, float maxDistance);

	static uint8_t GetVoxelValueAt(vec3 pos);

	static inline FixedVoxelWorld* voxelWorld = nullptr;

	static inline vec3 listener;

	static inline vec3 worldMin = vec3();

	static inline float voxelSize = 0.5f;

private:



	static inline MaterialProps props;

};

