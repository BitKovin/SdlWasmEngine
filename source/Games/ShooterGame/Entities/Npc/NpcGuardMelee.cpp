#include "NpcGuardMelee.h"

#include <Entities/Player/Weapons/Projectiles/Bullet.h>

// ── Constructor ─────────────────────────────────────────────────────────────

NpcGuardMelee::NpcGuardMelee()
{
	npcType = NpcType::Guard;
	modelPath = "GameData/models/npc/guard.glb";
	hostileTags.insert("bandit");

	attackRange = 8.0f;
	attackDesiredRange = 2.0f;

	animator = std::make_unique<NpcMeleeAnimator>(this);
}

// ── Public overrides ─────────────────────────────────────────────────────────

bool NpcGuardMelee::isStunned()
{
	return NpcBase::isStunned() || stunDelay.Wait();
}

bool NpcGuardMelee::IsPlayingRootMotionAnimation()
{
	return GetMeleeAnimator()->actionAnimation->IsAnimationPlaying();
}

bool NpcGuardMelee::IsRotationAllowedDuringRootMotion()
{
	return attackDelay.Wait();
}

// ── State machine ─────────────────────────────────────────────────────────

void NpcGuardMelee::CommitAttack()
{
	if (!CanAttackOrBlock()) return;

	if (isCurrentlyBlocking) return;

	// Priority 1: punish passive blocking
	if (nextAttackIsBreak || (RandomHelper::RandomFloat() < (timesParriedThisFight * 0.2f)))
	{
		timesParriedThisFight -= 4;
		timesParriedThisFight = std::max(timesParriedThisFight, 0);
		nextAttackIsBreak = false;
		if (RandomHelper::RandomFloat() < 0.3f)
			PerformBaitingMeleeAttack();
		else
			PerformBreakingAttack();
		return;
	}

	// Priority 2: escalate on repeated active blocks
	if (!blockCountDecayTimer.Wait())
	{
		playerBlockCount = std::max(0, playerBlockCount - 1);
		blockCountDecayTimer.AddDelay(4.0f);
	}

	const bool targetIsBlocking = IsTargetBlocking();

	if (playerBlockCount >= 3)
	{
		PerformBreakingAttack();
		playerBlockCount = 0;
	}
	else if (playerBlockCount == 2 && targetIsBlocking)
	{
		PerformBaitingMeleeAttack();
	}
	else
	{
		PerformMeleeAttack();
	}

	if (targetIsBlocking) playerBlockCount++;
}

