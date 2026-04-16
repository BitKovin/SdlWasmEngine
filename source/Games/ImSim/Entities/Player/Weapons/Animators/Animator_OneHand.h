#pragma once

#include "WeaponAnimator.h"

class Animator_OneHand : public WeaponAnimator
{
public:

	Animator_OneHand(Entity* ownerEntity) : WeaponAnimator(ownerEntity) {}

protected:

	void LoadAssets() override;

	AnimationPose ProcessResultPose() override;

	AnimationPose CalculateWeaponPose();
	AnimationPose CalculateIdlePose();
	AnimationPose CalculateAimPose();

};