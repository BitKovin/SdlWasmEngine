#include "StaticMesh.h"

#include "Renderer/Renderer.h"

#include "Entity.h"

#include "Helpers/StringHelper.h"

#include "LightSystem/LightManager.h"

#include "DrawCommands/DetailShadowUtils.h"

#include <BgfxStateManager.h>
#include <Renderer/Abstractions/ViewIdManager.h>

#include <World/WorldOrientationManager.h>

mat4 StaticMesh::GetWorldMatrix()
{

	mat4 posOffset = translate(positionOffset);
	mat4 rotOffset = MathHelper::GetRotationMatrix(rotationOffset);

	quat worldOrientation = glm::identity<quat>();

	if (GravityAlignedRotation)
	{
		worldOrientation = WorldOrientationManager::GetWorldRotationQuat();
	}

	return translate(Position) * glm::toMat4(worldOrientation) * rotOffset * MathHelper::GetRotationMatrix(Rotation) * scale(Scale) * posOffset;
}

LightVolPointData StaticMesh::GetLightVolData()
{

	if (OwnerEntity == nullptr ||
		model == nullptr) return LightVolPointData{ vec3(0),vec3(1),vec3(0) };

	return OwnerEntity->GetLightVolData(IsViewmodel);

}

// Standard luminance weights (Rec. 709 / sRGB), assumes linear color values
static inline float GetLuminance(const glm::vec3& color)
{
	return glm::dot(color, glm::vec3(0.2126f, 0.7152f, 0.0722f));
}

// Ensures 'light' has at least 'minBrightness' luminance, preserving hue; pure black returns white at minBrightness.
static glm::vec3 EnsureMinBrightness(const glm::vec3& light, float minBrightness)
{
	if (minBrightness <= 0.0f)
		return light;

	float brightness = GetLuminance(light);

	if (brightness >= minBrightness)
		return light; // already bright enough

	constexpr float kEpsilon = 1e-6f;
	if (brightness <= kEpsilon)
		return glm::vec3(minBrightness); // fully black -> white at min brightness

	float scale = minBrightness / brightness;
	return light * scale;
}

// Same formula as the old GetShadowColorMult(), minus the per-frame cache (redundant now that this only runs once per mesh per frame).
static vec3 ComputeShadowColorMult(const vec3& ambientColor, const vec3& directColor)
{
	const vec3 directionalColor = directColor + ambientColor * 0.0f;
	constexpr float kEps = 0.001f;

	vec3 shadowColor;
	shadowColor.r = (directionalColor.r > kEps) ? (ambientColor.r / directionalColor.r) : 1.0f;
	shadowColor.g = (directionalColor.g > kEps) ? (ambientColor.g / directionalColor.g) : 1.0f;
	shadowColor.b = (directionalColor.b > kEps) ? (ambientColor.b / directionalColor.b) : 1.0f;
	shadowColor = clamp(shadowColor, vec3(0.0f), vec3(1.0f));

	float ambientLum = dot(ambientColor, vec3(0.299f, 0.587f, 0.114f));
	float directionalLum = dot(directionalColor, vec3(0.299f, 0.587f, 0.114f));
	float uniformShadow = (directionalLum > kEps)
		? glm::clamp(ambientLum / directionalLum, 0.0f, 1.0f) : 1.0f;

	float blendT = 1.0f - uniformShadow;
	shadowColor = mix(vec3(uniformShadow), shadowColor, blendT);
	vec3 d = vec3(1.0f) - shadowColor;
	shadowColor = vec3(1.0f) - d * d;
	shadowColor += 0.02f;
	shadowColor = clamp(shadowColor, vec3(0.0f), vec3(1.0f));
	shadowColor = 1.0f - shadowColor;
	shadowColor *= 0.85f;
	shadowColor = 1.0f - shadowColor;

	return shadowColor;
}

