#pragma once

#include <Animator.h>

class WeaponAnimator : public Animator
{
public:
	
	WeaponAnimator(Entity* ownerEntity) : Animator(ownerEntity) {}
	WeaponAnimator(): Animator(){}

	AnimationPose inPose;

	float weaponAim = 0;

	Animation* idleAnimation;
	Animation* aimAnimation;

	virtual void Attack() {}


private:

};

