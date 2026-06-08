#pragma once

#include "NpcBase.h"
#include "Animators/NpcMeleeAnimator.h"

#include <RandomHelper.h>
#include <Entities/Player/Player.hpp>
#include <Entities/Enemy/Enemies/NpcHelper.h>

// ─────────────────────────────────────────────────────────────────────────────
//  NpcGuardMelee
//
//  Melee combat state machine overview
//
//   IDLE        – disengaged; waits until target enters attackRange
//   APPROACHING – closing the distance to enter melee range
//   PRESSURE    – tight-circling the target, probing for an opening
//   WIND_UP     – committed telegraph before a swing (can't cancel)
//   RECOVERING  – post-attack vulnerability window
//   RETREATING  – resetting spacing after an exchange
//   BLOCKING    – active parry raised in response to WarnAboutAttack
//   PUNISHING   – rapid counter immediately after a successful block
//   BAITING     – deliberately inviting an attack in order to punish it
// ─────────────────────────────────────────────────────────────────────────────

class NpcGuardMelee : public NpcBase
{
protected:

	enum class MeleeState
	{
		IDLE, APPROACHING, PRESSURE,
		WIND_UP, RECOVERING, RETREATING,
		BLOCKING, PUNISHING, BAITING
	};

	// ── State machine fields ──────────────────────────────────────────────────
	Delay      stunDelay;
	MeleeState meleeState = MeleeState::IDLE;
	Delay      stateTimer;
	Delay      retreatPosTimer;

	bool  pendingCounterAttack  = false;
	bool  isCurrentlyBlocking   = false;
	int   playerBlockCount      = 0;
	Delay blockCountDecayTimer;

	bool  playerWasBlockingPrev = false;
	Delay continuousBlockTimer;
	bool  nextAttackIsBreak     = false;

	int   timesParriedThisFight = 0;

	// ── State transition helpers ──────────────────────────────────────────────
	inline void EnterState(MeleeState next, float duration = 0.0f)
	{
		meleeState = next;
		if (duration > 0.0f) stateTimer.AddDelay(duration);
	}

	inline float RandomPressureDuration() const
	{
		float base = 0.9f + RandomHelper::RandomFloat() * 1.6f;
		base -= std::min(timesParriedThisFight * 0.12f, 0.55f);
		return std::max(base, 0.35f);
	}

	// ── State machine ─────────────────────────────────────────────────────────
	void CommitAttack();
	void UpdateMeleeStateMachine();

	// ── Actions ───────────────────────────────────────────────────────────────
	void Stun();
	void DoNormalAttackDamage();
	void DoBreakAttackDamage();
	void PerformMeleeAttack();
	void PerformBaitingMeleeAttack();
	void PerformBreakingAttack();
	void OnParried();
	void StartBlock();
	void StopBlock();

	// ── Queries ───────────────────────────────────────────────────────────────
	inline bool InMeleeAttackRange() const
	{
		Entity* t = Level::Current->FindEntityWithId(target_id);
		return t && glm::distance(Position, t->Position) < 2.0f;
	}

	inline bool CanAttackOrBlock() { return !isStunned(); }

	bool IsTargetBlocking() const;

	// ── Animation ─────────────────────────────────────────────────────────────
	void ProcessAnimationEvent(const AnimationEvent& event);
	void UpdateAnimations(bool forceFullUpdate = false) override;

	// Convenience accessor – safe to call from derived classes that also use
	// a NpcMeleeAnimator (or subclass thereof).
	inline NpcMeleeAnimator* GetMeleeAnimator() const
	{
		return static_cast<NpcMeleeAnimator*>(animator.get());
	}

public:

	NpcGuardMelee();

	bool isStunned()                        override;
	bool IsPlayingRootMotionAnimation()     override;
	bool IsRotationAllowedDuringRootMotion() override;

	void WarnAboutAttack(Entity* from)      override;
	void UpdateTargetAttack()               override;
	vec3 FindAttackLocation()               override;
};
