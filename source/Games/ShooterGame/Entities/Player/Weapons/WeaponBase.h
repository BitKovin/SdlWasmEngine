#pragma once

#include <Entity.h>
#include <json.hpp>
#include <Helpers/JsonHelper.hpp>
#include <Animation.h>

struct WeaponSlotData
{
	string className = "";
	int slot = 0;
	int priority = 0;

	std::string inventoryUUID = ""; // For inventory system tracking

	auto operator<=>(const WeaponSlotData&) const = default;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(WeaponSlotData, className,slot,priority)
};

class Player;

class Weapon : public Entity
{

protected:

	Delay SwitchDelay;

public:

	float HideWeapon = 0;

	WeaponSlotData Data;

	std::string ArmsModelPath = "GameData/models/player/arms.glb";

	bool Illegal = false;

	float bobScale = 1.0f;

	Player* owner = nullptr;

	float Spread = 0.0f;

	virtual bool CanAttack();

	bool ForceFirstPerson = false;

	bool Parrying = false;

	virtual void OnParried(){}

	virtual bool IsMelee()
	{
		return false;
	}

	virtual void SetData(WeaponSlotData data)
	{
		Data = data;
	}

	virtual bool CanChangeSlot()
	{
		return SwitchDelay.Wait() == false;
	}

	virtual WeaponSlotData GetDefaultData()
	{
		return WeaponSlotData();
	}

	virtual AnimationPose ApplyWeaponAnimation(AnimationPose thirdPersonPose)
	{
		return thirdPersonPose;
	}



private:

};

