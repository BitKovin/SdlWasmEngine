#include "WeaponPickupBase.hpp"

class WeaponPickupShotgun : public WeaponPickupBase
{
public:
	WeaponPickupShotgun()
	{
		weaponName = "weapon_shotgun";
		modelPath = "GameData/models/pickups/weapons/shotgun.glb";
	}

private:

};

REGISTER_ENTITY(WeaponPickupShotgun, "item_weapon_shotgun")