#include "RandomHelper.h"

#include <random>

float RandomHelper::RandomFloat()
{
	return static_cast<float>(rand()) / (float)RAND_MAX;
}
int RandomHelper::RandomInt()
{
    return static_cast<int>(rand()) / (float)RAND_MAX;
}

vec3 RandomHelper::RandomPosition(float radius)
{
    return glm::vec3(
        (RandomFloat() - 0.5f) * 2.0f * radius,
        (RandomFloat() - 0.5f) * 2.0f * radius,
        (RandomFloat() - 0.5f) * 2.0f * radius
    );
}