void StaticMesh::PreFinalize()
{
	// Runs in parallel, one call per mesh - the only place safe to read OwnerEntity (see the top-level README).

	// A model loaded lazily (see StaticMesh::LoadFromFile / AssetRegistry.h)
	// may still be streaming in. Fixed, once-per-frame point where a newly-
	// arrived (or newly-dropped) tier gets wired into draw commands - never
	// mid-Update(), so gameplay code never sees a half-applied change.
	if (model != nullptr && model->loadState.generation.load(std::memory_order_acquire) != lastWiredGeneration)
		RewireForCurrentModel();

	finalizedBoundingBox = GetBoundingBox();

	finalizedCameraVisible = PrecalculateCameraVisible();

	finalizedWorld = GetWorldMatrix();

	if (model != nullptr)
	{
		LightVolPointData lightData = GetLightVolData();

		finalizedLightAmbient = EnsureMinBrightness(lightData.ambientColor, 0.01f);
		finalizedLightDirect = EnsureMinBrightness(lightData.directColor, 0.03f);
		finalizedLightDirection = lightData.direction;

		finalizedShadowColorMult = CastDetailShadows
			? ComputeShadowColorMult(lightData.ambientColor, lightData.directColor)
			: vec3(1.0f); // "fully lit" -> Renderer::DrawDetailShadows skips it
	}

	finalizedMeshCustomShaderParams.clear();
	CollectCustomShaderParams(finalizedMeshCustomShaderParams);
}

bool StaticMesh::IsInFrustrum(Frustum frustrum)
{
	if (model == nullptr) return false;

	auto bounds = finalizedBoundingBox;

	return frustrum.IsBoxVisible(bounds.Min, bounds.Max);
}

BoundingBox StaticMesh::GetBoundingBox()
{

	if (model == nullptr) return BoundingBox();

	return model->boundingBox.Transform(GetWorldMatrix());

}


bool StaticMesh::PrecalculateCameraVisible()
{
	if (IsViewmodel) return isVisible();

	if((IsInFrustrum(Camera::frustum) && isVisible()) == false) return false;

	if (Level::Current->BspData.m_numOfVerts)
	{
		int cameraC = Level::Current->BspData.FindClusterAtPosition(Camera::finalizedPosition);

		auto bounds = finalizedBoundingBox;

		vec3 min = bounds.Min;
		vec3 max = bounds.Max;


		vec3 points[8] =
		{
			{min.x, min.y, min.z},
			{max.x, min.y, min.z},
			{min.x, max.y, min.z},
			{max.x, max.y, min.z},

			{min.x, min.y, max.z},
			{max.x, min.y, max.z},
			{min.x, max.y, max.z},
			{max.x, max.y, max.z}
		};

		bool visible = false;

		for (int i = 0; i < 8; i++)
		{
			int targetC = Level::Current->BspData.FindClusterAtPosition(points[i]);

			if (Level::Current->BspData.IsClusterVisible(cameraC, targetC))
			{
				visible = true;
				break;
			}
		}

		if (!visible)
			return false;
	}

	return true;
}

bool StaticMesh::IsCameraVisible()
{
	if (model == nullptr)
		return false;

	return finalizedCameraVisible;
}

void StaticMesh::RebuildDrawCommands()
{
	drawCommands.clear();

	if (model == nullptr) return;

	drawCommands.reserve(model->meshes.size());
	for (size_t i = 0; i < model->meshes.size(); ++i)
		drawCommands.push_back(CreateDrawCommand());

}

void StaticMesh::RewireForCurrentModel()
{
	RebuildDrawCommands();
	OnAssetsReloaded();
	lastWiredGeneration = model ? model->loadState.generation.load(std::memory_order_acquire) : 0;
}

void StaticMesh::CollectDrawCommands(std::vector<IDrawCommand*>& outCommands)
{
	for (auto& cmd : finalDrawCommands)
	{
		if (cmd->Hidden) continue;
		outCommands.push_back(cmd.get());
	}
}

