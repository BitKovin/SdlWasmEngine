#include "NpcBase.h"

#include <RandomHelper.h>
#include <Entities/Player/Player.hpp>
#include <Entities/Player/Weapons/Projectiles/Bullet.h>
#include "Animators/NpcMeleeAnimator.h"

#include <Entities/Enemy/Enemies/NpcHelper.h>

class NpcGuardMelee : public NpcBase
{
	// ─────────────────────────────────────────────────────────────────────────
	//  Melee combat state machine
	// ─────────────────────────────────────────────────────────────────────────
	//
	//  States and their roles:
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
	//
	//  The key behavioural loops:
	//
	//   Normal exchange  : PRESSURE → WIND_UP → RECOVERING → (RETREATING →) PRESSURE
	//   Punish on block  : WarnAboutAttack → BLOCKING → PUNISHING → RECOVERING
	//   Pattern break    : repeated player blocks accumulate playerBlockCount;
	//                      CommitAttack() escalates to Feint/GuardBreak accordingly
	//   Bait loop        : PRESSURE → BAITING → (player attacks) → PUNISHING
	//                                        → (player ignores) → WIND_UP (aggression)



	enum class MeleeState
	{
		IDLE,
		APPROACHING,
		PRESSURE,
		WIND_UP,
		RECOVERING,
		RETREATING,
		BLOCKING,
		PUNISHING,
		BAITING
	};

	// ── member variables ──────────────────────────────────────────────────────

protected:

	Delay stunDelay;

	MeleeState meleeState = MeleeState::IDLE;
	Delay      stateTimer;           // general-purpose countdown for the active state
	Delay      retreatPosTimer;      // throttle retreat-position resampling

	bool  pendingCounterAttack = false;  // set by WarnAboutAttack; consumed by state machine
	bool  isCurrentlyBlocking = false;

	// How many of our attacks the player has blocked without us switching strategy.
	// Drives CommitAttack() to escalate: normal → feint → guardbreak.
	int   playerBlockCount = 0;
	Delay blockCountDecayTimer;  // slowly forgive old blocks so the cycle can restart

	// Passive-block punishment.
	// If the player just holds block without attacking, the guard recognises it
	// and forces a breaking attack to stun them out of it.
	bool  playerWasBlockingPrev = false;
	Delay continuousBlockTimer;          // armed when player raises block; expires = punish
	bool  nextAttackIsBreak = false; // CommitAttack() reads this to override normal escalation

	// Parry tracking.
	// OnParried() is called by animation code when the guard's swing is perfectly parried.
	// Being parried repeatedly makes the guard more desperate (shorter pressure, higher block
	// chance) and extends the stagger window to reward skilled players.
	int   timesParriedThisFight = 0;

	// ── state transition helper ───────────────────────────────────────────────

	void EnterState(MeleeState next, float duration = 0.0f)
	{
		meleeState = next;
		if (duration > 0.0f)
			stateTimer.AddDelay(duration);
	}

	// How long to circle before forcing an action.  Randomised to prevent rhythm-reading.
	// Shrinks as parry count rises: a guard who keeps getting parried grows impatient
	// and commits faster, making them more dangerous but also more readable.
	float RandomPressureDuration() const
	{
		float base = 0.9f + RandomHelper::RandomFloat() * 1.6f;
		base -= std::min(timesParriedThisFight * 0.12f, 0.55f);
		return std::max(base, 0.35f);
	}

	// ── attack commitment ─────────────────────────────────────────────────────
	//
	//  Priority order:
	//   1. nextAttackIsBreak (passive-block punishment, set by continuous-block detection)
	//      → PerformBreakingAttack, or a PerformBaitingMeleeAttack feint into it.
	//   2. playerBlockCount escalation (punish a player who actively parried our attacks)
	//      0-1 blocks : normal swing
	//        2 blocks : feint (if target is blocking right now)
	//       3+ blocks : outright guardbreak, counter reset
	//   3. Normal attack.

