#include "WeaponBase.h"

#include "../Player.hpp"

bool Weapon::CanAttack()
{
	return owner->RunProgress < 0.01f && owner->dead == false;
}
