#pragma once

#include "Entities/PointLight.h"

class WeaponFireFlash : public PointLight
{
public:
	WeaponFireFlash()
	{
		color = vec3(1, 1, 0.4f);
	}
	~WeaponFireFlash(){}

	float duration = 0.2f;
	float flashRadius = 5;
	float flashIntencity = 1;

	void LateUpdate()
	{
		float progress = glm::clamp((float)((Time::GameTime - SpawnTime) / duration),0.0f,1.0f);

		if (progress >= 1)
		{
			Destroy();
			return;
		}

		radius = flashRadius * lerp(1, 0.5f, progress);
		intensity = lerp(flashIntencity, 0, progress);

	}

	static void CreateAt(vec3 position, float duration = 0.15f, float radius = 16, float intencity = 0.4)
	{

		WeaponFireFlash* flash = new WeaponFireFlash();
		flash->Position = position;
		flash->duration = duration;
		flash->flashRadius = radius;
		flash->flashIntencity = intencity;
		Level::Current->AddEntity(flash);
		flash->Start();
		flash->LateUpdate();

	}

private:

};