#include "WeaponPickupBase.hpp"

class WeaponPickupCane : public WeaponPickupBase
{
public:
	WeaponPickupCane()
	{
		weaponName = "weapon_cane";
		modelPath = "GameData/models/pickups/weapons/cane.glb";
	}

private:

};

REGISTER_ENTITY(WeaponPickupCane, "item_weapon_cane")
