#include "WorldOrientationManager.h"

#include <MathHelper.hpp>

#include <Physics.h>

void WorldOrientationManager::Reset()
{

	m_UpVector = vec3(0, 1, 0);

	UpdateWorld();

}

vec3 WorldOrientationManager::GetWorldRotation()
{
    vec3 defaultUp = vec3(0.0f, 1.0f, 0.0f);

    vec3 up = normalize(m_UpVector);

    if (glm::length2(up) < 0.000001f)
        return vec3(0.0f);

    glm::quat q = glm::rotation(defaultUp, up);

    return MathHelper::ToYawPitchRoll(q);
}

quat WorldOrientationManager::GetWorldRotationQuat()
{
    vec3 defaultUp = vec3(0.0f, 1.0f, 0.0f);

    vec3 up = normalize(m_UpVector);

    if (glm::length2(up) < 0.000001f)
        return vec3(0.0f);

    return glm::rotation(defaultUp, up);
}

void WorldOrientationManager::UpdateWorld()
{

	Physics::SetGravity(m_UpVector * -9.81f);

}

void WorldOrientationManager::SetUpVector(const vec3& up)
{

	m_UpVector = up;

}
