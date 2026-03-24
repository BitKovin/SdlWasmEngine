#pragma once

#include <glm.h>

class WorldOrientationManager
{
public:

	static void Reset();

	static inline vec3 GetUpVector() { return m_UpVector; }

	static vec3 GetWorldRotation();
	static quat GetWorldRotationQuat();

	static void UpdateWorld();

	static void SetUpVector(const vec3& up);

	static inline vec3 TransformDirectionToWorld(const vec3& localDirection)
	{
		return GetWorldRotationQuat() * localDirection;
	}

private:

	static inline vec3 m_UpVector = vec3(0.0f,1.0f,0.0f);

};