void NpcGuardMelee::UpdateMeleeStateMachine()
{
	Entity* targetRef = Level::Current->FindEntityWithId(target_id);
	if (!targetRef) return;

	const bool inRange = InMeleeAttackRange();
	const bool blocking = IsTargetBlocking();

	// ── Passive-block detection ───────────────────────────────────────────────
	{
		if (blocking && !playerWasBlockingPrev)
		{
			float patience = 0.75f + RandomHelper::RandomFloat() * 0.55f;
			patience -= std::min(timesParriedThisFight * 0.08f, 0.35f);
			patience = std::max(patience, 0.28f);
			continuousBlockTimer.AddDelay(patience);
		}

		if (!blocking) nextAttackIsBreak = false;

		if (blocking && !continuousBlockTimer.Wait() &&
			(meleeState == MeleeState::PRESSURE || meleeState == MeleeState::APPROACHING))
		{
			nextAttackIsBreak = true;
			continuousBlockTimer.AddDelay(1.8f + RandomHelper::RandomFloat() * 0.8f);
			EnterState(MeleeState::WIND_UP, 0.14f + RandomHelper::RandomFloat() * 0.20f);
		}

		playerWasBlockingPrev = blocking;
	}

	switch (meleeState)
	{
	case MeleeState::IDLE:
		if (target_attackInRange) EnterState(MeleeState::APPROACHING);
		break;

	case MeleeState::APPROACHING:
		if (!target_attackInRange) { EnterState(MeleeState::IDLE); break; }
		if (!attackPositionUpdateDelay.Wait())
		{
			attackPosition = FindAttackLocation();
			attackPositionUpdateDelay.AddDelay(0.28f);
		}
		if (inRange) EnterState(MeleeState::PRESSURE, RandomPressureDuration());
		break;

	case MeleeState::PRESSURE:
		if (!target_attackInRange) { EnterState(MeleeState::APPROACHING); break; }
		if (!attackPositionUpdateDelay.Wait())
		{
			attackPosition = FindAttackLocation();
			attackPositionUpdateDelay.AddDelay(0.30f + RandomHelper::RandomFloat() * 0.25f);
		}
		if (pendingCounterAttack) { pendingCounterAttack = false; EnterState(MeleeState::PUNISHING, 0.12f); break; }
		if (!stateTimer.Wait())
		{
			if (RandomHelper::RandomFloat() < 0.25f)
				EnterState(MeleeState::BAITING, 0.38f + RandomHelper::RandomFloat() * 0.55f);
			else
				EnterState(MeleeState::WIND_UP, 0.18f + RandomHelper::RandomFloat() * 0.32f);
			break;
		}
		if (inRange && !blocking && RandomHelper::RandomFloat() < 0.011f)
			EnterState(MeleeState::WIND_UP, 0.12f + RandomHelper::RandomFloat() * 0.18f);
		break;

	case MeleeState::WIND_UP:
		attackPosition = Position;
		if (!stateTimer.Wait())
		{
			if (inRange) CommitAttack();
			EnterState(MeleeState::RECOVERING, 0.55f + RandomHelper::RandomFloat() * 0.75f);
		}
		break;

	case MeleeState::RECOVERING:
		attackPosition = Position;
		if (!stateTimer.Wait())
		{
			float retreatChance = std::min(
				0.38f + playerBlockCount * 0.12f + timesParriedThisFight * 0.08f, 0.78f);
			if (RandomHelper::RandomFloat() < retreatChance)
				EnterState(MeleeState::RETREATING, 0.70f + RandomHelper::RandomFloat() * 0.55f);
			else
				EnterState(MeleeState::PRESSURE, RandomPressureDuration());
		}
		break;

	case MeleeState::RETREATING:
		if (!retreatPosTimer.Wait()) { attackPosition = FindAttackLocation(); retreatPosTimer.AddDelay(0.35f); }
		if (!stateTimer.Wait())
		{
			if (target_attackInRange) EnterState(MeleeState::PRESSURE, RandomPressureDuration());
			else                      EnterState(MeleeState::APPROACHING);
		}
		break;

	case MeleeState::BLOCKING:
		attackPosition = Position;
		if (!stateTimer.Wait())
		{
			StopBlock();
			isCurrentlyBlocking = false;
			if (pendingCounterAttack) { pendingCounterAttack = false; EnterState(MeleeState::PUNISHING, 0.10f); }
			else                      EnterState(MeleeState::PRESSURE, RandomPressureDuration() * 0.55f);
		}
		break;

	case MeleeState::PUNISHING:
		attackPosition = Position;
		if (!stateTimer.Wait())
		{
			if (inRange && CanAttackOrBlock()) PerformMeleeAttack();
			EnterState(MeleeState::RECOVERING, 0.42f + RandomHelper::RandomFloat() * 0.32f);
		}
		break;

	case MeleeState::BAITING:
		attackPosition = Position;
		if (pendingCounterAttack) { pendingCounterAttack = false; EnterState(MeleeState::PUNISHING, 0.07f); break; }
		if (!stateTimer.Wait())
			EnterState(MeleeState::WIND_UP, 0.10f + RandomHelper::RandomFloat() * 0.14f);
		break;
	}
}

// ── Actions ──────────────────────────────────────────────────────────────────

void NpcGuardMelee::Stun()
{
	PlayActionAnimation("stun", false, 0.2f, 1.2f);
	const float dur = GetActionAnimationRemainingTime();
	attackDelay.AddDelay(dur);
	stunDelay.AddDelay(dur);
}

void NpcGuardMelee::PerformMeleeAttack()
{
	Logger::Log("PerformMeleeAttack");
	PlayActionAnimation("sword_attack_main", false);
	attackDelay.AddDelay(GetActionAnimationRemainingTime());
}

void NpcGuardMelee::PerformBaitingMeleeAttack()
{
	Logger::Log("PerformBaitingMeleeAttack");
	PlayActionAnimation("sword_attack_break", false);
	attackDelay.AddDelay(GetActionAnimationRemainingTime());
}

