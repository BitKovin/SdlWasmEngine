#pragma once

#include "NpcAnimatorBase.h"

class NpcMeleeAnimator : public NpcAnimatorBase
{
public:

	NpcMeleeAnimator(Entity* ownerEntity) : NpcAnimatorBase(ownerEntity) {}

	void Update();

	Animation* actionAnimation = nullptr;

	bool blocking = false;

	bool IsPlayingActionAnimation();

	void PlayActionAnimation(std::string animationName, bool loop, float blendIn = 0.3f, float blendOut = 0.3f);

	void StopActionAnimation();

	float actionBlendOut = 0.3f;

protected:

	void LoadAssets();

	AnimationPose ProcessResultPose();

	Animation* blockAnimation = nullptr;

};
