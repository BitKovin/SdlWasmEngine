#pragma once
#include <Animator.h>
#include <cmath>
#include <algorithm>

class PlayerBodyAnimator : public Animator
{
public:

	PlayerBodyAnimator(Entity* ownerEntity) : Animator(ownerEntity) {}

	vec2 relativeMovement{}; // x-right y-forward
	bool crouched = false;


	// Only 3 poses ever evaluated per frame: idle + whichever fwd/back clip is currently
	// loaded on the forward axis + whichever left/right clip is loaded on the side axis.
	Animation* forwardAxisAnim = nullptr; // holds "run" or "run_b" (or crouch equivalents)
	Animation* sideAxisAnim = nullptr;    // holds "run_r" or "run_l" (or crouch equivalents)
	Animation* idleAnim = nullptr;
	Animation* upperBody = nullptr;

	float locomotionPhase = 0.0f; // 0..1, shared gait-cycle position across both axis clips

	static constexpr float kMoveThreshold = 0.1f;
	static constexpr float kFullMoveMagnitude = 3.0f;
	static constexpr float kBlendTime = 0.2f;
	static constexpr float kCrouchSpeedMultiplier = 2.0f;

	void LoadAssets()
	{
		const char* model = "GameData/models/npc/base.glb";

		forwardAxisAnim = AddAnimation(model, "run");
		sideAxisAnim = AddAnimation(model, "run_r");
		idleAnim = AddAnimation(model, "idle");
		upperBody = AddAnimation(model, "idle");
	}

	// Picks which clip each axis slot should currently hold, based on movement sign and crouch
	// state, and only re-triggers PlayAnimation (with crossfade) when the desired clip changes.
	void UpdateMoveClipNames(float x, float y)
	{
		bool wantForward = y >= 0.0f;
		const char* desiredForwardClip = crouched
			? (wantForward ? "crouch_f" : "crouch_b")
			: (wantForward ? "run" : "run_b");

		if (desiredForwardClip != forwardAxisAnim->GetAnimationName())
			forwardAxisAnim->PlayAnimation(desiredForwardClip, true, kBlendTime);

		bool wantRight = x >= 0.0f;
		const char* desiredSideClip = crouched
			? (wantRight ? "crouch_r" : "crouch_l")
			: (wantRight ? "run_r" : "run_l");

		if (desiredSideClip != sideAxisAnim->GetAnimationName())
			sideAxisAnim->PlayAnimation(desiredSideClip, true, kBlendTime);

		const char* desiredIdle = crouched ? "crouch_idle" : "idle";
		if (desiredIdle != idleAnim->GetAnimationName())
			idleAnim->PlayAnimation(desiredIdle, true, kBlendTime);
	}

	// Keeps whichever clips are currently loaded in the 2 axis slots at the same normalized
	// point in their gait cycle, regardless of each clip's own native length.
	void SyncMoveClipPhase()
	{
		float cycleDuration = forwardAxisAnim->GetAnimationDuration();
		if (cycleDuration <= 0.0f)
			return;

		locomotionPhase += (Time::DeltaTime * Speed) / cycleDuration;
		locomotionPhase -= std::floor(locomotionPhase);

		forwardAxisAnim->SetAnimationTime(locomotionPhase * forwardAxisAnim->GetAnimationDuration());
		sideAxisAnim->SetAnimationTime(locomotionPhase * sideAxisAnim->GetAnimationDuration());
	}

	void Update() override
	{
		UpdateMoveClipNames(relativeMovement.x, relativeMovement.y);
		SyncMoveClipPhase();

		Speed = crouched ? kCrouchSpeedMultiplier : 1.0f;

		Animator::Update();
	}

	AnimationPose ProcessResultPose()
	{
		float x = relativeMovement.x;
		float y = relativeMovement.y;
		float magnitude = std::sqrt(x * x + y * y);

		if (magnitude < kMoveThreshold)
		{
			x = 0.0f;
			y = 0.0f;
		}
		else
		{
			x /= kFullMoveMagnitude;
			y /= kFullMoveMagnitude;
		}

		float forwardWeight = std::fabs(y);
		float sideWeight = std::fabs(x);
		float moveSum = forwardWeight + sideWeight;

		float idleWeight;
		if (moveSum > 1.0f)
		{
			float inv = 1.0f / moveSum;
			forwardWeight *= inv;
			sideWeight *= inv;
			idleWeight = 0.0f;
		}
		else
		{
			idleWeight = 1.0f - moveSum;
		}

		AnimationPose pose = idleAnim->GetAnimationPose();
		float accumulated = idleWeight;

		auto blendIn = [&](Animation* anim, float weight)
			{
				if (weight <= 0.0f) return;
				float total = accumulated + weight;
				pose = AnimationPose::Lerp(pose, anim->GetAnimationPose(), weight / total);
				accumulated = total;
			};

		blendIn(forwardAxisAnim, forwardWeight);
		blendIn(sideAxisAnim, sideWeight);

		pose = AnimationPose::LayeredLerp("spine_01", idleAnim->GetRootNode(), pose, upperBody->GetAnimationPose(), 0.5, 0.5);
		pose = AnimationPose::LayeredLerp("spine_02", idleAnim->GetRootNode(), pose, upperBody->GetAnimationPose(), 1.0, 1.0);

		return pose;
	}

};