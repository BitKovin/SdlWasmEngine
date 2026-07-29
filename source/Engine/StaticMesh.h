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

#include <BgfxStateManager.h>

class Entity;

class StaticMesh : public IDrawMesh
{
private:

	friend class Renderer;

	LightVolPointData lastLightVolData = LightVolPointData();

	vec3 lastLightDir = vec3(0.0f, -1.0f, 0.0f); //so we don't recaclulate it twice per frame

protected:

	virtual void ApplyAdditionalShaderParams(Shader* shader_program)
	{
		for (auto& param : finalizedMeshCustomShaderParams)
		{
			shader_program->SetUniform(param.first, param.second);
		}
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
	vec4 finalizedColor = vec4(1);
	std::unordered_set<std::string> finalMeshHideList{};
	std::map<std::string, vec4> finalizedMeshCustomShaderParams{};
	bool finalizedCameraVisible = true;

	BoundingBox finalizedBoundingBox;

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

	bool GravityAlignedRotation = false; //should engine automatically rotate model to align with gravity vector as part of model matrix?

	float Brightness = 1.2f;

	BgfxStateManager::Blend blendMode = BgfxStateManager::Blend::Alpha;
	
	vec4 Color = vec4(1.0f);

	std::unordered_set<std::string> MeshHideList{};

	std::map<std::string, vec4> MeshCustomShaderParams{};

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

	mat4 GetWorldMatrix();

	virtual LightVolPointData GetLightVolData();

	vector<MeshUtils::PositionVerticesIndices> GetNavObstacleMeshes()
	{
		vector<MeshUtils::PositionVerticesIndices> result;

		if (model == nullptr)
			return result;

		mat3 world = GetWorldMatrix();

		for (auto& mesh : model->meshes)
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
		return distance(Camera::position, Position) * (IsViewmodel ? 0.01 : 1);
	}

	void FinalizeFrameData();

	void PreFinalize() override;

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

	bool PrecalculateCameraVisible();

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


	vec3 GetShadowColorMult();                                   // cached per-frame
	void DrawShadowVolumeStencil(mat4x4 view, mat4x4 projection); // Pass A only
	static void ApplyShadowDarkening(const vec3& shadowColor);    // Pass B only
	static void ClearShadowStencil();                             // Pass C only

	void DrawMeshShadow(mat4x4 view, mat4x4 projection);          // kept, single-mesh convenience wrapper


	void PreloadAssets();

	private:
		uint64_t shadowColorMultFrame = ~0ull;
		vec3     cachedShadowColorMult = vec3(1.0f);

};