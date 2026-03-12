#pragma once

#include <vector>

#include "ShaderManager.h"

#include "IDrawMesh.h"

#include "VertexData.h"

#include "MathHelper.hpp"

#include "skinned_model.hpp"

#include "glm.h"

#include "BSP/Quake3Bsp.h"
#include <string>
#include <unordered_set>


class Entity;

class StaticMesh : public IDrawMesh
{
private:



protected:

	virtual void ApplyAdditionalShaderParams(Shader* shader_program)
	{

	}

	string PixelShader = "fs_default";

	Shader* forward_shader_program = nullptr;

	int numInstances = -1;

	void OnDispose()
	{

	}

	vec3 positionOffset = vec3();
	vec3 rotationOffset = vec3();

	bool skipMeshLoad = false;


	mat4 finalizedWorld;

	vec3 finalizedPosition = vec3(0);
	vec3 finalizedRotation = vec3(0);
	vec3 finalizedScale = vec3(1);
	std::unordered_set<std::string> finalMeshHideList{};

public:

	roj::SkinnedModel* model = nullptr;

	Entity* OwnerEntity = nullptr;

	Texture* ColorTexture = nullptr;
	Texture* EmissiveTexture = nullptr;

	uint32_t ColorTextureId = 0;
	uint32_t EmissiveTextureId = 0;

	vec3 Position = vec3(0);
	vec3 Rotation = vec3(0);

	vec3 Scale = vec3(1);

	string TexturesLocation = "";

	bool DepthWrite = true;

	bool Masked = false;

	bool CastShadows = true;
	bool Visible = true;
	bool CastHiddenShadows = false;
	bool CastDetailShadows = false;

	float ViewmodelScaleFactor = 1.0f;

	bool TwoSided = false;

	int CustomId = 0;

	bool DepthPrePath = true;

	float Brightness = 1.0f;

	std::unordered_set<std::string> MeshHideList{};

	StaticMesh(Entity* owner)
	{
		OwnerEntity = owner;
	}

	virtual ~StaticMesh() = default;

	void SetPixelShader(string name)
	{
		PixelShader = name;

		forward_shader_program = nullptr;

	}

	mat4 GetWorldMatrix()
	{

		mat4 posOffset = translate(positionOffset);
		mat4 rotOffset = MathHelper::GetRotationMatrix(rotationOffset);

		return translate(Position) * rotOffset * MathHelper::GetRotationMatrix(Rotation) * scale(Scale) * posOffset;
	}

	virtual LightVolPointData GetLightVolData();

	vector<MeshUtils::PositionVerticesIndices> GetNavObstacleMeshes()
	{
		vector<MeshUtils::PositionVerticesIndices> result;

		if (model == nullptr)
			return result;

		mat3 world = GetWorldMatrix();

		for (auto mesh : model->meshes)
		{

			MeshUtils::PositionVerticesIndices meshData;

			meshData.indices = mesh.indices;

			for (auto& vertex : mesh.vertices)
			{
				meshData.vertices.push_back(world * vertex.Position);
			}

			result.push_back(meshData);

		}

		return result;
	}

	float GetDistanceToCamera()
	{
		return distance(Camera::position, Position) * (IsViewmodel ? 0.1 : 1);
	}

	void FinalizeFrameData();



	//obj or gml files are strongly recommended
	virtual void LoadFromFile(const string& path)
	{

		if (skipMeshLoad)
		{
			model = AssetRegistry::GetSkinnedAnimationFromFile(path);
		}
		else
		{
			model = AssetRegistry::GetSkinnedModelFromFile(path);
		}

		

	}

	bool IsInFrustrum(Frustum frustrum);

	BoundingBox GetBoundingBox();

	virtual bool isVisible()
	{

		if (model == nullptr)
			return false;

		return Visible;
	}

	bool IsCameraVisible();

	bool IsShadowVisible()
	{

		if (model == nullptr)
			return false;

		return (isVisible() && CastShadows) || CastHiddenShadows;
	}

	bool IsDetailShadow() { return CastDetailShadows; }

	void DrawForward(mat4x4 view, mat4x4 projection);

	void DrawDepth(mat4x4 view, mat4x4 projection);
	void DrawCustomId(mat4x4 view, mat4x4 projection);

	void DrawShadow(mat4x4 view, mat4x4 projection);

	void PreloadAssets();


private:

};