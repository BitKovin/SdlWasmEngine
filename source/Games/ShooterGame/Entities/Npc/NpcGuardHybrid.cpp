#include "NpcGuardHybrid.h"

// ── Constructor / lifecycle ──────────────────────────────────────────────────

NpcGuardHybrid::NpcGuardHybrid()
{
	// NpcGuardMelee's constructor runs first, setting up melee defaults.
	// Override the animator and the initial attack ranges for ranged mode.
	animator = std::make_unique<NpcHybridAnimator>(this);

	attackRange        = rangedAttackRange;
	attackDesiredRange = rangedDesiredRange;
}

void NpcGuardHybrid::Start()
{
	NpcGuardMelee::Start();

	// ── Melee weapon mesh ─────────────────────────────────────────────────────
	// Create and attach the melee weapon to the dominant hand bone.
	// Configure the model path to match your project's asset layout.
	//
	//   meleeWeaponMesh = SpawnChildEntity<StaticMesh>();
	//   meleeWeaponMesh->SetModel("GameData/models/weapons/sword.glb");
	//   meleeWeaponMesh->AttachToBone(mesh, "hand_r");
	//
	// The ranged weapon (weaponMesh) is created by NpcBase::Start() or
	// LoadAssets() — no extra work needed here for that one.

	// Start in the correct visual state for the initial mode
	SwitchToMode(currentMode);
}

// ── Disarm interface ──────────────────────────────────────────────────────────

void NpcGuardHybrid::DisableRangedWeapon()
{
	rangedDisabled = true;
	SwitchToMode(CombatMode::Melee);   // SwitchToMode is a no-op if already melee
	UpdateWeaponMeshVisibility();
}

void NpcGuardHybrid::EnableRangedWeapon()
{
	rangedDisabled = false;
	UpdateWeaponMeshVisibility();
	// UpdateCombatMode() will pick up the correct mode on the next frame
}

// ── Mode management ───────────────────────────────────────────────────────────

void NpcGuardHybrid::UpdateCombatMode()
{
	// Disarmed: always stay in melee
	if (rangedDisabled)
	{
		SwitchToMode(CombatMode::Melee);
		return;
	}

	if (target_id.empty() || !target_follow) return;

	Entity* targetRef = Level::Current->FindEntityWithId(target_id);
	if (!targetRef) return;

	const float dist = glm::distance(Position, targetRef->Position);

	// Hysteresis: only switch at the appropriate threshold for the active mode
	if (currentMode == CombatMode::Ranged && dist < meleeThreshold)
		SwitchToMode(CombatMode::Melee);
	else if (currentMode == CombatMode::Melee && dist > rangedThreshold)
		SwitchToMode(CombatMode::Ranged);
}

void NpcGuardHybrid::SwitchToMode(CombatMode mode)
{
	if (currentMode == mode) return;
	currentMode = mode;

	if (mode == CombatMode::Ranged)
	{
		// Reset the melee state machine cleanly so it is fresh next time
		if (isCurrentlyBlocking) { StopBlock(); isCurrentlyBlocking = false; }
		pendingCounterAttack  = false;
		nextAttackIsBreak     = false;
		playerWasBlockingPrev = false;
		EnterState(MeleeState::IDLE);

		attackRange        = rangedAttackRange;
		attackDesiredRange = rangedDesiredRange;
	}
	else // Melee
	{
		// Begin approaching immediately; state machine handles the rest
		EnterState(MeleeState::APPROACHING);

		attackRange        = 8.0f;
		attackDesiredRange = 2.0f;
	}

	// Sync animator blend target
	GetHybridAnimator()->targetCombatMode = mode;

	UpdateWeaponMeshVisibility();
}

void NpcGuardHybrid::UpdateWeaponMeshVisibility()
{
	// Show the ranged weapon only in ranged mode and when not disarmed
	if (weaponMesh)
		weaponMesh->Visible = (currentMode == CombatMode::Ranged && !rangedDisabled);

	// Show the melee weapon only in melee mode
	if (meleeWeaponMesh)
		meleeWeaponMesh->Visible = (currentMode == CombatMode::Melee);
}

// ── Entity overrides ──────────────────────────────────────────────────────────

void NpcGuardHybrid::UpdateTargetAttack()
{
	// Evaluate distance and flip mode before running the active combat logic
	UpdateCombatMode();

	if (currentMode == CombatMode::Melee)
	{
		// Full melee state machine (blocking, parry, feint, etc.)
		NpcGuardMelee::UpdateTargetAttack();
	}
	else
	{
		// Standard ranged attack logic from NpcBase (strafing, projectiles)
		NpcBase::UpdateTargetAttack();
	}
}

vec3 NpcGuardHybrid::FindAttackLocation()
{
	// Delegate to the appropriate parent depending on active mode
	return (currentMode == CombatMode::Melee)
		? NpcGuardMelee::FindAttackLocation()
		: NpcBase::FindAttackLocation();
}

void NpcGuardHybrid::UpdateAnimations(bool forceFullUpdate)
{
	// All melee animator state (blocking, action root motion, events) is
	// handled by the parent.  NpcHybridAnimator inherits from NpcMeleeAnimator
	// so the static_cast<NpcMeleeAnimator*> inside NpcGuardMelee::UpdateAnimations
	// remains correct here — no override of the cast is needed.
	NpcGuardMelee::UpdateAnimations(forceFullUpdate);

	// Push the current mode so the animator can smoothly blend between
	// the two weapon poses.
	GetHybridAnimator()->targetCombatMode = currentMode;
}

REGISTER_ENTITY(NpcGuardHybrid, "npc_guard_hybrid")
