#include "NpcAnimatorBase.h"

void NpcAnimatorBase::LoadAssets()
{
	locomotion = AddAnimation("GameData/models/npc/base.glb","idle");
	pistol = AddAnimation("GameData/models/npc/base.glb", "idle", false);
	taskAnimation = AddAnimation("GameData/models/npc/base.glb", "idle", false);
	taskAnimation->StopAnimation();
	//inPain = AddAnimation("GameData/animations/npc/inPain.glb");
}

void NpcAnimatorBase::Update()
{

	if (loaded == false) return;

	std::string desiredAnimation = scared ? "scared" : "idle";

	if (weapon_holds)
	{

		desiredAnimation = "pistol_hold";

		if (weapon_ready)
		{
			desiredAnimation = "pistol_idle";
		}

		if (weapon_aims)
		{
			desiredAnimation = "pistol_aim";
		}

		if (weapon_pendingAttack)
		{
			desiredAnimation = "pistol_fire";
			pistol->PlayAnimation("pistol_fire",false,0.1f);
			weapon_pendingAttack = false;
		}
	}

	if ((pistol->currentAnimationData->animationName == "pistol_fire" && pistol->IsAnimationPlaying()) == false)
	{

		if (pistol->currentAnimationData->animationName != desiredAnimation)
		{

			if (pistol->currentAnimationData->animationName == "idle")
			{
				pistol->PasteAnimationPose(lastPose);
			}

			pistol->PlayAnimation(desiredAnimation, false, 0.3);
		}

	}

	


	std::string desiredLocomotionAnimation = scared ? "scared" : "idle";

	if (movementSpeed > 1)
	{
		desiredLocomotionAnimation = "walk";
	}
	if (movementSpeed > 3)
	{
		desiredLocomotionAnimation = "run";
	}

	if (locomotion->currentAnimationData->animationName != desiredLocomotionAnimation)
	{
		locomotion->PlayAnimation(desiredLocomotionAnimation, true, 0.2);
	}

	Animator::Update();

}

void NpcAnimatorBase::PlayTaskAnimation(std::string animationName, bool loop)
{

	if (taskAnimation->IsAnimationPlaying() == false)
	{
		taskAnimation->PasteAnimationPose(lastPose);
	}

	taskAnimation->PlayAnimation(animationName, loop, 0.3f);

}

void NpcAnimatorBase::StopTaskAnimation()
{

	if (taskAnimation->currentAnimationData)
	{

		std::string currAnimationName = locomotion->currentAnimationData->animationName;
		std::string currWeaponAnimationName = pistol->currentAnimationData->animationName;

		auto taskPose = taskAnimation->GetAnimationPose();

		locomotion->PasteAnimationPose(taskPose);
		pistol->PasteAnimationPose(taskPose);
		locomotion->PlayAnimation(currAnimationName, true, 0.3f);
		pistol->PlayAnimation(currWeaponAnimationName, true, 0.3f);
	}

	taskAnimation->StopAnimation();

}

AnimationPose NpcAnimatorBase::ProcessResultPose()
{

	auto pistolPos = pistol->GetAnimationPose();

	//auto painPose = inPain->GetAnimationPose();

	AnimationPose locomotionPose = locomotion->GetAnimationPose();

	AnimationPose weaponResultPose;

	if (weapon_holds)
	{

		hashed_string startBone = "clavicle_r";

		if (weapon_ready || weapon_aims)
		{

			spineRotation.x = glm::clamp(spineRotation.x, -70.0f, 70.0f);
			spineRotation.y = glm::clamp(spineRotation.y, -30.0f, 30.0f);

			if (weapon_aims)
			{
				pistolPos.boneTransforms["spine_02"] = pistolPos.boneTransforms["spine_02"] * MathHelper::GetRotationMatrix(vec3(spineRotation.y, spineRotation.y * 0.3f, -spineRotation.x * 0.75f));
			}


			startBone = "spine_01";
		}

		weaponResultPose = AnimationPose::LayeredLerp(startBone, pistol->GetRootNode(), locomotionPose, pistolPos, true, 1);// AnimationPose::Lerp(locomotion, painPose, PainProgress);
	}
	else
	{
		weaponResultPose = locomotionPose;
	}

	AnimationPose taskResult;

	if (taskAnimation->IsAnimationPlaying())
	{

		auto taskPose = taskAnimation->GetAnimationPose();

		taskResult = AnimationPose::Lerp(weaponResultPose, taskPose, 1);
	}
	else
	{
		taskResult = weaponResultPose;
	}

	return taskResult;

}