#include "WeaponBase.h"

#include "../Player.hpp"

bool Weapon::CanAttack()
{
	return owner->RunProgress < 0.01f && owner->dead == false;
}

void Weapon::LoadAssets()
{

	SkeletalMesh* skm = new SkeletalMesh(this);

	skm->LoadFromFile(thirdPersonModelPath);
	skm->PreloadAssets();
	delete skm;


}
