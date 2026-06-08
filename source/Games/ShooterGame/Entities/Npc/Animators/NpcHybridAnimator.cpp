#include "NpcHybridAnimator.h"

void NpcHybridAnimator::LoadAssets()
{
	// Inherits everything: locomotion, pistol, taskAnimation, actionAnimation
	// (base), blockAnimation (melee).  Nothing extra is needed.
	NpcMeleeAnimator::LoadAssets();
}

void NpcHybridAnimator::Update()
{
	if (!loaded) return;

	// ── Smooth combat mode blend ──────────────────────────────────────────────
	const float targetBlend = (targetCombatMode == CombatMode::Ranged) ? 1.0f : 0.0f;
	combatModeBlend = glm::mix(combatModeBlend, targetBlend,
	                           glm::clamp(Time::DeltaTimeF * combatModeBlendSpeed, 0.0f, 1.0f));

	// ── Locomotion – use sword variants when firmly in melee mode ─────────────
	std::string desiredLocoAnim = scared ? "scared" : "idle";
	if (movementSpeed > 1) desiredLocoAnim = "walk";
	if (movementSpeed > 3) desiredLocoAnim = "run";

	if (combatModeBlend < 0.5f && weapon_ready)
	{
		if (desiredLocoAnim == "idle") desiredLocoAnim = "sword_idle";
		if (desiredLocoAnim == "walk") desiredLocoAnim = "sword_walk";
	}

	if (locomotion->currentAnimationData->animationName != desiredLocoAnim)
		locomotion->PlayAnimation(desiredLocoAnim, true, 0.3f);

	// ── Ranged (pistol) layer – keep updated even in melee for smooth blend ───
	std::string desiredPistolAnim = scared ? "scared" : "idle";
	if (weapon_holds)
	{
		desiredPistolAnim = "pistol_hold";
		if (weapon_ready) desiredPistolAnim = "pistol_idle";
		if (weapon_aims)  desiredPistolAnim = "pistol_aim";

		if (weapon_pendingAttack)
		{
			pistol->PlayAnimation("pistol_fire", false, 0.1f);
			weapon_pendingAttack = false;
			desiredPistolAnim    = "pistol_fire";
		}
	}

	const bool pistolFiring = pistol->currentAnimationData->animationName == "pistol_fire"
	                       && pistol->IsAnimationPlaying();

	if (!pistolFiring && pistol->currentAnimationData->animationName != desiredPistolAnim)
	{
		if (pistol->currentAnimationData->animationName == "idle")
			pistol->PasteAnimationPose(lastPose);
		pistol->PlayAnimation(desiredPistolAnim, false, 0.3f);
	}

	// ── Melee (block) layer – keep updated even in ranged for smooth blend ────
	const std::string desiredBlockAnim = blocking ? "sword_block" : "sword_idle";
	if (blockAnimation->currentAnimationData->animationName != desiredBlockAnim)
	{
		blockAnimation->PasteAnimationPose(lastPose);
		blockAnimation->PlayAnimation(desiredBlockAnim, true, 0.3f);
	}

	Animator::Update();
}

AnimationPose NpcHybridAnimator::BuildCombatPose()
{
	AnimationPose locomotionPose = locomotion->GetAnimationPose();
	if (!weapon_holds) return locomotionPose;

	// ── Melee pose: block layer from spine_01 upward ──────────────────────────
	AnimationPose meleePose = AnimationPose::LayeredLerp(
		"spine_01", locomotion->GetRootNode(),
		locomotionPose, blockAnimation->GetAnimationPose(), 1, 1);

	// ── Ranged pose: pistol layer with optional aim spine rotation ────────────
	auto pistolPose = pistol->GetAnimationPose();
	hashed_string rangedStartBone = "clavicle_r";

	if (weapon_ready || weapon_aims)
	{
		spineRotation.x = glm::clamp(spineRotation.x, -70.0f, 70.0f);
		spineRotation.y = glm::clamp(spineRotation.y, -30.0f, 30.0f);

		if (weapon_aims)
		{
			pistolPose.boneTransforms["spine_02"] = pistolPose.boneTransforms["spine_02"]
				* MathHelper::GetRotationMatrix(
				    vec3(spineRotation.y, spineRotation.y * 0.3f, -spineRotation.x * 0.75f));
		}

		rangedStartBone = "spine_01";
	}

	AnimationPose rangedPose = AnimationPose::LayeredLerp(
		rangedStartBone, pistol->GetRootNode(),
		locomotionPose, pistolPose, true, 1);

	// ── Cross-fade: 0 = melee, 1 = ranged ────────────────────────────────────
	return AnimationPose::Lerp(meleePose, rangedPose, combatModeBlend);
}
