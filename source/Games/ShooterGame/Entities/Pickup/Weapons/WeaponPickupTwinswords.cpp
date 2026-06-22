#include "WeaponPickupBase.hpp"

class WeaponPickupTwinswords : public WeaponPickupBase
{
public:
	WeaponPickupTwinswords()
	{
		weaponName = "weapon_twinsword";
		modelPath = "GameData/models/pickups/weapons/twinswords.glb";
	}

private:

};

REGISTER_ENTITY(WeaponPickupTwinswords, "item_weapon_twinswords")