	void CommitAttack()
	{
		if (!CanAttackOrBlock()) return;

		// ── Priority 1: punish passive blocking ───────────────────────────────
		if (nextAttackIsBreak)
		{
			nextAttackIsBreak = false;
			// 45 % chance to feint into the break (harder to react to);
			// 55 % chance to go direct (faster, less telegraphed).
			if (RandomHelper::RandomFloat() < 0.45f)
				PerformBaitingMeleeAttack();
			else
				PerformBreakingAttack();
			return;
		}

		// ── Priority 2: escalate on repeated active blocks ────────────────────
		// Gradually forgive old blocks so the NPC doesn't stay on guardbreak forever
		if (!blockCountDecayTimer.Wait())
		{
			playerBlockCount = std::max(0, playerBlockCount - 1);
			blockCountDecayTimer.AddDelay(4.0f);
		}

		const bool targetIsBlocking = IsTargetBlocking();

		if (targetIsBlocking)
		{
			PerformBreakingAttack();
			playerBlockCount = 0;
		}
		else if (playerBlockCount >= 2 && targetIsBlocking)
		{
			PerformBaitingMeleeAttack();
		}
		else
		{
			PerformMeleeAttack();
		}

		// If the player was blocking when we connected, note the failure for next time
		if (targetIsBlocking)
			playerBlockCount++;
	}

	// ── state machine tick ────────────────────────────────────────────────────

