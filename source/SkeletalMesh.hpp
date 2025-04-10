#pragma once


#include <vector>

#include "ShaderManager.h"

#include "IDrawMesh.h"

#include "VertexData.h"

#include "MathHelper.hpp"

#include "skinned_model.hpp"

#include "glm.h"

#include "animator.hpp"

#include "StaticMesh.hpp"

#include "Time.hpp"

using namespace std;

struct AnimationPose 
{
	std::unordered_map<std::string, mat4> boneTransforms;

	static AnimationPose Lerp(AnimationPose a, AnimationPose b, float progress)
	{
		if (progress < 0.002)
			return a;

		if (progress > 0.995)
			return b;

		std::unordered_map<std::string, mat4> resultPose;

		for (auto bonePose : a.boneTransforms)
		{
			mat4 aMat = a.boneTransforms[bonePose.first];
			mat4 bMat = b.boneTransforms[bonePose.first];

			auto aTrans = MathHelper::DecomposeMatrix(aMat);
			auto bTrans = MathHelper::DecomposeMatrix(bMat);

			auto resultTrans = MathHelper::Transform::Lerp(aTrans, bTrans, progress);

			resultPose[bonePose.first] = resultTrans.ToMatrix();

		}

		AnimationPose result;

		result.boneTransforms = resultPose;

		return result;

	}

};

class SkeletalMesh : public StaticMesh
{
private:

	roj::Animator animator;

	std::vector<mat4> boneTransforms;

	std::vector<mat4> finalizedBoneTransforms;

	double blendStartTime = 0;
	double blendEndTime = 0;

	bool firstAnimation = true;

	float GetBlendInProgress()
	{
		double currentTime = Time::GameTime;

		if (blendEndTime <= blendStartTime)
			return 1.0f; // Avoid division by zero or invalid range

		if (currentTime <= blendStartTime)
			return 0.0f;

		if (currentTime >= blendEndTime)
			return 1.0f;

		double progress = (currentTime - blendStartTime) / (blendEndTime - blendStartTime);
		return static_cast<float>(progress);
	}
	AnimationPose blendStartPose;

protected:

	void ApplyAdditionalShaderParams(ShaderProgram* shader_program)
	{
		for (int i = 0; i < finalizedBoneTransforms.size(); ++i)
			shader_program->SetUniform("finalBonesMatrices[" + std::to_string(i) + "]", finalizedBoneTransforms[i]);
	}

public:

	AnimationPose GetAnimationPose()
	{

		AnimationPose pose;
		pose.boneTransforms = animator.GetBonePoseArray();

		return pose;
	}

	void PasteAnimationPose(AnimationPose pose)
	{
		animator.ApplyBonePoseArray(pose.boneTransforms);
		boneTransforms = animator.getBoneMatrices();
	}

	void PlayAnimation(string name, float interpIn = 0.12)
	{
		animator.set(name);
		PlayAnimation(interpIn);
	}

	void FinalizeFrameData()
	{
		StaticMesh::FinalizeFrameData();
		finalizedBoneTransforms = boneTransforms;
	}

	void PlayAnimation(float interpIn = 0.12)
	{

		if (firstAnimation || animator.m_currAnim == nullptr)
		{
			interpIn = 0;
			firstAnimation = false;
		}

		if (interpIn > 0.01)
		{
			blendStartPose = GetAnimationPose();
			blendStartTime = Time::GameTime;
			blendEndTime = Time::GameTime + interpIn;
		}

		animator.play();
	}

	void Update(float timeScale = 1)
	{
		animator.update(Time::DeltaTimeF * timeScale);

		float blendProgress = GetBlendInProgress();

		if (blendProgress < 0.995)
		{
			AnimationPose currentPose = GetAnimationPose();


			AnimationPose newPose = AnimationPose::Lerp(blendStartPose, currentPose, blendProgress);

			PasteAnimationPose(newPose);

		}

		boneTransforms = animator.getBoneMatrices();
	}

	void SetLooped(bool looped)
	{
		animator.Loop = looped;
	}

	bool GetLooped()
	{
		return animator.Loop;
	}

	void LoadFromFile(string path)
	{

		StaticMesh::LoadFromFile(path);

		boneTransforms.resize(model->boneInfoMap.size());

		for (int i = 0; i < boneTransforms.size(); i++)
		{
			boneTransforms[i] = glm::identity<mat4>();
		}

		animator = roj::Animator(model);
	}

};