#pragma once

#include <vector>
#include <memory>

#include "ShaderManager.h"

#include "IDrawMesh.h"
#include "DrawCommands/IDrawCommand.h"
#include "DrawCommands/StaticMeshDrawCommand.h"

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

	// Renderer never dynamic_cast<StaticMesh*>s anymore - "friend class Renderer;" is gone.

	// One persistent IDrawCommand per entry in model->meshes, owned here.
	std::vector<std::shared_ptr<StaticMeshDrawCommand>> drawCommands;
	std::vector<std::shared_ptr<StaticMeshDrawCommand>> finalDrawCommands;

	void RebuildDrawCommands();

protected:

	// Called from PreFinalize() to produce this frame's custom shader params as plain data - override instead of touching a Shader* directly.
	virtual void CollectCustomShaderParams(std::map<std::string, vec4>& out)
	{
		for (auto& param : MeshCustomShaderParams)
		{
			out[param.first] = param.second;
		}
	}

	// Overridden by SkeletalMesh so RebuildDrawCommands() produces SkeletalMeshDrawCommand instead.
	virtual std::shared_ptr<StaticMeshDrawCommand> CreateDrawCommand()
	{
		return std::make_unique<StaticMeshDrawCommand>();
	}

	// For subclasses reaching into a specific submesh's command beyond Material (see SkeletalMesh::LoadFromFile).
	StaticMeshDrawCommand& GetDrawCommandAt(size_t submeshIndex)
	{
		return *drawCommands.at(submeshIndex);
	}

	string PixelShader = "fs_default";

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

	// Computed in PreFinalize(), consumed in FinalizeFrameData() - see PreFinalize() in the .cpp.
	vec3 finalizedLightAmbient = vec3(0);
	vec3 finalizedLightDirect = vec3(0);
	vec3 finalizedLightDirection = vec3(0, -1, 0);
	vec3 finalizedShadowColorMult = vec3(1.0f);

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

	void CollectDrawCommands(std::vector<IDrawCommand*>& outCommands) override;

	// mesh->GetMaterial(0).Masked = true; - index must be < model->meshes.size(), only valid after LoadFromFile.
	StaticMeshColorEmissiveMaterial& GetMaterial(size_t submeshIndex = 0)
	{
		return drawCommands.at(submeshIndex)->Material;
	}

	size_t GetSubMeshCount() const { return drawCommands.size(); }

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

		RebuildDrawCommands();

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

	// Draws this mesh's own detail shadow immediately, outside the batched Renderer::DrawDetailShadows grouping.
	void DrawMeshShadow(mat4x4 view, mat4x4 projection);

	void PreloadAssets();

};
