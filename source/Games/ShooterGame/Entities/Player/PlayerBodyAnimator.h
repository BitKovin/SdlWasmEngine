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


	// Only 3 poses ever evaluated per frame: idle + whichever fwd/back/slide clip is currently
	// loaded on the forward axis + whichever left/right clip is loaded on the side axis.
	Animation* forwardAxisAnim = nullptr; // holds "run", "run_b", "crouch_f", "crouch_b", or "slide"
	Animation* sideAxisAnim = nullptr;    // holds "run_r" or "run_l" (or crouch equivalents)
	Animation* idleAnim = nullptr;
	Animation* upperBody = nullptr;

	float locomotionPhase = 0.0f;  // 0..1, shared gait-cycle position across both axis clips
	float crouchDwellTime = 0.0f;  // seconds continuously crouched - gates slide entry so it always transitions through a crouch pose first

	static constexpr float kMoveThreshold = 0.1f;
	static constexpr float kBlendTime = 0.2f;
	static constexpr float kCrouchSpeedMultiplier = 2.0f;
	static constexpr float kSlideSpeedThreshold = 3.0f; // raw movement magnitude while crouched above which slide replaces crouch-walk
	static constexpr float kSlideEntryDelay = 0.1f; // must be crouched this long before slide can trigger, so the run->crouch blend finishes first

	void LoadAssets()
	{
		const char* model = "GameData/models/npc/base.glb";

		forwardAxisAnim = AddAnimation(model, "run");
		sideAxisAnim = AddAnimation(model, "run_r");
		idleAnim = AddAnimation(model, "idle");
		upperBody = AddAnimation(model, "idle");
	}

	bool ComputeIsSliding() const
	{
		float rawMagnitude = std::sqrt(relativeMovement.x * relativeMovement.x + relativeMovement.y * relativeMovement.y);
		return crouched && rawMagnitude > kSlideSpeedThreshold && crouchDwellTime >= kSlideEntryDelay;
	}

	// Picks which clip each axis slot should currently hold, based on movement sign, crouch
	// state, and slide state, and only re-triggers PlayAnimation (with crossfade) when the
	// desired clip changes.
	void UpdateMoveClipNames(float x, float y, bool isSliding)
	{
		bool wantForward = y >= 0.0f;
		const char* desiredForwardClip = isSliding
			? "slide"
			: (crouched
				? (wantForward ? "crouch_f" : "crouch_b")
				: (wantForward ? "run" : "run_b"));

		if (desiredForwardClip != forwardAxisAnim->GetAnimationName())
			forwardAxisAnim->PlayAnimation(desiredForwardClip, true, kBlendTime);

		// Side axis contributes zero weight while sliding (see ProcessResultPose), so skip
		// updating it to avoid an unnecessary clip switch.
		if (!isSliding)
		{
			bool wantRight = x >= 0.0f;
			const char* desiredSideClip = crouched
				? (wantRight ? "crouch_r" : "crouch_l")
				: (wantRight ? "run_r" : "run_l");

			if (desiredSideClip != sideAxisAnim->GetAnimationName())
				sideAxisAnim->PlayAnimation(desiredSideClip, true, kBlendTime);
		}

		const char* desiredIdle = crouched ? "crouch_idle" : "idle";
		if (desiredIdle != idleAnim->GetAnimationName())
			idleAnim->PlayAnimation(desiredIdle, true, kBlendTime);
	}

	// Keeps whichever clips are currently loaded in the 2 axis slots at the same normalized
	// point in their gait cycle, regardless of each clip's own native length. Skips the forward
	// slot while it's holding "slide" - slide is a one-shot entry/dive, not a locomotion cycle,
	// so it should play from its own natural start rather than being scrubbed to a phase
	// inherited from whatever run/crouch cycle preceded it.
	void SyncMoveClipPhase()
	{
		float cycleDuration = forwardAxisAnim->GetAnimationDuration();
		if (cycleDuration <= 0.0f)
			return;

		locomotionPhase += (Time::DeltaTime * Speed) / cycleDuration;
		locomotionPhase = std::fmodf(locomotionPhase,1.0f);

		bool forwardIsSlide = (forwardAxisAnim->GetAnimationName() == "slide");
		if (!forwardIsSlide)
			forwardAxisAnim->SetAnimationTime(locomotionPhase * forwardAxisAnim->GetAnimationDuration());

		sideAxisAnim->SetAnimationTime(locomotionPhase * sideAxisAnim->GetAnimationDuration());
	}

	void Update() override
	{
		crouchDwellTime = crouched ? (crouchDwellTime + Time::DeltaTime) : 0.0f;

		bool isSliding = ComputeIsSliding();

		if (isSliding || length(relativeMovement) < kMoveThreshold)
		{
			locomotionPhase = 0; //reset animation time when not moving. Helps solve issue where animation on shadow mesh doesn't match with visible mesh
		}

		UpdateMoveClipNames(relativeMovement.x, relativeMovement.y, isSliding);
		SyncMoveClipPhase();

		Speed = crouched ? kCrouchSpeedMultiplier : 1.0f;

		Animator::Update();
	}

	AnimationPose ProcessResultPose()
	{
		bool isSliding = ComputeIsSliding();

		float x = relativeMovement.x;
		float y = relativeMovement.y;
		float magnitude = std::sqrt(x * x + y * y);

		float targetSpeed = crouched ? 2.5f : 6.0f;

		AnimationPose pose;

		if (isSliding)
		{
			// Fully committed slide pose - bypasses idle/forward/side blending entirely,
			// since a slide is a discrete state rather than something proportional to input.
			pose = forwardAxisAnim->GetAnimationPose();
		}
		else
		{
			if (magnitude < kMoveThreshold)
			{
				x = 0.0f;
				y = 0.0f;
			}
			else
			{
				x /= targetSpeed;
				y /= targetSpeed;
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

			pose = idleAnim->GetAnimationPose();
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
		}

		pose = AnimationPose::LayeredLerp("spine_01", idleAnim->GetRootNode(), pose, upperBody->GetAnimationPose(), 0.5, 0.5);
		pose = AnimationPose::LayeredLerp("spine_02", idleAnim->GetRootNode(), pose, upperBody->GetAnimationPose(), 1.0, 1.0);

		return pose;
	}

};