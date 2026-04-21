#include <Entities/Brushes/TriggerBase.hpp>
#include <Entities/Player/Player.hpp>

#include <Helpers/StringHelper.h>

class TriggerKeyMultiple : public TriggerBase
{
public:
	
	std::set<DoorKey> requiredKeys;

	DoorKey ParseKeyFromName(std::string name)
	{

		name = StringHelper::ToLower(name);

		if (name == "red") return DoorKey::Red;
		if (name == "green") return DoorKey::Green;
		if (name == "blue") return DoorKey::Blue;
		if (name == "secret") return DoorKey::Secret;
		if (name == "special") return DoorKey::Special;

		return DoorKey::None;

	}

	void FromData(EntityData data)
	{
		TriggerBase::FromData(data);

		std::string keysVal = data.GetPropertyString("requiredKeys");

		auto res = StringHelper::Split(keysVal, ' ');

		for (auto keyName : res)
		{

			DoorKey k = ParseKeyFromName(keyName);

			if(k!=DoorKey::None)
				requiredKeys.insert(k);
		}

	}

	bool CanBeTriggered(Entity* entity) override
	{

		Player* player = dynamic_cast<Player*>(entity);

		if (player == nullptr) return false;

		for (auto& k : requiredKeys)
		{
			if (player->keysInventory.count(k) == 0) return false;
		}
		return true;
	}

private:

};

REGISTER_ENTITY(TriggerKeyMultiple, "trigger_key_multi")