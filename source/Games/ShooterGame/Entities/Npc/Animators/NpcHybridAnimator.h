#pragma once

#include "NpcMeleeAnimator.h"

enum class CombatMode { Melee, Ranged };

// Hybrid animator: holds both a melee block layer (from NpcMeleeAnimator) and
// a ranged pistol layer (from NpcAnimatorBase).  The NPC drives targetCombatMode
// and combatModeBlend smoothly tracks it, producing a continuous cross-fade
// rather than a hard cut when the NPC transitions between weapon modes.
class NpcHybridAnimator : public NpcMeleeAnimator
{
public:

	NpcHybridAnimator(Entity* ownerEntity) : NpcMeleeAnimator(ownerEntity) {}

	// Written by the NPC every frame; read by the animator to blend.
	CombatMode targetCombatMode   = CombatMode::Ranged;

	// 0.0 = full melee, 1.0 = full ranged.  Tracks targetCombatMode over time.
	float combatModeBlend      = 1.0f;
	float combatModeBlendSpeed = 3.0f; // blend units per second

	void Update() override;

protected:

	void LoadAssets() override;
	AnimationPose BuildCombatPose() override;
};