void StaticMesh::FinalizeFrameData()
{
	// Main thread. Resolves/creates bgfx-backed resources and commits already-computed plain data from PreFinalize() onto the commands.

	finalizedPosition = Position;
	finalizedRotation = Rotation;
	finalizedScale = Scale;
	finalMeshHideList = std::unordered_set<std::string>(MeshHideList);
	finalizedColor = Color;

	if (model == nullptr) return;

	for (size_t i = 0; i < model->meshes.size() && i < drawCommands.size(); ++i)
	{
		roj::SkinnedMesh& mesh = model->meshes[i];
		StaticMeshDrawCommand& cmd = *drawCommands[i];

		bool isHiddenMesh = false;
		for (const auto& hideName : finalMeshHideList)
		{
			if (StringHelper::StartsWith(mesh.name, hideName) || hideName == mesh.name)
			{
				isHiddenMesh = true;
				break;
			}
		}
		cmd.Hidden = isHiddenMesh;

		cmd.WorldMatrix = finalizedWorld;
		cmd.WorldBounds = finalizedBoundingBox;

		cmd.IsViewmodel = IsViewmodel;
		cmd.OnlyShadows = OnlyShadows;
		cmd.ReceiveDetailShadows = ReceiveDetailShadows;

		cmd.DepthWrite = DepthWrite;
		cmd.DepthPrePass = DepthPrePath;
		cmd.TwoSided = TwoSided;
		cmd.Brightness = Brightness;
		cmd.BlendMode = blendMode;
		cmd.PixelShader = PixelShader;

		cmd.LightAmbientColor = finalizedLightAmbient;
		cmd.LightDirectColor = finalizedLightDirect;
		cmd.LightDirection = finalizedLightDirection;

		cmd.ShadowColorMultValue = finalizedShadowColorMult;
		cmd.ShadowLightDirection = finalizedLightDirection;

		cmd.Material.Color = finalizedColor;
		cmd.Material.Masked = Masked;
		cmd.Material.ViewmodelScaleFactor = ViewmodelScaleFactor;
		cmd.Material.CustomId = CustomId;

		// Plain data from PreFinalize() - not a callback into this mesh.
		cmd.CustomShaderParams = finalizedMeshCustomShaderParams;

		if (isHiddenMesh)
			continue; // no point resolving textures/shadow data for a submesh that won't be drawn

		if (ColorTexture == nullptr && ColorTextureId == 0)
		{
			string baseTextureName;
			string emissiveTextureName;

			for (auto& texture : mesh.textures)
			{
				if (texture.type == aiTextureType_BASE_COLOR)
					baseTextureName = texture.src;

				if (texture.type == aiTextureType_EMISSIVE)
					emissiveTextureName = texture.src;
			}

			// Resource creation - main thread only, hence here and not PreFinalize.
			// PreloadAssets() already fills cachedBaseColor synchronously
			// during the loading screen, so this only ever fires for
			// something that was never preloaded - i.e. exactly the
			// runtime-load case GetTextureFromFile is lazy-by-default for
			// (see AssetRegistry.h). Shader::SetTexture already falls back
			// to a neutral texture while ColorTexture is still loading, so
			// there's nothing else to gate on here.
			if (mesh.cachedBaseColor == nullptr)
			{
				const string textureRoot = TexturesLocation;
				mesh.cachedBaseColor = AssetRegistry::GetTextureFromFile(textureRoot + baseTextureName);
			}

			if (mesh.cachedEmissiveColor == nullptr)
			{
				const string& textureRoot = TexturesLocation;

				mesh.cachedEmissiveColor = emissiveTextureName != ""
					? AssetRegistry::GetTextureFromFile(textureRoot + emissiveTextureName)
					: AssetRegistry::GetTextureFromFile(textureRoot + StringHelper::Replace(baseTextureName, ".", "_em."));
			}

			cmd.Material.ColorTexture = mesh.cachedBaseColor;
			cmd.Material.ColorTextureId = 0;
			cmd.Material.EmissiveTexture = mesh.cachedEmissiveColor;
			cmd.Material.EmissiveTextureId = 0;
		}
		else
		{
			cmd.Material.ColorTexture = ColorTexture;
			cmd.Material.ColorTextureId = ColorTextureId;
			cmd.Material.EmissiveTexture = EmissiveTexture;
			cmd.Material.EmissiveTextureId = EmissiveTextureId;
		}

		cmd.VertexBuffer = mesh.vbh;
		cmd.IndexBuffer = mesh.ibh;

		if (CastDetailShadows)
		{
			const roj::ShadowVolumePrecomp& precomp = mesh.shadowVolumePrecomp;
			cmd.ShadowCapVbh = precomp.capVbh;
			cmd.ShadowCapIbh = precomp.capIbh;
			cmd.ShadowCapIndexCount = precomp.capIndexCount;
			cmd.ShadowEdgeVbh = precomp.edgeVbh;
			cmd.ShadowEdgeIbh = precomp.edgeIbh;
			cmd.ShadowEdgeIndexCount = precomp.edgeIndexCount;
		}
		else
		{
			cmd.ShadowCapVbh = BGFX_INVALID_HANDLE;
			cmd.ShadowCapIbh = BGFX_INVALID_HANDLE;
			cmd.ShadowCapIndexCount = 0;
			cmd.ShadowEdgeVbh = BGFX_INVALID_HANDLE;
			cmd.ShadowEdgeIbh = BGFX_INVALID_HANDLE;
			cmd.ShadowEdgeIndexCount = 0;
		}
	}

	finalDrawCommands = drawCommands;

}

