#pragma once

#include <vector>

#include "ShaderManager.h"

#include "IDrawMesh.h"

#include "VertexData.h"

#include "MathHelper.hpp"

#include "skinned_model.hpp"

#include "glm.h"


using namespace std;

class StaticMesh : public IDrawMesh
{
private:

	vec3 finalPosition = vec3(0);
	vec3 finalRotation = vec3(0);
	vec3 finalScale = vec3(0);

protected:

	virtual void ApplyAdditionalShaderParams(ShaderProgram* shader_program)
	{

	}

public:

	roj::SkinnedModel* model = nullptr;

	string PixelShader = "default_pixel";

	Texture* ColorTexture = nullptr;

	vec3 Position = vec3(0);
	vec3 Rotation = vec3(0);
	vec3 Scale = vec3(1);

	StaticMesh()
	{

	}
	~StaticMesh()
	{

	}

	float GetDistanceToCamera()
	{
		return distance(Camera::position, Position) * (IsViewmodel ? 0.1 : 1);
	}

	void FinalizeFrameData()
	{
		finalPosition = Position;
		finalRotation = Rotation;
		finalScale = Scale;
	}



	//obj or gml files are strongly recommended
	virtual void LoadFromFile(const string& path)
	{

		model = AssetRegistry::GetSkinnedModelFromFile(path);

	}

	bool IsInFrustrum(Frustum frustrum)
	{

		auto sphere = model->boundingSphere.Transform(Position, Rotation, Scale);

		return frustrum.IsSphereVisible(sphere.offset, sphere.Radius);
	};
	


	void DrawForward(mat4x4 view, mat4x4 projection)
	{

		ShaderProgram* shader_program = ShaderManager::GetShaderProgram("skeletal", PixelShader);

		shader_program->UseProgram();


		mat4x4 world = translate(finalPosition) * MathHelper::GetRotationMatrix(finalRotation) * scale(finalScale);

		shader_program->SetUniform("view", view);
		shader_program->SetUniform("projection", projection);

		shader_program->SetUniform("world", world);

		shader_program->SetUniform("isViewmodel", IsViewmodel);

		ApplyAdditionalShaderParams(shader_program);


		for (const roj::SkinnedMesh& mesh : model->meshes)
		{

			if (ColorTexture == nullptr)
			{

				string baseTextureName;

				for (auto texture : mesh.textures)
				{
					if (texture.type == aiTextureType_BASE_COLOR)
					{
						baseTextureName = texture.src;
						break;
					}
				}

				const string textureRoot = "GameData/Textures/";

				shader_program->SetTexture("u_texture", AssetRegistry::GetTextureFromFile(textureRoot + baseTextureName));
			}
			else
			{
				shader_program->SetTexture("u_texture", ColorTexture);
			}

			mesh.VAO->Bind();
			glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(mesh.VAO->IndexCount), GL_UNSIGNED_INT, 0);
		}


	}

	void DrawDepth(mat4x4 view, mat4x4 projection)
	{
		ShaderProgram* shader_program = ShaderManager::GetShaderProgram("skeletal", "empty_pixel");

		shader_program->UseProgram();

		mat4x4 world = translate(finalPosition) * MathHelper::GetRotationMatrix(finalRotation) * scale(finalScale);

		shader_program->SetUniform("view", view);
		shader_program->SetUniform("projection", projection);

		shader_program->SetUniform("world", world);

		shader_program->SetUniform("isViewmodel", IsViewmodel);

		ApplyAdditionalShaderParams(shader_program);


		for (const roj::SkinnedMesh& mesh : model->meshes)
		{
			mesh.VAO->Bind();
			glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(mesh.VAO->IndexCount), GL_UNSIGNED_INT, 0);
		}
	}

	void DrawShadow(mat4x4 view, mat4x4 projection)
	{
		ShaderProgram* shader_program = ShaderManager::GetShaderProgram("skeletal", "empty_pixel");

		shader_program->UseProgram();

		mat4x4 world = translate(finalPosition) * MathHelper::GetRotationMatrix(finalRotation) * scale(finalScale);

		shader_program->SetUniform("view", view);
		shader_program->SetUniform("projection", projection);

		shader_program->SetUniform("world", world);

		ApplyAdditionalShaderParams(shader_program);


		for (const roj::SkinnedMesh& mesh : model->meshes)
		{
			mesh.VAO->Bind();
			glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(mesh.VAO->IndexCount), GL_UNSIGNED_INT, 0);
		}
	}




private:

};