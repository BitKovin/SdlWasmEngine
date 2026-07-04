#pragma once

#include "../PickupBase.h"

#include <Entities/Player/Player.hpp>

class WeaponPickupBase : public PickupBase
{
public:
	
	std::string weaponName = "";
	std::string modelPath = "";

	void OnPickup(Player* player) override
	{

		player->AddWeaponByName(weaponName);

		PickupBase::OnPickup(player);
	}

	void LoadAssets() override
	{
		PickupBase::LoadAssets();
		auto mesh = new StaticMesh(this);
		mesh->LoadFromFile(modelPath);
		mesh->TexturesLocation = modelPath+"/";
		mesh->Position = Position;
		mesh->Rotation = Rotation - vec3(0, 90, 0);
		mesh->MeshCustomShaderParams["rim_pow"] = vec4(2.5f);
		mesh->MeshCustomShaderParams["rim_color"] = vec4(2.0f, 2.0f, 0.4f, 1.0f);
		mesh->Brightness = 2.5f;
		mesh->Scale = vec3(1.5f);
		Drawables.push_back(mesh);

	}

private:

};

