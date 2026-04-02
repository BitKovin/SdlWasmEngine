#pragma once
#include "Animation.h"

class Animator
{
public:

	float Speed = 1;
	bool UpdatePose = true; 

	bool UsePrecomputedFrames = false;

	Animator();
	Animator(Entity* ownerEntity);
	virtual ~Animator();

	virtual void Update();
	AnimationPose GetResultPose();

	void LoadAssetsIfNeeded();

	vector<AnimationEvent> PullAnimationEvents();

protected:

	AnimationPose lastPose;

	vector<Animation*> animationsToUpdate;

	vector<AnimationEvent> pendingAnimationEvents;

	Entity* owner = nullptr;

	bool loaded = false;

	Animation* AddAnimation(string path, std::string name = "", bool loop = true);

	virtual AnimationPose ProcessResultPose();

	virtual void LoadAssets(){}

};

