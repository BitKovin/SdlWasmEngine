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

	void ShowMessage(Player* player)
	{
		std::string msg = "${KEYS_REQUIRED_MESSAGE}";
		for (auto& k : requiredKeys)
		{
			switch (k)
			{
			case DoorKey::Red:
				msg += "${KEYS_NAME_RED} ";
				break;
			case DoorKey::Green:
				msg += "${KEYS_NAME_GREEN} ";
				break;
			case DoorKey::Blue:
				msg += "${KEYS_NAME_BLUE} ";
				break;
			case DoorKey::Secret:
				msg += "${KEYS_NAME_SECRET} ";
				break;
			case DoorKey::Special:
				msg += "${KEYS_NAME_SPECIAL} ";
				break;
			default:
				break;
			}
		}

		if (requiredKeys.size() == 1)
		{
			switch (*requiredKeys.begin())
			{
			case DoorKey::Red:
				msg = "${KEYS_REQUIRED_RED}";
				break;
			case DoorKey::Green:
				msg = "${KEYS_REQUIRED_GREEN}";
				break;
			case DoorKey::Blue:
				msg = "${KEYS_REQUIRED_BLUE}";
				break;
			case DoorKey::Secret:
				msg = "${KEYS_REQUIRED_SECRET}";
				break;
			case DoorKey::Special:
				msg = "${KEYS_REQUIRED_SPECIAL}";
				break;
			default:
				break;
			}
		}

		player->Hud.ShowMessage(msg, 2.0f);
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
			if (player->keysInventory.count(k) == 0)
			{
				ShowMessage(player);
				return false;
			}
		}
		return true;
	}

private:

};

REGISTER_ENTITY(TriggerKeyMultiple, "trigger_key_multi")