void NpcGuardMelee::PerformBreakingAttack()
{
	Logger::Log("PerformBreakingAttack");
	PlayActionAnimation("sword_kick", false);
	attackDelay.AddDelay(GetActionAnimationRemainingTime());
}

void NpcGuardMelee::StartBlock() 
{ 

	isCurrentlyBlocking = true;
	pendingCounterAttack = true;
	EnterState(MeleeState::BLOCKING, 0.50f + RandomHelper::RandomFloat() * 0.18f);

}
void NpcGuardMelee::StopBlock() { Logger::Log("StopBlock"); }

void NpcGuardMelee::OnParried()
{
	timesParriedThisFight++;
	pendingCounterAttack = false;
	nextAttackIsBreak = false;

	float stagger = std::min(0.85f + RandomHelper::RandomFloat() * 0.45f
		+ timesParriedThisFight * 0.10f, 1.90f);
	EnterState(MeleeState::RECOVERING, stagger);
	attackPosition = Position;
}

// ── Damage application ────────────────────────────────────────────────────────

void NpcGuardMelee::DoNormalAttackDamage()
{
	vec3 start = Position + vec3(0, 0.5f, 0);
	auto hit = Physics::SphereTrace(start, start + MathHelper::GetForwardVector(mesh->Rotation),
		0.3f, BodyType::GroupHitTest, {}, { this });

	if (hit.hasHit && hit.entity->HasTag("player"))
	{
		if (NpcHelper::CheckParry(MathHelper::GetForwardVector(mesh->Rotation), hit.entity))
		{
			OnParried();
			Stun();
		}
		else if(NpcHelper::CheckBlock(MathHelper::GetForwardVector(mesh->Rotation), hit.entity))
		{
			playerBlockCount++;
		}
		else
		{
			hit.entity->OnPointDamage(15, hit.shapePosition,
				MathHelper::FastNormalize(hit.shapePosition - Position),
				hit.hitboxName, this, this);
		}
	}
}

void NpcGuardMelee::DoBreakAttackDamage()
{
	vec3 start = Position + vec3(0, 0.5f, 0);
	auto hit = Physics::SphereTrace(start, start + MathHelper::GetForwardVector(mesh->Rotation),
		0.3f, BodyType::GroupHitTest, {}, { this });

	if (hit.hasHit && hit.entity->HasTag("player"))
	{
		hit.entity->OnPointDamage(10, hit.shapePosition,
			MathHelper::FastNormalize(hit.shapePosition - Position),
			hit.hitboxName, this, this);
	}
}

// ── Animation ─────────────────────────────────────────────────────────────────

void NpcGuardMelee::ProcessAnimationEvent(const AnimationEvent& event)
{
	if (event.eventName == "attack_sword_normal") DoNormalAttackDamage();
	else if (event.eventName == "attack_sword_break")  DoBreakAttackDamage();
	else if (event.eventName == "attack_kick")         DoBreakAttackDamage();
}

void NpcGuardMelee::UpdateAnimations(bool forceFullUpdate)
{
	NpcMeleeAnimator* meleeAnimator = GetMeleeAnimator();

	// actionAnimation is now in NpcAnimatorBase, accessible via the melee cast
	meleeAnimator->actionAnimation->Rotation = mesh->Rotation;
	meleeAnimator->blocking = isCurrentlyBlocking;

	NpcBase::UpdateAnimations(forceFullUpdate);

	if (!controller) return;

	// Apply root motion from the action animation (attacks, stun)
	vec3 currPos = controller->GetPosition();
	auto rootMotion = meleeAnimator->actionAnimation->PullRootMotion();
	controller->SetPosition(currPos + rootMotion.Position);

	auto events = meleeAnimator->PullAnimationEvents();
	for (auto& event : events) ProcessAnimationEvent(event);
}

// ── WarnAboutAttack ───────────────────────────────────────────────────────────

