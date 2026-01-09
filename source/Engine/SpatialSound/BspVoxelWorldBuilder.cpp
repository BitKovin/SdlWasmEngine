#include "BspVoxelWorldBuilder.h"
#include "../Level.hpp"

uint8_t BspVoxelWorldBuilder::GetDensity(const glm::vec3& worldPos) const
{

	int dencity = (Level::Current->BspData.FindClusterAtPosition(worldPos) >= 0) ? 0 : 20;

	return dencity;
}
