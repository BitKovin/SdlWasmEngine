#pragma once

#include "WeaponAnimator.h"

class Animator_Rifle : public WeaponAnimator
{
public:

	Animator_Rifle(Entity* ownerEntity) : WeaponAnimator(ownerEntity) {}

protected:

	void LoadAssets() override;

	AnimationPose ProcessResultPose() override;

	AnimationPose CalculateWeaponPose();
	AnimationPose CalculateIdlePose();
	AnimationPose CalculateAimPose();

};