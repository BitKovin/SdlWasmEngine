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

class SkeletalMesh : public StaticMesh
{
private:

	roj::Animator animator;

	std::vector<mat4> boneTransforms;

	std::vector<mat4> finalizedBoneTransforms;


protected:

	void ApplyAdditionalShaderParams(ShaderProgram* shader_program)
	{
		for (int i = 0; i < finalizedBoneTransforms.size(); ++i)
			shader_program->SetUniform("finalBonesMatrices[" + std::to_string(i) + "]", finalizedBoneTransforms[i]);
	}

public:

	void PlayAnimation(string name)
	{
		animator.set(name);
		animator.play();
	}

	void FinalizeFrameData()
	{
		StaticMesh::FinalizeFrameData();
		finalizedBoneTransforms = boneTransforms;
	}

	void PlayAnimation()
	{
		animator.play();
	}

	void Update(float timeScale = 1)
	{
		animator.update(Time::DeltaTimeF * timeScale);
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