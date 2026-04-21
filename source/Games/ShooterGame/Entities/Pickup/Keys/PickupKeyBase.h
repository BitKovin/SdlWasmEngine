#pragma once

#include "../PickupBase.h"
#include "KeyTypes.h"
#include <Entities/Player/Player.hpp>

class KeyBase : public PickupBase
{

protected:

	DoorKey keyType = DoorKey::None;

	void OnPickup(Player* player) override
	{

		if(player->keysInventory.count(keyType) == 0)
			player->keysInventory.insert(keyType);


		Destroy();
		PickupBase::OnPickup(player);

	}


};

