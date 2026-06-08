#pragma once

#include <Animator.h>

class NpcAnimatorBase : public Animator
{
public:

	NpcAnimatorBase(Entity* ownerEntity) : Animator(ownerEntity) {}

	float movementSpeed = 0;
	float PainProgress  = 0;

	bool weapon_holds        = false;
	bool weapon_ready        = false;
	bool weapon_aims         = false;
	bool weapon_pendingAttack = false;

	bool scared = false;

	vec3 spineRotation = vec3();

	void Update() override;

	// ── Task animation ──────────────────────────────────────────────────────
	void PlayTaskAnimation(std::string animationName, bool loop);
	void StopTaskAnimation();
	Animation* taskAnimation = nullptr;

	// ── Action animation ────────────────────────────────────────────────────
	// Available on every NPC type (stun, special moves, root-motion attacks).
	// NpcBase exposes a thin convenience wrapper that delegates here.
	bool  IsPlayingActionAnimation() const;
	void  PlayActionAnimation(const std::string& animationName, bool loop,
	                          float blendIn = 0.3f, float blendOut = 0.3f);
	void  StopActionAnimation();
	float GetActionAnimationRemainingTime() const;

	float     actionBlendOut   = 0.3f;
	Animation* actionAnimation = nullptr;

protected:

	Animation* locomotion = nullptr;
	Animation* pistol     = nullptr;
	Animation* inPain     = nullptr;

	virtual void LoadAssets();

	// ── Template-method pattern ─────────────────────────────────────────────
	//
	//  ProcessResultPose is sealed here so that task and action overlays are
	//  applied consistently for every NPC type, regardless of weapon mode.
	//
	//  Subclasses implement BuildCombatPose() to supply their weapon-layer
	//  pose (ranged, melee, or a blend of both).  They must NOT override
	//  ProcessResultPose directly.
	//
	AnimationPose ProcessResultPose() override final;
	virtual AnimationPose BuildCombatPose();
};
