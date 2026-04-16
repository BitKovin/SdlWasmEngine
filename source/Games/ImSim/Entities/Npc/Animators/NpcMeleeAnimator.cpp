#include "NpcMeleeAnimator.h"


void NpcMeleeAnimator::Update()
{

	if (loaded == false) return;


	std::string desiredLocomotionAnimation = scared ? "scared" : "idle";

	if (movementSpeed > 1)
	{
		desiredLocomotionAnimation = "walk";
	}
	if (movementSpeed > 3)
	{
		desiredLocomotionAnimation = "run";
	}

	if (weapon_ready)
	{
		if (desiredLocomotionAnimation == "idle")
			desiredLocomotionAnimation = "sword_idle";

		if (desiredLocomotionAnimation == "walk")
			desiredLocomotionAnimation = "sword_walk";
	}

	if (locomotion->currentAnimationData->animationName != desiredLocomotionAnimation)
	{
		locomotion->PlayAnimation(desiredLocomotionAnimation, true, 0.3);
	}

	std::string desiredSwordAnimation = "sword_idle";

	if(blocking)
		desiredSwordAnimation = "sword_block";

	if (blockAnimation->currentAnimationData->animationName != desiredSwordAnimation)
	{
		blockAnimation->PasteAnimationPose(lastPose);
		blockAnimation->PlayAnimation(desiredSwordAnimation, true, 0.3);
	}

	Animator::Update();

}

bool NpcMeleeAnimator::IsPlayingActionAnimation()
{
	return actionAnimation->IsAnimationPlaying();
}

void NpcMeleeAnimator::LoadAssets()
{
	locomotion = AddAnimation("GameData/models/npc/base.glb", "idle");
	pistol = AddAnimation("GameData/models/npc/base.glb", "idle", false);
	blockAnimation = AddAnimation("GameData/models/npc/base.glb", "sword_block", false);
	taskAnimation = AddAnimation("GameData/models/npc/base.glb", "idle", false);
	taskAnimation->StopAnimation();

	actionAnimation = AddAnimation("GameData/models/npc/base.glb", "idle", false);
	actionAnimation->StopAnimation();

}

AnimationPose NpcMeleeAnimator::ProcessResultPose()
{
	auto pistolPos = pistol->GetAnimationPose();

	//auto painPose = inPain->GetAnimationPose();

	AnimationPose locomotionPose = locomotion->GetAnimationPose();

	AnimationPose weaponResultPose;

	if (weapon_holds)
	{
		weaponResultPose = AnimationPose::LayeredLerp("spine_01", locomotion->GetRootNode(), locomotionPose, blockAnimation->GetAnimationPose(), 1, 1);
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

	if (actionAnimation->IsAnimationPlaying())
	{

		auto actionPose = actionAnimation->GetAnimationPose();

		float remainTime = actionAnimation->GetAnimationDuration() - actionAnimation->GetAnimationTime();

		float blend = std::clamp(remainTime / actionBlendOut,0.0f,1.0f);

		taskResult = AnimationPose::Lerp(actionPose, taskResult, 1.0f - blend);// should be inverced to get root motion from 2nd one
	}

	return taskResult;
}

void NpcMeleeAnimator::PlayActionAnimation(std::string animationName, bool loop, float blendIn, float blendOut)
{

	if (actionAnimation->IsAnimationPlaying() == false)
	{
		actionAnimation->PasteAnimationPose(lastPose);
	}

	actionAnimation->PlayAnimation(animationName, loop, blendIn);
	actionBlendOut = blendOut;

}

void NpcMeleeAnimator::StopActionAnimation()
{

	if (actionAnimation->currentAnimationData)
	{

		std::string currAnimationName = locomotion->currentAnimationData->animationName;
		std::string currWeaponAnimationName = pistol->currentAnimationData->animationName;

		auto taskPose = actionAnimation->GetAnimationPose();

		locomotion->PasteAnimationPose(taskPose);
		pistol->PasteAnimationPose(taskPose);
		locomotion->PlayAnimation(currAnimationName, true, 0.3f);
		pistol->PlayAnimation(currWeaponAnimationName, true, 0.3f);
	}

	actionAnimation->StopAnimation();

}