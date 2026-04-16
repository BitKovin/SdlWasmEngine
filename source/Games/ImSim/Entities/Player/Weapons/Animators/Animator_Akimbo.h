#pragma once

#include "WeaponAnimator.h"

class Animator_Akimbo : public WeaponAnimator
{
public:

	Animator_Akimbo(Entity* ownerEntity) : WeaponAnimator(ownerEntity) {}

protected:

	void LoadAssets() override;

	AnimationPose ProcessResultPose() override;

	AnimationPose CalculateWeaponPose();
	AnimationPose CalculateIdlePose();
	AnimationPose CalculateAimPose();

};