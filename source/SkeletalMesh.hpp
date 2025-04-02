#pragma once


#include <vector>

#include "ShaderManager.h"

#include "IDrawMesh.h"

#include "VertexData.h"

#include "MathHelper.hpp"

#include "skinned_model.hpp"

using namespace std;

class SkeletalMesh : IDrawMesh
{
public:

	roj::SkinnedModel* model = nullptr;

	std::vector<mat4> boneTransforms;

	SkeletalMesh()
	{

	}
	~SkeletalMesh()
	{

	}

	void DrawForward(mat4x4 view, mat4x4 projection, Texture* texture)
	{

		ShaderProgram* shader_program = ShaderManager::GetShaderProgram("skeletal");

		shader_program->UseProgram();

		shader_program->SetTexture("u_texture", texture);

		vec3 pos = vec3(0, 0, 2);

		vec3 rot = vec3(0);

		mat4x4 world = scale(vec3(1)) * MathHelper::GetRotationMatrix(rot) * translate(pos);

		shader_program->SetUniform("view", view);
		shader_program->SetUniform("projection", projection);

		shader_program->SetUniform("world", world);

		for (int i = 0; i < boneTransforms.size(); ++i)
			shader_program->SetUniform("finalBonesMatrices[" + std::to_string(i) + "]", boneTransforms[i]);

		for (roj::SkinnedMesh mesh : *model)
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