#pragma once

#include "NpcAnimatorBase.h"

// Melee animator: swaps the upper-body weapon layer for a sword/block layer.
// Action animations (stun, special moves) are handled by the sealed
// ProcessResultPose in NpcAnimatorBase — no duplication needed here.
class NpcMeleeAnimator : public NpcAnimatorBase
{
public:

	NpcMeleeAnimator(Entity* ownerEntity) : NpcAnimatorBase(ownerEntity) {}

	void Update() override;

	bool blocking = false;

protected:

	void LoadAssets() override;
	AnimationPose BuildCombatPose() override;

	Animation* blockAnimation = nullptr;
};
