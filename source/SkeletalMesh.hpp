#pragma once


#include <vector>

#include "ShaderManager.h"

#include "IDrawMesh.h"

#include "VertexData.h"

#include "MathHelper.hpp"

#include "skinned_model.hpp"

#include "glm.h"

using namespace std;

class SkeletalMesh : public IDrawMesh
{

private:

	vec3 finalPosition = vec3(0);
	vec3 finalRotation = vec3(0);
	vec3 finalSize = vec3(0);

public:

	roj::SkinnedModel* model = nullptr;

	std::vector<mat4> boneTransforms;

	Texture* ColorTexture = nullptr;

	vec3 Position = vec3(0);
	vec3 Rotation = vec3(0);
	vec3 Size = vec3(1);

	SkeletalMesh()
	{

	}
	~SkeletalMesh()
	{

	}

	void FinalizeFrameData()
	{
		finalPosition = Position;
		finalRotation = Rotation;
		finalSize = Size;
	}



	//obj or gml files are strongly recommended
	void LoadFromFile(const string& path)
	{

		model = AssetRegistry::GetSkinnedModelFromFile(path);

		boneTransforms.resize(model->boneInfoMap.size());

		for (int i = 0; i < boneTransforms.size(); i++)
		{
			boneTransforms[i] = glm::identity<mat4>();
		}

	}



	void DrawForward(mat4x4 view, mat4x4 projection)
	{

		ShaderProgram* shader_program = ShaderManager::GetShaderProgram("skeletal");

		shader_program->UseProgram();

		shader_program->SetTexture("u_texture", ColorTexture);


		mat4x4 world = translate(finalPosition) * MathHelper::GetRotationMatrix(finalRotation) * scale(finalSize);

		shader_program->SetUniform("view", view);
		shader_program->SetUniform("projection", projection);

		shader_program->SetUniform("world", world);

		for (int i = 0; i < boneTransforms.size(); ++i)
			shader_program->SetUniform("finalBonesMatrices[" + std::to_string(i) + "]", boneTransforms[i]);


		for (const roj::SkinnedMesh& mesh : model->meshes)
		{
			mesh.VAO->Bind();
			glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(mesh.VAO->IndexCount), GL_UNSIGNED_INT, 0);
		}


	}

	void DrawDepth(mat4x4 viewProjection)
	{

	}

	void DrawShadow(mat4x4 viewProjection)
	{

	}




private:

};