#include "MovebleBrush.hpp"

void MovebleBrush::UpdateObstacle()
{
	if (obstacleRef != 0)
	{
		NavigationSystem::RemoveObstacle(obstacleRef);
		obstacleRef = 0;
	}
	
	vec3 min = vec3(FLT_MAX);
	vec3 max = vec3(-FLT_MAX);

	for (auto d : Drawables)
	{
		auto bounds = d->GetBoundingBox();
		min.x = glm::min(min.x, bounds.Min.x);
		min.y = glm::min(min.y, bounds.Min.y);
		min.z = glm::min(min.z, bounds.Min.z);

		max.x = glm::max(max.x, bounds.Max.x);
		max.y = glm::max(max.y, bounds.Max.y);
		max.z = glm::max(max.z, bounds.Max.z);
	}


	obstacleRef = NavigationSystem::CreateObstacleBox(min, max);
	
}
