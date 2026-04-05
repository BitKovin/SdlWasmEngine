#include "PickupBase.h"
#include <Entities/Player/Player.hpp>

class PickupAmmoShotgun : public PickupBase
{

	int ammoAmount = 16;

	void OnPickup(class Player* player) override
	{
		auto ammoType = WeaponAmmoType::ShotgunShells;

		if (player->GetAmmo(ammoType) == player->GetAmmoLimit(ammoType)) return;
		player->AddAmmo(ammoType, ammoAmount);
		Destroy();
	}

	void FromData(EntityData data) override
	{
		PickupBase::FromData(data);
		ammoAmount = data.GetPropertyFloat("ammoAmount", ammoAmount);
	}

	void LoadAssets() override
	{
		PickupBase::LoadAssets();
		auto mesh = new StaticMesh(this);
		mesh->LoadFromFile("GameData/models/pickups/shotgunAmmo.obj");
		mesh->TexturesLocation = "GameData/models/pickups/";
		mesh->Position = Position;
		mesh->Rotation = Rotation - vec3(0, 90, 0);
		mesh->Scale = vec3(1 / 32.f);
		Drawables.push_back(mesh);
	}

};

REGISTER_ENTITY(PickupAmmoShotgun, "item_ammo_shotgun")