	void UpdateMeleeStateMachine()
	{
		Entity* targetRef = Level::Current->FindEntityWithId(target_id);
		if (!targetRef) return;

		const bool inRange = InMeleeAttackRange();
		const bool blocking = IsTargetBlocking();

		// ── Passive-block detection (runs every frame, outside the switch) ────
		//
		//  If the player raises their block, we arm a patience timer.  When it
		//  expires while they're still blocking, we flag the next attack as a
		//  breaking attack to punish the turtling.  The window is shorter when
		//  the guard is already irritated from being parried.
		{
			if (blocking && !playerWasBlockingPrev)
			{
				// Player just raised block – start the patience countdown
				float patience = 0.75f + RandomHelper::RandomFloat() * 0.55f;
				patience -= std::min(timesParriedThisFight * 0.08f, 0.35f);
				patience = std::max(patience, 0.28f);
				continuousBlockTimer.AddDelay(patience);
			}

			if (!blocking)
			{
				// Player dropped block – cancel any pending break intent
				nextAttackIsBreak = false;
			}

			// Patience expired while player is still blocking: force a break
			if (blocking && !continuousBlockTimer.Wait() &&
				(meleeState == MeleeState::PRESSURE || meleeState == MeleeState::APPROACHING))
			{
				nextAttackIsBreak = true;
				// Arm the timer again so we don't spam breaks every frame
				continuousBlockTimer.AddDelay(1.8f + RandomHelper::RandomFloat() * 0.8f);
				EnterState(MeleeState::WIND_UP, 0.14f + RandomHelper::RandomFloat() * 0.20f);
			}

			playerWasBlockingPrev = blocking;
		}

		switch (meleeState)
		{

			// ─────────────────────────────────────────────────────────────────────
		case MeleeState::IDLE:
		{
			if (target_attackInRange)
				EnterState(MeleeState::APPROACHING);
			break;
		}

		// ─────────────────────────────────────────────────────────────────────
		case MeleeState::APPROACHING:
		{
			if (!target_attackInRange)
			{
				EnterState(MeleeState::IDLE);
				break;
			}

			// Refresh the target position often while closing in
			if (!attackPositionUpdateDelay.Wait())
			{
				attackPosition = FindAttackLocation();
				attackPositionUpdateDelay.AddDelay(0.28f);
			}

			if (inRange)
				EnterState(MeleeState::PRESSURE, RandomPressureDuration());

			break;
		}

		// ─────────────────────────────────────────────────────────────────────
		// Core fencing loop: circle close, look for an opening, decide on action.
		case MeleeState::PRESSURE:
		{
			if (!target_attackInRange)
			{
				EnterState(MeleeState::APPROACHING);
				break;
			}

			// Keep circling while in this state
			if (!attackPositionUpdateDelay.Wait())
			{
				attackPosition = FindAttackLocation();
				attackPositionUpdateDelay.AddDelay(0.30f + RandomHelper::RandomFloat() * 0.25f);
			}

			// A successful block just happened – immediately punish, don't wait
			if (pendingCounterAttack)
			{
				pendingCounterAttack = false;
				EnterState(MeleeState::PUNISHING, 0.12f);
				break;
			}

			// Pressure timer expired: commit to some action
			if (!stateTimer.Wait())
			{
				// 25 % chance to bait instead of swinging directly
				if (RandomHelper::RandomFloat() < 0.25f)
					EnterState(MeleeState::BAITING, 0.38f + RandomHelper::RandomFloat() * 0.55f);
				else
					EnterState(MeleeState::WIND_UP, 0.18f + RandomHelper::RandomFloat() * 0.32f);
				break;
			}

			// Opportunistic early strike: target isn't blocking AND barely moving
			if (inRange && !blocking)
			{
				// ~1 % per frame chance; feels human, not robotic
				if (RandomHelper::RandomFloat() < 0.011f)
					EnterState(MeleeState::WIND_UP, 0.12f + RandomHelper::RandomFloat() * 0.18f);
			}

			break;
		}

		// ─────────────────────────────────────────────────────────────────────
		// Committed telegraph.  NPC freezes briefly; the player has a window
		// to react.  This is the "honest" signal before the hit lands.
		case MeleeState::WIND_UP:
		{
			attackPosition = Position; // freeze in place during wind-up

			if (!stateTimer.Wait())
			{
				if (inRange)
					CommitAttack();

				float recovery = 0.55f + RandomHelper::RandomFloat() * 0.75f;
				EnterState(MeleeState::RECOVERING, recovery);
			}
			break;
		}

		// ─────────────────────────────────────────────────────────────────────
		// Post-attack window.  The NPC is open here – this is the reward for
		// reading the wind-up correctly.
		case MeleeState::RECOVERING:
		{
			attackPosition = Position; // stay put while recovering

			if (!stateTimer.Wait())
			{
				// More likely to retreat if we keep getting blocked or parried;
				// a rattled guard needs space to reassess their strategy.
				float retreatChance = 0.38f
					+ playerBlockCount * 0.12f
					+ timesParriedThisFight * 0.08f;
				retreatChance = std::min(retreatChance, 0.78f);

				if (RandomHelper::RandomFloat() < retreatChance)
					EnterState(MeleeState::RETREATING, 0.70f + RandomHelper::RandomFloat() * 0.55f);
				else
					EnterState(MeleeState::PRESSURE, RandomPressureDuration());
			}
			break;
		}

		// ─────────────────────────────────────────────────────────────────────
		// Stepped back to reset the duel's spacing.
		case MeleeState::RETREATING:
		{
			// Resample retreat destination periodically in case geometry blocks the path
			if (!retreatPosTimer.Wait())
			{
				attackPosition = FindAttackLocation();
				retreatPosTimer.AddDelay(0.35f);
			}

			if (!stateTimer.Wait())
			{
				if (target_attackInRange)
					EnterState(MeleeState::PRESSURE, RandomPressureDuration());
				else
					EnterState(MeleeState::APPROACHING);
			}
			break;
		}

		// ─────────────────────────────────────────────────────────────────────
		// Active block held after WarnAboutAttack fires.
		// The block window is deliberately short so it can't be exploited
		// as a passive permanent defence.
		case MeleeState::BLOCKING:
		{
			attackPosition = Position; // locked in place while blocking

			if (!stateTimer.Wait())
			{
				StopBlock();
				isCurrentlyBlocking = false;

				// If the block succeeded, punish.  pendingCounterAttack was set
				// by WarnAboutAttack when we decided to raise the block.
				if (pendingCounterAttack)
				{
					pendingCounterAttack = false;
					EnterState(MeleeState::PUNISHING, 0.10f);
				}
				else
				{
					// We raised the block but the attack may not have landed yet;
					// keep pressure short so we don't just stand around.
					EnterState(MeleeState::PRESSURE, RandomPressureDuration() * 0.55f);
				}
			}
			break;
		}

		// ─────────────────────────────────────────────────────────────────────
		// Rapid counter-attack directly after a successful block.
		// Represents the classic parry-riposte: brief delay then immediate hit.
		case MeleeState::PUNISHING:
		{
			attackPosition = Position;

			if (!stateTimer.Wait())
			{
				if (inRange && CanAttackOrBlock())
					PerformMeleeAttack(); // always a direct, fast strike – not escalated

				EnterState(MeleeState::RECOVERING, 0.42f + RandomHelper::RandomFloat() * 0.32f);
			}
			break;
		}

		// ─────────────────────────────────────────────────────────────────────
		// Deliberately lower guard to invite an attack.  A skilled player
		// recognises the tell and holds off; a predictable player swings and
		// gets punished hard.  Either way the NPC wins the exchange.
		case MeleeState::BAITING:
		{
			attackPosition = Position; // stand conspicuously still

			// Player took the bait – punish with minimal delay
			if (pendingCounterAttack)
			{
				pendingCounterAttack = false;
				EnterState(MeleeState::PUNISHING, 0.07f); // near-instant retaliation
				break;
			}

			if (!stateTimer.Wait())
			{
				// Player didn't bite – press forward aggressively as a follow-up
				EnterState(MeleeState::WIND_UP, 0.10f + RandomHelper::RandomFloat() * 0.14f);
			}
			break;
		}

		} // end switch
	}

