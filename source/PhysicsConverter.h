#pragma once

#include <Jolt/Jolt.h>
#include "glm.h"

inline glm::vec3 FromPhysics(JPH::Vec3 v)
{
	return vec3(v.GetX(), v.GetY(), v.GetZ());
}

inline JPH::Vec3 ToPhysics(glm::vec3 v)
{
	return JPH::Vec3(v.x, v.y, v.z);
}

inline glm::quat FromPhysics(JPH::Quat q)
{
	return quat(q.GetW(), q.GetX(), q.GetY(), q.GetZ());
}

inline JPH::Quat ToPhysics(glm::quat q)
{
	return JPH::Quat(q.x, q.y, q.z, q.w);
}