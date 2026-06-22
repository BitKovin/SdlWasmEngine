#include "WeaponPickupBase.hpp"

class WeaponPickupPistol : public WeaponPickupBase
{
public:
	WeaponPickupPistol()
	{
		weaponName = "weapon_pistol";
		modelPath = "GameData/models/pickups/weapons/pistol.glb";
	}

private:

};

REGISTER_ENTITY(WeaponPickupPistol, "item_weapon_pistol")