	void Stun()
	{
		PlayActionAnimation("stun",0.2,1.2);
		attackDelay.AddDelay(GetActionAnimationRemainingTime());
		stunDelay.AddDelay(GetActionAnimationRemainingTime());
	}

	void DoNormalAttackDamage()
	{

		vec3 start = Position + vec3(0,0.5f,0);

		auto hit = Physics::SphereTrace(start, start + MathHelper::GetForwardVector(mesh->Rotation), 0.3, BodyType::GroupHitTest, {}, {this});

		if (hit.hasHit)
		{
			if (hit.entity->HasTag("player"))
			{
				if (NpcHelper::CheckParry(MathHelper::GetForwardVector(mesh->Rotation), hit.entity))
				{
					OnParried();
					Stun();
					return;
				}
				else
				{
					hit.entity->OnPointDamage(15, hit.shapePosition, MathHelper::FastNormalize(hit.shapePosition - Position), hit.hitboxName, this, this);
					
				}
			}
		}

	}

	void DoBreakAttackDamage()
	{

		vec3 start = Position + vec3(0, 0.5f, 0);

		auto hit = Physics::SphereTrace(start, start + MathHelper::GetForwardVector(mesh->Rotation), 0.3, BodyType::GroupHitTest, {}, { this });

		if (hit.hasHit)
		{
			if (hit.entity->HasTag("player"))
			{
				hit.entity->OnPointDamage(10, hit.shapePosition, MathHelper::FastNormalize(hit.shapePosition - Position), hit.hitboxName, this, this);
			}
		}

	}


	void ProcessAnimationEvent(const AnimationEvent& event)
	{
		if (event.eventName == "attack_sword_normal")
		{
			DoNormalAttackDamage();
		}
		else if (event.eventName == "attack_sword_break")
		{
			DoBreakAttackDamage();
		} 
		else if (event.eventName == "attack_kick")
		{
			DoBreakAttackDamage();
		}
	}

