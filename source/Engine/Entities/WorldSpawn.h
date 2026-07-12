#pragma once

#include "../Entity.h"
#include "../BSP/Quake3Bsp.h"

#include "Brushes/Sound/FmodReverbZone.h"

class WorldSpawn : public Entity
{
public:

	WorldSpawn()
	{
		Static = true;
	}
	~WorldSpawn() = default;
	
	static inline WorldSpawn* Instance = nullptr;

	void FromData(EntityData data)
	{
		Entity::FromData(data);

		printf("world spawn created\n");

		CQuake3BSP::lightVolGridSize = data.GetPropertyVector("gridsize", vec3(32, 32, 64));

		Instance = this;

		FmodReverbZone::DefaultSnapshotPath = data.GetPropertyString("reverbSnapshot", "");

	}

private:

};
