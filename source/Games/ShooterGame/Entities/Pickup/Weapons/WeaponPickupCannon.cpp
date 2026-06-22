#include "WeaponPickupBase.hpp"

class WeaponPickupCannon : public WeaponPickupBase
{
public:
	WeaponPickupCannon()
	{
		weaponName = "weapon_cannon";
		modelPath = "GameData/models/pickups/weapons/cannon.glb";
	}

private:

};

REGISTER_ENTITY(WeaponPickupCannon, "item_weapon_cannon")