	void UpdateAnimations(bool forceFullUpdate = false) override
	{

		auto meleeAnimator = static_cast<NpcMeleeAnimator*>(animator.get());
		meleeAnimator->actionAnimation->Rotation = mesh->Rotation;


		NpcBase::UpdateAnimations(forceFullUpdate);

		if (controller == nullptr) return;

		vec3 currPos = controller->GetPosition();
		auto rootMotion = meleeAnimator->actionAnimation->PullRootMotion();
		controller->SetPosition(currPos + rootMotion.Position);

		auto events = meleeAnimator->PullAnimationEvents();

		for (auto& event : events)
		{
			ProcessAnimationEvent(event);
		}

	}



public:

	bool IsPlayingRootMotionAnimation() override
	{
		auto meleeAnimator = static_cast<NpcMeleeAnimator*>(animator.get());

		return meleeAnimator->actionAnimation->IsAnimationPlaying();
	}

	bool IsRotationAllowedDuringRootMotion() override
	{
		return attackDelay.Wait();
	}

	bool isStunned() override
	{
		return NpcBase::isStunned() || stunDelay.Wait();
	}

	NpcGuardMelee()
	{
		isGuard = true;
		modelPath = "GameData/models/npc/guard.glb";
		hostileTags.insert("bandit");

		attackRange = 6;   // start approach logic outside this radius
		attackDesiredRange = 2.0f; // tight melee engagement radius

		animator = std::make_unique<NpcMeleeAnimator>(this);

	}


	void PlayActionAnimation(std::string name, float blendIn = 0.3f, float blendOut = 0.3f)
	{
		auto meleeAnimator = static_cast<NpcMeleeAnimator*>(animator.get());
		meleeAnimator->PlayActionAnimation(name, false, blendIn, blendOut);
	}

	float GetActionAnimationRemainingTime()
	{
		auto meleeAnimator = static_cast<NpcMeleeAnimator*>(animator.get());

		if (meleeAnimator->actionAnimation->IsAnimationPlaying() == false) return 0;

		return meleeAnimator->actionAnimation->GetAnimationDuration() - meleeAnimator->actionAnimation->GetAnimationTime();

	}

	void PerformMeleeAttack()
	{

		Logger::Log("PerformMeleeAttack");
		PlayActionAnimation("sword_attack_main");

		float duration = GetActionAnimationRemainingTime();

		attackDelay.AddDelay(duration);
	}

	void PerformBaitingMeleeAttack()
	{
		Logger::Log("PerformBaitingMeleeAttack");
		PlayActionAnimation("sword_attack_break");
		attackDelay.AddDelay(GetActionAnimationRemainingTime());
	}

	void PerformBreakingAttack()
	{
		Logger::Log("PerformBreakingAttack");
		PlayActionAnimation("sword_kick");
		attackDelay.AddDelay(GetActionAnimationRemainingTime());
	}

	bool IsTargetBlocking()
	{
		Entity* targetRef = Level::Current->FindEntityWithId(target_id);
		if (targetRef == nullptr) return false;

		if (targetRef == Player::Instance)
		{
			auto mainWeapon = Player::Instance->currentWeapon;
			auto offhandWeapon = Player::Instance->currentOffhandWeapon;

			if (mainWeapon && mainWeapon->Blocking)    return true;
			if (offhandWeapon && offhandWeapon->Blocking) return true;
		}

		return false;
	}

	bool CanAttackOrBlock()
	{
		if (isStunned()) return false;
		// Also returns false if already in attack animation
		return true;
	}

	bool InMeleeAttackRange()
	{
		Entity* targetRef = Level::Current->FindEntityWithId(target_id);
		if (targetRef == nullptr) return false;
		return glm::distance(Position, targetRef->Position) < 2.0f;
	}

	void StartBlock() {}
	void StopBlock() {}

	// ─────────────────────────────────────────────────────────────────────────
	//  OnParried – called by animation code when this NPC's attack is perfectly
	//  parried by the player.
	//
	//  A parry is harder to land than a block and deserves a bigger punish window.
	//  Repeated parries make the guard more desperate (shorter pressure, higher block
	//  chance) but also extend the stagger each time, rewarding a skilled player who
	//  commits to the parry read.
	// ─────────────────────────────────────────────────────────────────────────

