#include "WeaponBase.h"
#include "../Player.hpp"

#include <SkeletalMesh.hpp>

class weapon_swords : public Weapon
{

private:

	SkeletalMesh* thirdPersonMesh = nullptr;

	bool comboContinueWait = false;
	bool comboContinue = false;

	float fullBodyAnimationInterp = 0;
	bool fullbodyAnimation = false;

	float timeSinceLastAttackStart = 0;

public:

	weapon_swords()
	{

	}
	~weapon_swords()
	{

	}

	void Update()
	{

		if (Input::GetAction("attack")->PressedBuffered(0.35f))
		{

			if (thirdPersonMesh->GetAnimationName() != "idle")
			{
				if (comboContinueWait)
				{
					comboContinueWait = false;
					comboContinue = true;
				}
					
			}
			else
			{
				thirdPersonMesh->PlayAnimation("combo_1_1");
			}


		}

		if (thirdPersonMesh->IsAnimationPlaying() == false)
		{
			thirdPersonMesh->PlayAnimation("idle", true);
		}

		thirdPersonMesh->Update(comboContinueWait ? 0.5f : 1.2f);

		auto events = thirdPersonMesh->PullAnimationEvents();

		for (auto& event : events)
		{

			if (event.eventName == "fullbody_start")
			{
				fullbodyAnimation = true;
			}

			if (event.eventName == "fullbody_end")
			{
				fullbodyAnimation = false;
			}

			if (event.eventName == "nextAttack_start")
			{
				comboContinue = false;
				comboContinueWait = true;
				timeSinceLastAttackStart = 0;
			}

			if (event.eventName == "nextAttack_end")
			{

				//bool comboContinue = Input::GetAction("attack")->PressedBuffered(0.55f);

				if (comboContinue == false)
				{
					fullbodyAnimation = false;
					thirdPersonMesh->PlayAnimation("idle", true, 0.6);
				}
				else
				{
					if (event.userData1.empty() == false)
					{
						thirdPersonMesh->PlayAnimation(event.userData1, false, 0.2);
					}
				}


				timeSinceLastAttackStart = 0;

				comboContinue = false;
				comboContinueWait = false;

			}

		}

		if (fullbodyAnimation)
		{
			fullBodyAnimationInterp += Time::DeltaTimeF * 4;
		}
		else
		{
			fullBodyAnimationInterp -= Time::DeltaTimeF * 4;
		}

		fullBodyAnimationInterp = std::clamp(fullBodyAnimationInterp, 0.f, 1.f);

		timeSinceLastAttackStart += Time::DeltaTimeF;

	}

	void LateUpdate()
	{
		thirdPersonMesh->Position = owner->bodyMesh->Position;
		thirdPersonMesh->Rotation = owner->bodyMesh->Rotation;
	}

	AnimationPose ApplyWeaponAnimation(AnimationPose thirdPersonPose) override
	{

		auto weaponPose = thirdPersonMesh->GetAnimationPose();

		auto leftFootT = MathHelper::DecomposeMatrix(AnimationPose::GetModelSpaceTransform("foot_l", thirdPersonMesh->GetRootNode(), thirdPersonPose));
		auto rightFootT = MathHelper::DecomposeMatrix(AnimationPose::GetModelSpaceTransform("foot_r", thirdPersonMesh->GetRootNode(), thirdPersonPose));

		auto resultPose = AnimationPose::LayeredLerp("pelvis", thirdPersonMesh->GetRootNode(),
			thirdPersonPose, weaponPose, 1, 0.6);
		

		resultPose = AnimationPose::LayeredLerp("thigh_l", thirdPersonMesh->GetRootNode(),
			resultPose, thirdPersonPose, 1, 0.5);

		resultPose = AnimationPose::LayeredLerp("thigh_r", thirdPersonMesh->GetRootNode(),
			resultPose, thirdPersonPose, 1, 0.5);

		resultPose = AnimationPose::ApplyFABRIK("thigh_l", "foot_l", thirdPersonMesh->GetRootNode(),
			resultPose, leftFootT.Position, leftFootT.RotationQuaternion);

		resultPose = AnimationPose::ApplyFABRIK("thigh_r", "foot_r", thirdPersonMesh->GetRootNode(),
			resultPose, rightFootT.Position, rightFootT.RotationQuaternion);

		resultPose = AnimationPose::LayeredLerp("thigh_l", thirdPersonMesh->GetRootNode(),
			resultPose, thirdPersonPose, 1, 0.7);

		resultPose = AnimationPose::LayeredLerp("thigh_r", thirdPersonMesh->GetRootNode(),
			resultPose, thirdPersonPose, 1, 0.7);

		resultPose = AnimationPose::LayeredLerp("spine_01", thirdPersonMesh->GetRootNode(),
			resultPose, weaponPose, 1, 1.0);

		resultPose = AnimationPose::Lerp(resultPose, weaponPose, fullBodyAnimationInterp);


		thirdPersonMesh->PasteAnimationPose(resultPose);

		return resultPose;

	}

	WeaponSlotData GetDefaultData() override
	{
		WeaponSlotData data;
		data.className = "weapon_swords"; // Override in subclasses
		data.slot = 0; // Override in subclasses
		return data;
	}

protected:

	void LoadAssets() override
	{
		
		Drawables.push_back(thirdPersonMesh = new SkeletalMesh(owner));

		thirdPersonMesh->LoadFromFile("GameData/models/player/weapons/swords/swords_tp.glb");

		thirdPersonMesh->PlayAnimation("idle");

	}

};

REGISTER_ENTITY(weapon_swords,"weapon_swords")