void NpcGuardMelee::WarnAboutAttack(Entity* from)
{
	if (from->Id != target_id) return;

	vec3 forward = MathHelper::GetForwardVector(mesh->Rotation);
	vec3 attackerForward = MathHelper::GetForwardVector(from->Rotation);

	DebugDraw::Line(Position + vec3(0, 0.0f, 0), Position + vec3(0, 0.0f, 0) + forward * 2.0f, 1.1f, 0.1f, DebugColor::Red);
	DebugDraw::Line(from->Position + vec3(0, 0.0f, 0), from->Position + vec3(0, 0.0f, 0) + attackerForward * 2.0f, 1.1f, 0.1f, DebugColor::Green);

	float dotOF = glm::dot(forward, attackerForward);

	if (glm::dot(forward, attackerForward) > -0.6f) return;

	if (meleeState == MeleeState::BLOCKING)
	{
		stateTimer.AddDelay(0.5f);
	}


	float blockChance = 1.0f;
	switch (meleeState)
	{
	case MeleeState::BAITING:    blockChance = 0.92f; break;
	case MeleeState::PRESSURE:   blockChance = 0.55f; break;
	case MeleeState::IDLE:
	case MeleeState::APPROACHING:blockChance = 0.40f; break;
	case MeleeState::RECOVERING: blockChance = 0.18f; break;
	case MeleeState::WIND_UP:    blockChance = 0.08f; break;
	default:                     blockChance = 0.30f; break;
	}

	blockChance += std::min(timesParriedThisFight * 0.06f, 0.22f);
	if (RandomHelper::RandomFloat() > blockChance) return;

	StartBlock();

}

// ── UpdateTargetAttack ────────────────────────────────────────────────────────

void NpcGuardMelee::UpdateTargetAttack()
{
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

	if (!target_follow || !target_sees || !target_attack || isStunned() || DoingTask)
	{
		if (isCurrentlyBlocking) { StopBlock(); isCurrentlyBlocking = false; }
		pendingCounterAttack = false;
		nextAttackIsBreak = false;
		playerWasBlockingPrev = false;
		EnterState(MeleeState::IDLE);
		return;
	}

	if (attackDelay.Wait()) return;

	UpdateMeleeStateMachine();
}

// ── FindAttackLocation ────────────────────────────────────────────────────────

