#pragma once

#include "NpcAnimatorBase.h"

class NpcMeleeAnimator : public NpcAnimatorBase
{
public:

	NpcMeleeAnimator(Entity* ownerEntity) : NpcAnimatorBase(ownerEntity) {}

	void Update();

	Animation* actionAnimation = nullptr;

	bool IsPlayingActionAnimation();

	void PlayActionAnimation(std::string animationName, bool loop);

	void PlayActionAnimation();

protected:

	void LoadAssets();

	AnimationPose ProcessResultPose();



};
