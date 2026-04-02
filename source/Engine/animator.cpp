#include "Animator.h"

Animator::Animator()
{
	owner = nullptr;
}

Animator::Animator(Entity* ownerEntity)
{
	owner = ownerEntity;
}

Animator::~Animator()
{

	for (Animation* anim : animationsToUpdate)
	{
		delete(anim);
	}

	animationsToUpdate.clear();

}

void Animator::Update()
{

	if (loaded == false) return;

	for (Animation* anim : animationsToUpdate)
	{

		anim->UpdatePose = UpdatePose;
		anim->UsePrecomputedFrames = UsePrecomputedFrames;

		anim->Update(Speed);

		auto events = anim->PullAnimationEvents();

		pendingAnimationEvents.insert(pendingAnimationEvents.end(), events.begin(), events.end());

	}

}

AnimationPose Animator::GetResultPose()
{
	if (loaded == false) return AnimationPose();

	if (UpdatePose == false)
	{
		return lastPose;
	}

	lastPose = ProcessResultPose();

	return lastPose;
}

void Animator::LoadAssetsIfNeeded()
{
	if (loaded) return;

	LoadAssets();
	loaded = true;
}

vector<AnimationEvent> Animator::PullAnimationEvents()
{

	auto copy = pendingAnimationEvents;
	pendingAnimationEvents.clear();

	return copy;
}

Animation* Animator::AddAnimation(string path, std::string name, bool loop)
{

	Animation* animation = new Animation(owner);
	animation->LoadFromFile(path);
	
	if (animation->model == nullptr) return animation;

	if (name == "")
	{

		if (animation->model->animations.size() > 0)
		{
			name = animation->model->animations.begin()->first.str();
		}

		
	}

	animation->PlayAnimation(name, loop, 0);

	animationsToUpdate.push_back(animation);
	return animation;

}

AnimationPose Animator::ProcessResultPose()
{
	return AnimationPose();

}
