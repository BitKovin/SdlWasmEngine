#pragma once

#include <Animator.h>

class NpcAnimatorBase : public Animator
{
public:

	NpcAnimatorBase(Entity* ownerEntity) : Animator(ownerEntity) {}

	float movementSpeed = 0;

	float PainProgress = 0;

	bool weapon_holds = false;
	bool weapon_ready = false;
	bool weapon_aims = false;

	bool weapon_pendingAttack = false;

	bool scared = false;

	vec3 spineRotation = vec3();

	void Update();

	void PlayTaskAnimation(std::string animationName, bool loop);

	void StopTaskAnimation();

	Animation* taskAnimation = nullptr;

protected:

	Animation* locomotion = nullptr;
	Animation* pistol = nullptr;

	Animation* inPain = nullptr;

	void LoadAssets();

	AnimationPose ProcessResultPose();

};
