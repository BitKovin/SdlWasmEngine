#include "WeaponPickupBase.hpp"

class WeaponPickupSMG : public WeaponPickupBase
{
public:
	WeaponPickupSMG()
	{
		weaponName = "weapon_mpsd";
		modelPath = "GameData/models/pickups/weapons/stg44.glb";
	}

private:

};

REGISTER_ENTITY(WeaponPickupSMG, "item_weapon_smg")