void StaticMesh::DrawMeshShadow(mat4x4 view, mat4x4 projection)
{
	if (model == nullptr) return;

	vec3 shadowColor = vec3(1.0f);
	for (auto& cmd : finalDrawCommands)
	{
		if (cmd->Hidden) continue;
		shadowColor = cmd->GetShadowColorMult();
		break; // same value on every submesh - see FinalizeFrameData()
	}

	if (shadowColor.r >= 0.999f && shadowColor.g >= 0.999f && shadowColor.b >= 0.999f)
		return;

	auto savedState = BgfxStateManager::GetState();

	for (auto& cmd : finalDrawCommands)
	{
		if (cmd->Hidden) continue;
		cmd->DrawShadowVolumeStencil(view, projection);
	}

	DetailShadowUtils::ApplyShadowDarkening(shadowColor);
	DetailShadowUtils::ClearShadowStencil();

	BgfxStateManager::SetState(savedState);
}

void StaticMesh::PreloadAssets()
{

	for (roj::SkinnedMesh& mesh : model->meshes)
	{

		if (ColorTexture == nullptr)
		{

			string baseTextureName = "";
			string emissiveTextureName = "";

			for (auto texture : mesh.textures)
			{
				if (texture.type == aiTextureType_BASE_COLOR)
				{
					baseTextureName = texture.src;

				}

				if (texture.type == aiTextureType_EMISSIVE)
				{
					emissiveTextureName = texture.src;

				}

			}

			if (mesh.cachedBaseColor == nullptr)
			{
				const string textureRoot = TexturesLocation;

				mesh.cachedBaseColor = AssetRegistry::GetTextureFromFile(textureRoot + baseTextureName);
			}

			if (mesh.cachedEmissiveColor == nullptr)
			{
				const string& textureRoot = TexturesLocation;

				if (emissiveTextureName != "")
				{
					mesh.cachedEmissiveColor = AssetRegistry::GetTextureFromFile(textureRoot + emissiveTextureName);
				}
				else
				{
					mesh.cachedEmissiveColor = AssetRegistry::GetTextureFromFile(textureRoot + StringHelper::Replace(baseTextureName, ".", "_em."));
				}

			}

		}

	}
}


AssetLoadTier StaticMesh::GetLoadState() const
{
	if (model == nullptr)
		return AssetLoadTier::None;

	const AssetLoadTier modelTier = model->loadState.currentTier.load(std::memory_order_acquire);

	// Geometry (and bones) not even at Visual yet – report that directly.
	if (modelTier != AssetLoadTier::Visual)
		return modelTier;

	// Model claims Visual; verify every texture that has actually been
	// requested for its submeshes has also finished uploading.  Un-touched
	// cached* pointers are ignored (they will be filled lazily on first use).
	for (const auto& mesh : model->meshes)
	{
		if (mesh.cachedBaseColor &&
			mesh.cachedBaseColor->loadState.currentTier.load(std::memory_order_acquire) != AssetLoadTier::Visual)
			return AssetLoadTier::Logic;   // geometry ready, textures still streaming

		if (mesh.cachedEmissiveColor &&
			mesh.cachedEmissiveColor->loadState.currentTier.load(std::memory_order_acquire) != AssetLoadTier::Visual)
			return AssetLoadTier::Logic;
	}

	return AssetLoadTier::Visual;
}

AssetLoadTier StaticMesh::GetDesiredLoadState() const
{
	if (model == nullptr)
		return AssetLoadTier::None;

	return model->loadState.queuedUpTo.load(std::memory_order_acquire);
}