vec3 NpcGuardMelee::FindAttackLocation()
{
	Entity* targetEntity = Level::Current->FindEntityWithId(target_id);
	if (!targetEntity) return Position;

	const vec3& targetPos = targetEntity->Position;

	// ── Retreating ────────────────────────────────────────────────────────────
	if (meleeState == MeleeState::RETREATING)
	{
		vec3  awayDir = MathHelper::Normalized(MathHelper::XZ(Position - targetPos));
		float currentDist = glm::length(MathHelper::XZ(Position - targetPos));
		float retreatDist = std::min(currentDist + 1.8f + RandomHelper::RandomFloat() * 1.4f,
			attackRange * 0.80f);
		vec3  candidate = targetPos + awayDir * retreatDist;

		auto hit = Physics::CylinderTrace(Position, candidate, 0.5f, 0.8f,
			BodyType::World | BodyType::MainBody);
		if (!hit.hasHit) return candidate;

		vec3 perpRight = MathHelper::Normalized(glm::cross(awayDir, vec3(0, 1, 0)));
		for (float side : { 1.0f, -1.0f })
		{
			vec3 c = Position + perpRight * side * 2.4f;
			if (!Physics::CylinderTrace(Position, c, 0.5f, 0.8f,
				BodyType::World | BodyType::MainBody).hasHit) return c;
		}
		return Position;
	}

	// ── IDLE: back away slightly ──────────────────────────────────────────────
	if (meleeState == MeleeState::IDLE)
	{
		vec3 awayDir = MathHelper::Normalized(MathHelper::XZ(Position - targetPos));
		vec3 candidate = Position + awayDir * 4.0f;
		auto hit = Physics::CylinderTrace(Position, candidate, 0.5f, 0.8f,
			BodyType::World | BodyType::MainBody);
		if (!hit.hasHit) return candidate;

		vec3 perpRight = MathHelper::Normalized(glm::cross(awayDir, vec3(0, 1, 0)));
		for (float side : { 1.0f, -1.0f })
		{
			vec3 c = Position + perpRight * side * 2.4f;
			if (!Physics::CylinderTrace(Position, c, 0.5f, 0.8f,
				BodyType::World | BodyType::MainBody).hasHit) return c;
		}
		return Position;
	}

	// ── Frozen states: hold position ──────────────────────────────────────────
	if (meleeState == MeleeState::WIND_UP || meleeState == MeleeState::RECOVERING ||
		meleeState == MeleeState::BLOCKING || meleeState == MeleeState::PUNISHING ||
		meleeState == MeleeState::BAITING)
		return Position;

	// ── PRESSURE / APPROACHING: flanking positions ────────────────────────────
	const float meleeRadius = 1.70f;
	vec3 targetFwd = MathHelper::Normalized(MathHelper::GetForwardVector(targetEntity->Rotation));
	targetFwd.y = 0.0f;
	vec3 targetRight = MathHelper::Normalized(glm::cross(targetFwd, vec3(0, 1, 0)));

	std::vector<vec3> allyPositions;
	for (auto ob : AiPerceptionSystem::GetObserversInRadius(Position, 12.0f))
		allyPositions.push_back(ob->position - vec3(0, 1.3f, 0));

	std::vector<vec3> candidates;
	for (int i = 0; i < 36 && (int)candidates.size() < 10; ++i)
	{
		float r = RandomHelper::RandomFloat();
		float angleDeg;
		if (r < 0.40f) angleDeg = -82.0f + (RandomHelper::RandomFloat() - 0.5f) * 68.0f;
		else if (r < 0.80f) angleDeg = 82.0f + (RandomHelper::RandomFloat() - 0.5f) * 68.0f;
		else if (r < 0.90f) angleDeg = 180.0f + (RandomHelper::RandomFloat() - 0.5f) * 48.0f;
		else                angleDeg = 0.0f + (RandomHelper::RandomFloat() - 0.5f) * 24.0f;

		float rad = MathHelper::ToRadians(angleDeg);
		vec3  dir = MathHelper::Normalized(std::cos(rad) * targetFwd + std::sin(rad) * targetRight);
		float dist = meleeRadius * (0.85f + RandomHelper::RandomFloat() * 0.30f);
		vec3  c = targetPos + dir * dist;

		if (glm::distance(c, Position) > 5.0f) continue;
		if (Physics::CylinderTrace(Position, c, 0.5f, 0.8f,
			BodyType::World | BodyType::MainBody).hasHit) continue;
		candidates.push_back(c);
	}

	if (candidates.empty())
		return Position + MathHelper::Normalized(MathHelper::XZ(targetPos - Position)) * 0.8f;

	vec3  bestPos = candidates[0];
	float bestScore = -std::numeric_limits<float>::max();
	vec3  currRadial = MathHelper::Normalized(MathHelper::XZ(Position - targetPos));

	for (const vec3& c : candidates)
	{
		vec3  toTargetDir = MathHelper::Normalized(MathHelper::XZ(c - targetPos));
		vec3  moveDir = MathHelper::Normalized(MathHelper::XZ(c - Position));
		float flankScore = -std::abs(glm::dot(toTargetDir, targetFwd));
		float circleScore = 1.0f - std::abs(glm::dot(moveDir, currRadial));
		float clusterPen = 0.0f;

		for (const vec3& ally : allyPositions)
		{
			float d = glm::distance(c, ally);
			if (d > 0.001f) clusterPen += std::exp(-(d * d) / (2.0f * 1.4f * 1.4f));
		}

		float score = flankScore * 2.5f + circleScore * 1.6f
			+ RandomHelper::RandomFloat() * 0.28f - clusterPen * 2.0f;

		if (score > bestScore) { bestScore = score; bestPos = c; }
	}

	return bestPos;
}

// ── Helpers ───────────────────────────────────────────────────────────────────

bool NpcGuardMelee::IsTargetBlocking() const
{
	Entity* t = Level::Current->FindEntityWithId(target_id);
	if (!t) return false;

	if (t == Player::Instance)
	{
		if (Player::Instance->currentWeapon && Player::Instance->currentWeapon->Blocking)      return true;
		if (Player::Instance->currentOffhandWeapon && Player::Instance->currentOffhandWeapon->Blocking) return true;
	}

	return false;
}

REGISTER_ENTITY(NpcGuardMelee, "npc_guard_melee")