	void OnParried()
	{
		timesParriedThisFight++;

		// Clear any pending intentions – we're staggered, not attacking
		pendingCounterAttack = false;
		nextAttackIsBreak = false;

		// Stagger duration grows with repeated parries (up to a cap).
		// First parry is a modest opening; by the third the guard is badly rattled.
		float stagger = 0.85f
			+ RandomHelper::RandomFloat() * 0.45f
			+ timesParriedThisFight * 0.10f;
		stagger = std::min(stagger, 1.90f);

		EnterState(MeleeState::RECOVERING, stagger);
		attackPosition = Position;
	}

	// ─────────────────────────────────────────────────────────────────────────
	//  React to an incoming player attack.
	//
	//  Called externally when the player's weapon swing starts.  The guard has a
	//  state-dependent chance to raise a block; if it does, it primes
	//  pendingCounterAttack so the state machine can riposte immediately after.
	//
	//  Being parried repeatedly raises the base block chance: the guard has
	//  learned to respect the player's timing.
	// ─────────────────────────────────────────────────────────────────────────

	void WarnAboutAttack(Entity* from) override
	{
		if (from->Id != target_id) return;

		// Must be facing the attacker to block
		vec3 forward = MathHelper::GetForwardVector(mesh->Rotation);
		vec3 attackerForward = MathHelper::GetForwardVector(from->Rotation);
		if (glm::dot(forward, attackerForward) > -0.6f) return;

		// Block likelihood depends on what the NPC is currently doing.
		float blockChance;
		switch (meleeState)
		{
		case MeleeState::BAITING:    blockChance = 0.92f; break; // we were waiting for this
		case MeleeState::PRESSURE:   blockChance = 0.55f; break; // alert, watching closely
		case MeleeState::IDLE:
		case MeleeState::APPROACHING:blockChance = 0.40f; break; // attentive but not primed
		case MeleeState::RECOVERING: blockChance = 0.18f; break; // caught mid-recovery
		case MeleeState::WIND_UP:    blockChance = 0.08f; break; // fully committed to swing
		default:                     blockChance = 0.30f; break;
		}

		// A guard who has been parried learns to respect the player's timing
		blockChance += std::min(timesParriedThisFight * 0.06f, 0.22f);

		if (RandomHelper::RandomFloat() > blockChance) return;

		// Raise the block and prime the counter-attack
		StartBlock();
		isCurrentlyBlocking = true;
		pendingCounterAttack = true;

		// Short block window – blocking isn't a sustained passive stance
		float blockDuration = 0.20f + RandomHelper::RandomFloat() * 0.18f;
		EnterState(MeleeState::BLOCKING, blockDuration);
	}

	// ─────────────────────────────────────────────────────────────────────────
	//  UpdateTargetAttack – main per-frame entry point for combat logic.
	//
	//  Handles spine rotation toward the target and drives the state machine.
	//  All actual attack decisions live inside UpdateMeleeStateMachine().
	// ─────────────────────────────────────────────────────────────────────────

	void UpdateTargetAttack() override
	{
		// Always track spine rotation toward the target for natural body posture
		vec3 desiredSpineRotation = vec3();
		if (target_follow)
		{
			Entity* targetRef = Level::Current->FindEntityWithId(target_id);
			if (targetRef)
			{
				vec3 toTarget = glm::normalize(targetRef->Position - Position);
				vec3 localVec = MathHelper::RotateVector(toTarget, mesh->Rotation * -1.0f);
				desiredSpineRotation = MathHelper::FindLookAtRotation(vec3(), localVec);
			}
		}
		spineRotation = glm::mix(spineRotation, desiredSpineRotation, Time::DeltaTimeF * 10.0f);

		// Drop out of combat if the necessary conditions aren't met
		if (!target_follow || !target_sees || !target_attack || isStunned() || DoingTask)
		{
			// Disengage cleanly: stop any active block and reset to idle
			if (isCurrentlyBlocking)
			{
				StopBlock();
				isCurrentlyBlocking = false;
			}
			pendingCounterAttack = false;
			nextAttackIsBreak = false;
			playerWasBlockingPrev = false;
			EnterState(MeleeState::IDLE);
			return;
		}

		if (attackDelay.Wait()) return;

		UpdateMeleeStateMachine();
	}

