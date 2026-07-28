#pragma once

#include "../../Entity.h"


class DestructibleBrush : public Entity
{
public:

	DestructibleBrush() : Entity()
	{
		Static = false;
	}

	void OnDamage(float Damage, Entity* DamageCauser = nullptr, Entity* Weapon = nullptr) override
	{
		Destroy();
	}

	void Start()
	{
		for (auto model : Drawables)
		{
			BSPModelRef* m = (BSPModelRef*)model;

			if (m)
			{
				m->Static = false;
			}

		}
	}

private:

};
