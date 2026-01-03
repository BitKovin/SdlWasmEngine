#include "WeaponBase.h"
#include "../Player.hpp"
#include "WeaponFirearm.h"

#include <Animation.h>

#include "Projectiles/CaneProjectile.h"
#include <SoundSystem/FmodEventInstance.h>


class weapon_lefthand_empty : public Weapon
{
public:

	SkeletalMesh* viewmodel = nullptr;
	SkeletalMesh* arms = nullptr;

	vec3 weaponOffset = vec3(0.0, -0.02, -0.0);


	weapon_lefthand_empty()
	{

		LateUpdateWhenPaused = true;
	}


	void Start()
	{

	}


	void LoadAssets()
	{

		viewmodel = new SkeletalMesh(this);
		arms = new SkeletalMesh(this);

		viewmodel->LoadFromFile("GameData/models/player/weapons/leftHand/empty.glb");
 
		viewmodel->PlayAnimation("rest", true);
		viewmodel->PreloadAssets();
		viewmodel->Visible = false;

		viewmodel->IsViewmodel = true;

		Drawables.push_back(viewmodel);

		arms->LoadFromFile(ArmsModelPath);
		arms->IsViewmodel = true;
		Drawables.push_back(arms);

		PreloadEntityType("caneProjectile");

	}



	void AsyncUpdate()
	{
		viewmodel->Update();

		auto pose = viewmodel->GetAnimationPose();

		arms->PasteAnimationPose(pose);
	}

	void LateUpdate()
	{

		if (Player::Instance == nullptr)
			return;

		arms->Visible = true;

		if (Player::Instance->currentWeapon != nullptr)
		{

			WeaponFirearm* firearm = dynamic_cast<WeaponFirearm*>(Player::Instance->currentWeapon);

			arms->Visible = firearm->akimbo == false;
		}

		viewmodel->Position = Position + (mat3)Camera::GetRotationMatrix() * weaponOffset;
		viewmodel->Rotation = Rotation;

		arms->Position = viewmodel->Position;
		arms->Rotation = viewmodel->Rotation;

	}


	WeaponSlotData GetDefaultData() override
	{

		WeaponSlotData data;

		data.className = "weapon_lefthand_empty";

		return data;
	}


private:

};

REGISTER_ENTITY(weapon_lefthand_empty, "weapon_lefthand_empty")