#pragma once

#include <Entity.h>
#include <json.hpp>
#include <Helpers/JsonHelper.hpp>
#include <Animation.h>

enum class WeaponAmmoType : uint8_t
{
	None = 0,
	PistolBullets,
	ShotgunShells,
	CannonBullets
};

struct WeaponSlotData
{
	string className = "";
	int slot = 0;
	int priority = 0;

	int startAmmo = 4;
	WeaponAmmoType AmmoType = WeaponAmmoType::None;

	bool offhand = false;

	std::string inventoryUUID = ""; // For inventory system tracking

	auto operator<=>(const WeaponSlotData&) const = default;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(WeaponSlotData, className,slot,priority, offhand, AmmoType)
};


class Player;

class Weapon : public Entity
{

protected:

	Delay SwitchDelay;

public:

	uint8_t weaponHandlingType = 0; //0 default with left hand holding, 1 for left hand resting, 2 for 2 separate hands holding 2 copies of weapon

	float HideWeapon = 0;

	WeaponSlotData Data;

	static inline std::string ArmsModelPath = "GameData/models/player/arms_p.glb";

	std::string thirdPersonModelPath = "GameData/models/player/weapons/pistol/pistol_tp.glb";

	bool Illegal = false;

	float bobScale = 1.0f;

	Player* owner = nullptr;

	float Spread = 0.0f;

	virtual bool CanAttack();

	bool ForceFirstPerson = false;

	bool Parrying = false;
	bool Blocking = false;

	bool SupportsOffhandWeapon = true;

	virtual void OnParried(){}
	virtual void OnBlocked() {}

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

	WeaponAmmoType GetAmmoType()
	{
		return Data.AmmoType;
	}

	virtual AnimationPose ApplyWeaponAnimation(AnimationPose thirdPersonPose)
	{
		return thirdPersonPose;
	}

	void LoadAssets() override;


private:

};