	// ─────────────────────────────────────────────────────────────────────────
	//  FindAttackLocation – returns the world-space position the NPC should
	//  navigate toward.  The result is consumed by the movement system every
	//  time attackPositionUpdateDelay (or retreatPosTimer) fires.
	//
	//  Behaviour varies by state:
	//   • RETREATING              → a position directly behind the NPC
	//   • WIND_UP/RECOVERING/etc. → current position (stay frozen)
	//   • PRESSURE/APPROACHING   → a flanking position at melee distance
	// ─────────────────────────────────────────────────────────────────────────

	vec3 FindAttackLocation() override
	{
		Entity* targetEntity = Level::Current->FindEntityWithId(target_id);
		if (!targetEntity) return Position;

		const vec3& targetPos = targetEntity->Position;

		// ── Retreating: step directly away ───────────────────────────────────
		if (meleeState == MeleeState::RETREATING)
		{
			vec3  awayDir = MathHelper::Normalized(MathHelper::XZ(Position - targetPos));
			float dist = 3.2f + RandomHelper::RandomFloat() * 2.0f;
			vec3  candidate = Position + awayDir * dist;

			auto hit = Physics::CylinderTrace(Position, candidate, 0.5f, 0.8f,
				BodyType::World | BodyType::MainBody);
			if (!hit.hasHit) return candidate;

			// Wall directly behind us – try escaping perpendicular instead
			vec3 perpRight = MathHelper::Normalized(glm::cross(awayDir, vec3(0, 1, 0)));
			for (float side : { 1.0f, -1.0f })
			{
				vec3 perpCandidate = Position + perpRight * side * 2.4f;
				auto pH = Physics::CylinderTrace(Position, perpCandidate, 0.5f, 0.8f,
					BodyType::World | BodyType::MainBody);
				if (!pH.hasHit) return perpCandidate;
			}
			return Position; // cornered – hold ground
		}

		if (meleeState == MeleeState::IDLE)
		{
			vec3  awayDir = MathHelper::Normalized(MathHelper::XZ(Position - targetPos));
			float dist = 4.f;
			vec3  candidate = Position + awayDir * dist;

			auto hit = Physics::CylinderTrace(Position, candidate, 0.5f, 0.8f,
				BodyType::World | BodyType::MainBody);
			if (!hit.hasHit) return candidate;

			// Wall directly behind us – try escaping perpendicular instead
			vec3 perpRight = MathHelper::Normalized(glm::cross(awayDir, vec3(0, 1, 0)));
			for (float side : { 1.0f, -1.0f })
			{
				vec3 perpCandidate = Position + perpRight * side * 2.4f;
				auto pH = Physics::CylinderTrace(Position, perpCandidate, 0.5f, 0.8f,
					BodyType::World | BodyType::MainBody);
				if (!pH.hasHit) return perpCandidate;
			}
			return Position; // cornered – hold ground
		}

		// ── Frozen states: don't navigate ────────────────────────────────────
		if (meleeState == MeleeState::WIND_UP ||
			meleeState == MeleeState::RECOVERING ||
			meleeState == MeleeState::BLOCKING ||
			meleeState == MeleeState::PUNISHING ||
			meleeState == MeleeState::BAITING)
		{
			return Position;
		}

		// ── PRESSURE / APPROACHING: circle at tight melee distance ────────────
		//
		//  Angular bias mirrors realistic fencing footwork:
		//   40 % left flank | 40 % right flank | 10 % behind | 10 % front
		//
		//  Positions are scored for flanking angle, tangential movement, and
		//  avoidance of clustering with ally NPCs.

		const float meleeRadius = 1.70f;

		vec3 targetFwd = MathHelper::GetForwardVector(targetEntity->Rotation);
		targetFwd.y = 0.0f;
		targetFwd = MathHelper::Normalized(targetFwd);
		vec3 targetRight = MathHelper::Normalized(glm::cross(targetFwd, vec3(0, 1, 0)));

		// Gather nearby ally positions for clustering penalty
		std::vector<vec3> allyPositions;
		for (auto ob : AiPerceptionSystem::GetObserversInRadius(Position, 12.0f))
			allyPositions.push_back(ob->position - vec3(0, 1.3f, 0));

		// Generate flanking candidates
		std::vector<vec3> candidates;
		const int wantCandidates = 10;
		const int maxIter = 36;

		for (int i = 0; i < maxIter && (int)candidates.size() < wantCandidates; ++i)
		{
			float r = RandomHelper::RandomFloat();
			float angleDeg;
			if (r < 0.40f) angleDeg = -82.0f + (RandomHelper::RandomFloat() - 0.5f) * 68.0f;
			else if (r < 0.80f) angleDeg = 82.0f + (RandomHelper::RandomFloat() - 0.5f) * 68.0f;
			else if (r < 0.90f) angleDeg = 180.0f + (RandomHelper::RandomFloat() - 0.5f) * 48.0f;
			else                angleDeg = 0.0f + (RandomHelper::RandomFloat() - 0.5f) * 24.0f;

			float rad = MathHelper::ToRadians(angleDeg);
			vec3  dir = MathHelper::Normalized(std::cos(rad) * targetFwd
				+ std::sin(rad) * targetRight);

			float dist = meleeRadius * (0.85f + RandomHelper::RandomFloat() * 0.30f);
			vec3  candidate = targetPos + dir * dist;

			// Skip positions the NPC can't quickly reach
			if (glm::distance(candidate, Position) > 5.0f) continue;

			auto hit = Physics::CylinderTrace(Position, candidate, 0.5f, 0.8f,
				BodyType::World | BodyType::MainBody);
			if (hit.hasHit) continue;

			candidates.push_back(candidate);
		}

		if (candidates.empty())
		{
			// Fallback: step directly toward the target
			vec3 toTarget = MathHelper::Normalized(MathHelper::XZ(targetPos - Position));
			return Position + toTarget * 0.8f;
		}

		// ── Score candidates ──────────────────────────────────────────────────
		vec3  bestPos = candidates[0];
		float bestScore = -std::numeric_limits<float>::max();

		vec3 currRadial = MathHelper::Normalized(MathHelper::XZ(Position - targetPos));

		for (const vec3& c : candidates)
		{
			vec3 toTargetDir = MathHelper::Normalized(MathHelper::XZ(c - targetPos));

			// Flanking score: higher when perpendicular to target's forward axis
			float flankScore = -std::abs(glm::dot(toTargetDir, targetFwd));

			// Circling score: prefer moving tangentially rather than in/out
			vec3  moveDir = MathHelper::Normalized(MathHelper::XZ(c - Position));
			float circleScore = 1.0f - std::abs(glm::dot(moveDir, currRadial));

			// Clustering penalty: discourage piling into the same spot as allies
			float clusterPenalty = 0.0f;
			for (const vec3& ally : allyPositions)
			{
				float d = glm::distance(c, ally);
				if (d > 0.001f)
					clusterPenalty += std::exp(-(d * d) / (2.0f * 1.4f * 1.4f));
			}

			float score = flankScore * 2.5f
				+ circleScore * 1.6f
				+ RandomHelper::RandomFloat() * 0.28f  // unpredictability jitter
				- clusterPenalty * 2.0f;

			if (score > bestScore)
			{
				bestScore = score;
				bestPos = c;
			}
		}

		return bestPos;
	}

private:

};

REGISTER_ENTITY(NpcGuardMelee, "npc_guard_melee")