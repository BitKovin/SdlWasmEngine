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

	// Last model->loadState.generation we've actually wired drawCommands (and,
	// for SkeletalMesh, bones) against - see PreFinalize() in the .cpp. Starts
	// at 0, same as a freshly-loaded model's generation before anything's landed.
	uint64_t lastWiredGeneration = 0;

	// RebuildDrawCommands() + OnAssetsReloaded() + stamps lastWiredGeneration.
	// Called both eagerly from LoadFromFile() (sync case - model's already
	// fully there) and from PreFinalize() when generation changes underneath
	// us (async case). Keeping both call sites doing exactly this, and only
	// this, is what makes the "wire once, at a fixed point" guarantee hold.
	void RewireForCurrentModel();

protected:

	// Called once, from RewireForCurrentModel(), whenever the model's asset
	// tier has actually changed since we last looked. Override to react to
	// newly-arrived (or newly-dropped) data - see SkeletalMesh::OnAssetsReloaded().
	virtual void OnAssetsReloaded() {}


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
	//
	// Lazy by default outside a level load: returns immediately, and unless
	// requestedTier == AssetLoadTier::Visual, geometry/textures stream in
	// over the next frames instead of blocking here (see AssetRegistry.h) -
	// PreFinalize() notices once something's actually landed and wires it in
	// then, same as it does right away for a fully-synchronous load. Inside
	// BeginLevelLoad()/EndLevelLoad() this is always fully synchronous,
	// exactly as before, regardless of requestedTier.
	virtual void LoadFromFile(const string& path, AssetLoadTier requestedTier = AssetLoadTier::Logic)
	{

		if (skipMeshLoad)
		{
			model = AssetRegistry::GetSkinnedAnimationFromFile(path, requestedTier);
		}
		else
		{
			model = AssetRegistry::GetSkinnedModelFromFile(path, requestedTier);
		}

		// Wires whatever's already there right now (everything, for a sync
		// load; possibly nothing yet, for a lazy one) - PreFinalize() picks
		// up anything that arrives later on its own.
		RewireForCurrentModel();

	}

	// Highest tier that is fully ready for this mesh instance, including every
	// texture that has been requested so far. Returns Visual only when
	// AllAssetsLoaded() would be true; otherwise the model's own currentTier
	// (Logic / None). Textures that have never been touched yet are ignored.
	AssetLoadTier GetLoadState() const;

	// Highest tier that has been requested for this model (queuedUpTo). Does not
	// wait for completion – useful for deciding whether a further RequestVisualLoad
	// is still needed.
	AssetLoadTier GetDesiredLoadState() const;

	// True once this model's geometry is uploaded and every submesh texture
	// that's actually been requested so far has finished uploading too.
	bool AllAssetsLoaded() const
	{
		if (model == nullptr) return false;
		if (model->loadState.currentTier.load(std::memory_order_acquire) != AssetLoadTier::Visual) return false;

		for (const auto& mesh : model->meshes)
		{
			if (mesh.cachedBaseColor && mesh.cachedBaseColor->loadState.currentTier.load(std::memory_order_acquire) != AssetLoadTier::Visual)
				return false;
			if (mesh.cachedEmissiveColor && mesh.cachedEmissiveColor->loadState.currentTier.load(std::memory_order_acquire) != AssetLoadTier::Visual)
				return false;
		}

		return true;
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
