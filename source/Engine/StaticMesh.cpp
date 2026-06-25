#include "StaticMesh.h"

#include "Renderer/Renderer.h"

#include "Entity.h"

#include "Helpers/StringHelper.h"

#include "LightSystem/LightManager.h"

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

void StaticMesh::FinalizeFrameData()
{

	finalizedWorld = GetWorldMatrix();
	finalizedPosition = Position;
	finalizedRotation = Rotation;
	finalizedScale = Scale;
	finalMeshHideList = std::unordered_set<std::string>(MeshHideList);
	finalizedColor = Color;
	finalizedMeshCustomShaderParams = MeshCustomShaderParams;


	finalizedBoundingBox = GetBoundingBox();
}

bool StaticMesh::IsInFrustrum(Frustum frustrum)
{
	if (model == nullptr) return false;

	//DebugDraw::Bounds(sphere.offset - vec3(sphere.Radius), sphere.offset + vec3(sphere.Radius), 0.01f);

	auto bounds = GetBoundingBox();

	return frustrum.IsBoxVisible(bounds.Min, bounds.Max);
}

BoundingBox StaticMesh::GetBoundingBox()
{

	if (model == nullptr) return BoundingBox();

	return model->boundingBox.Transform(GetWorldMatrix());

}


bool StaticMesh::IsCameraVisible()
{
	if (model == nullptr)
		return false;

	if (IsViewmodel) return isVisible();

	if (Level::Current->BspData.m_numOfVerts)
	{
		int cameraC = Level::Current->BspData.FindClusterAtPosition(Camera::finalizedPosition);

		auto bounds = GetBoundingBox();

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

	return IsInFrustrum(Camera::frustum) && isVisible();
}

void StaticMesh::DrawForward(mat4x4 view, mat4x4 projection)
{

	if (model == nullptr) return;

	auto startState = BgfxStateManager::GetState();


	BgfxStateManager::SetWriteDepth(DepthWrite);



	BgfxStateManager::SetBlend(blendMode);

	BgfxStateManager::SetCull(TwoSided ? BgfxStateManager::Cull::None : BgfxStateManager::Cull::CW);


	if (forward_shader_program == nullptr)
		forward_shader_program = ShaderManager::GetShaderProgram("vs_default", PixelShader);

	forward_shader_program->UseProgram();


	forward_shader_program->SetUniform("masked", Masked);

	Renderer::SetSurfaceShaderUniforms(forward_shader_program, Brightness);

	mat4x4 world = finalizedWorld;

	auto lightData = GetLightVolData();

	lastLightDir = lightData.direction * -1.0f;
	lastLightVolData = lightData;

	forward_shader_program->SetUniform("modelColor", finalizedColor);

	forward_shader_program->SetUniform("light_color", lightData.ambientColor);
	forward_shader_program->SetUniform("direct_light_color", lightData.directColor);
	forward_shader_program->SetUniform("direct_light_dir", lightData.direction);

	forward_shader_program->SetUniform("view", view);
	forward_shader_program->SetUniform("projection", projection);
	forward_shader_program->SetUniform("viewmodelScaleFactor", ViewmodelScaleFactor);

	forward_shader_program->SetUniform("world", world);

	forward_shader_program->SetUniform("isViewmodel", IsViewmodel);

	forward_shader_program->SetUniform("view", view);

	forward_shader_program->SetUniform("customId", CustomId);
	forward_shader_program->SetUniform("flipedNormals", false);

	ApplyAdditionalShaderParams(forward_shader_program);


	auto bounds = GetBoundingBox();

	LightManager::ApplyPointLightToShader(forward_shader_program, bounds.Min, bounds.Max);

	for (roj::SkinnedMesh& mesh : model->meshes)
	{

		bool isHiddenMesh = false;
		for (const auto& hideName : finalMeshHideList)
		{
			if (StringHelper::StartsWith(mesh.name, hideName) || hideName == mesh.name)
			{
				isHiddenMesh = true;
				break;
			}
		}
		if (isHiddenMesh)
			continue;

		if (ColorTexture == nullptr && ColorTextureId == 0)
		{

			string baseTextureName;
			string emissiveTextureName;

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

			Texture* texture = mesh.cachedBaseColor;
			Texture* textureEm = mesh.cachedEmissiveColor;

			if (Transparent && mesh.transparentTexture)
			{

				BgfxStateManager::SetWriteDepth(false);

			}


			forward_shader_program->SetTexture("u_texture", texture);
			forward_shader_program->SetTexture("u_textureEmissive", textureEm);
		}
		else
		{

			if (ColorTexture)
			{
				forward_shader_program->SetTexture("u_texture", ColorTexture);
			}
			else
			{
				forward_shader_program->SetTexture("u_texture", ColorTextureId);
			}

			forward_shader_program->SetTexture("u_textureEmissive", 0);
			if (EmissiveTexture)
			{
				forward_shader_program->SetTexture("u_textureEmissive", EmissiveTexture);
			}
			else
			{
				forward_shader_program->SetTexture("u_textureEmissive", EmissiveTextureId);
			}
		}


		bgfx::setVertexBuffer(0, mesh.vbh);
		bgfx::setIndexBuffer(mesh.ibh);

		BgfxStateManager::Apply();

		forward_shader_program->Submit(ViewIdManager::GetCurrentId());

	}

	BgfxStateManager::SetState(startState);

}

void StaticMesh::DrawDepth(mat4x4 view, mat4x4 projection)
{
	if (model == nullptr) return;
	if (DepthPrePath == false) return;

	if (Color.a < 1.0f) return;

	auto startState = BgfxStateManager::GetState();

	BgfxStateManager::SetCull(TwoSided ? BgfxStateManager::Cull::None : BgfxStateManager::Cull::CW);

	const bool mask = Transparent || Masked;

	Shader* shader = ShaderManager::GetShaderProgram("vs_default", mask ? "fs_mask" : "fs_empty");

	shader->UseProgram();

	mat4x4 world = finalizedWorld;

	shader->SetUniform("view", view);
	shader->SetUniform("projection", projection);
	shader->SetUniform("viewmodelScaleFactor", ViewmodelScaleFactor);
	shader->SetUniform("world", world);
	shader->SetUniform("isViewmodel", IsViewmodel);

	ApplyAdditionalShaderParams(shader);

	for (roj::SkinnedMesh& mesh : model->meshes)
	{
		bool isHiddenMesh = false;
		for (const auto& hideName : finalMeshHideList)
		{
			if (StringHelper::StartsWith(mesh.name, hideName) || hideName == mesh.name)
			{
				isHiddenMesh = true;
				break;
			}
		}
		if (isHiddenMesh)
			continue;

		if (mask)
		{
			if (ColorTexture == nullptr && ColorTextureId == 0)
			{
				if (mesh.cachedBaseColor == nullptr)
				{
					string baseTextureName;
					for (auto& texture : mesh.textures)
					{
						if (texture.type == aiTextureType_BASE_COLOR)
						{
							baseTextureName = texture.src;
							break;
						}
					}
					mesh.cachedBaseColor = AssetRegistry::GetTextureFromFile(TexturesLocation + baseTextureName);
				}

				shader->SetTexture("u_texture", mesh.cachedBaseColor);
			}
			else
			{
				if (ColorTexture)
					shader->SetTexture("u_texture", ColorTexture);
				else
					shader->SetTexture("u_texture", ColorTextureId);
			}
		}

		bgfx::setVertexBuffer(0, mesh.vbh);
		bgfx::setIndexBuffer(mesh.ibh);

		BgfxStateManager::Apply();

		shader->Submit(ViewIdManager::GetCurrentId());
	}

	BgfxStateManager::SetState(startState);
}

void StaticMesh::DrawCustomId(mat4x4 view, mat4x4 projection)
{
	if (model == nullptr) return;
	if (CustomId == 0) return;

	auto startState = BgfxStateManager::GetState();

	BgfxStateManager::SetCull(TwoSided ? BgfxStateManager::Cull::None : BgfxStateManager::Cull::CW);

	const bool mask = Transparent;

	Shader* shader = ShaderManager::GetShaderProgram("vs_default", "fs_customId");

	shader->UseProgram();

	mat4x4 world = finalizedWorld;

	shader->SetUniform("view", view);
	shader->SetUniform("projection", projection);
	shader->SetUniform("viewmodelScaleFactor", ViewmodelScaleFactor);
	shader->SetUniform("world", world);
	shader->SetUniform("customId", CustomId);
	shader->SetUniform("isViewmodel", IsViewmodel);

	ApplyAdditionalShaderParams(shader);

	for (roj::SkinnedMesh& mesh : model->meshes)
	{
		bool isHiddenMesh = false;
		for (const auto& hideName : finalMeshHideList)
		{
			if (StringHelper::StartsWith(mesh.name, hideName) || hideName == mesh.name)
			{
				isHiddenMesh = true;
				break;
			}
		}
		if (isHiddenMesh)
			continue;


		if (ColorTexture == nullptr && ColorTextureId == 0)
		{
			if (mesh.cachedBaseColor == nullptr)
			{
				string baseTextureName;
				for (auto& texture : mesh.textures)
				{
					if (texture.type == aiTextureType_BASE_COLOR)
					{
						baseTextureName = texture.src;
						break;
					}
				}
				mesh.cachedBaseColor = AssetRegistry::GetTextureFromFile(TexturesLocation + baseTextureName);
			}

			shader->SetTexture("u_texture", mesh.cachedBaseColor);
		}
		else
		{
			if (ColorTexture)
				shader->SetTexture("u_texture", ColorTexture);
			else
				shader->SetTexture("u_texture", ColorTextureId);
		}


		bgfx::setVertexBuffer(0, mesh.vbh);
		bgfx::setIndexBuffer(mesh.ibh);

		BgfxStateManager::Apply();

		shader->Submit(ViewIdManager::GetCurrentId());
	}

	BgfxStateManager::SetState(startState);
}

void StaticMesh::DrawShadow(mat4x4 view, mat4x4 projection)
{
	if (model == nullptr) return;

	auto startState = BgfxStateManager::GetState();

	BgfxStateManager::SetCull(TwoSided ? BgfxStateManager::Cull::None : BgfxStateManager::Cull::CW);

	const bool mask = Transparent;

	Shader* shader = ShaderManager::GetShaderProgram("vs_default", mask ? "fs_mask" : "fs_empty");

	shader->UseProgram();

	mat4x4 world = finalizedWorld;

	shader->SetUniform("view", view);
	shader->SetUniform("projection", projection);
	shader->SetUniform("world", world);
	shader->SetUniform("isViewmodel", false);

	ApplyAdditionalShaderParams(shader);

	for (roj::SkinnedMesh& mesh : model->meshes)
	{
		bool isHiddenMesh = false;
		for (const auto& hideName : finalMeshHideList)
		{
			if (StringHelper::StartsWith(mesh.name, hideName) || hideName == mesh.name)
			{
				isHiddenMesh = true;
				break;
			}
		}
		if (isHiddenMesh)
			continue;

		if (mask)
		{
			if (ColorTexture == nullptr && ColorTextureId == 0)
			{
				if (mesh.cachedBaseColor == nullptr)
				{
					string baseTextureName;
					for (auto& texture : mesh.textures)
					{
						if (texture.type == aiTextureType_BASE_COLOR)
						{
							baseTextureName = texture.src;
							break;
						}
					}
					mesh.cachedBaseColor = AssetRegistry::GetTextureFromFile(TexturesLocation + baseTextureName);
				}

				shader->SetTexture("u_texture", mesh.cachedBaseColor);
			}
			else
			{
				if (ColorTexture)
					shader->SetTexture("u_texture", ColorTexture);
				else
					shader->SetTexture("u_texture", ColorTextureId);
			}
		}

		bgfx::setVertexBuffer(0, mesh.vbh);
		bgfx::setIndexBuffer(mesh.ibh);

		BgfxStateManager::Apply();

		shader->Submit(ViewIdManager::GetCurrentId());
	}

	BgfxStateManager::SetState(startState);
}

void StaticMesh::DrawMeshShadow(mat4x4 view, mat4x4 projection)
{
    if (model == nullptr) return;
 
	auto lightData = GetLightVolData();

	lastLightDir = lightData.direction * -1.0f;
	lastLightVolData = lightData;

    // ── Shadow colour (unchanged) ─────────────────────────────────────────────
    const vec3  ambientColor     = lastLightVolData.ambientColor;
    const vec3  directionalColor = lastLightVolData.directColor;
    constexpr float kEps = 0.001f;
 
    vec3 shadowColor;
    shadowColor.r = (directionalColor.r > kEps) ? (ambientColor.r / directionalColor.r) : 1.0f;
    shadowColor.g = (directionalColor.g > kEps) ? (ambientColor.g / directionalColor.g) : 1.0f;
    shadowColor.b = (directionalColor.b > kEps) ? (ambientColor.b / directionalColor.b) : 1.0f;
    shadowColor   = clamp(shadowColor, vec3(0.0f), vec3(1.0f));
 
    float ambientLum     = dot(ambientColor,     vec3(0.299f, 0.587f, 0.114f));
    float directionalLum = dot(directionalColor, vec3(0.299f, 0.587f, 0.114f));
    float uniformShadow  = (directionalLum > kEps)
        ? glm::clamp(ambientLum / directionalLum, 0.0f, 1.0f) : 1.0f;
 
    float blendT = 1.0f - uniformShadow;
    shadowColor  = mix(vec3(uniformShadow), shadowColor, blendT);
    vec3 d       = vec3(1.0f) - shadowColor;
    shadowColor  = vec3(1.0f) - d * d;
    shadowColor += 0.05f;
    shadowColor  = clamp(shadowColor, vec3(0.0f), vec3(1.0f));
    shadowColor  = 1.0f - shadowColor;
    shadowColor *= 1.12f;
    shadowColor  = 1.0f - shadowColor;
 
    if (shadowColor.r >= 0.999f && shadowColor.g >= 0.999f && shadowColor.b >= 0.999f)
        return;
 
    // Fake light position for directional light.
    // lastLightVolData.direction is FROM light TOWARD surface.
    // We want the fake source in the opposite direction, very far away.
    const vec3 fakeLightPos = normalize(lastLightVolData.direction) * 1e6f;
 
    const bgfx::ViewId viewId = ViewIdManager::GetCurrentId();
 
    Shader* capShader  = ShaderManager::GetShaderProgram("shadowVolume/vs_shadowvolcap",  "shadowVolume/fs_shadowvol");
    Shader* edgeShader = ShaderManager::GetShaderProgram("shadowVolume/vs_shadowvoledge", "shadowVolume/fs_shadowvol");
 
    auto setCommonUniforms = [&](Shader* sh)
    {
        sh->UseProgram();
        sh->SetUniform("world",            finalizedWorld);
        sh->SetUniform("view",             view);
        sh->SetUniform("projection",       projection);
        sh->SetUniform("u_shadowLightPos", vec4(fakeLightPos, 0.0f));
        ApplyAdditionalShaderParams(sh);
    };
 
    // ─────────────────────────────────────────────────────────────────────────
    // PASS A — Z-FAIL STENCIL
    // No colour write, no depth write, depth test read-only, no face culling.
    // Front face depth-fail → DECREMENT, back face depth-fail → INCREMENT.
    // ─────────────────────────────────────────────────────────────────────────
 
    auto savedState = BgfxStateManager::GetState();
    BgfxStateManager::SetCull(BgfxStateManager::Cull::None);
    BgfxStateManager::SetWriteRGB(false);
    BgfxStateManager::SetWriteAlpha(false);
    BgfxStateManager::SetWriteDepth(false);
    BgfxStateManager::SetDepthTest(BgfxStateManager::DepthTest::Less);
	BgfxStateManager::SetMSAA(false);
    BgfxStateManager::Apply();
 
    const uint32_t frontStencil =
        BGFX_STENCIL_TEST_ALWAYS      | BGFX_STENCIL_FUNC_REF(0) |
        BGFX_STENCIL_FUNC_RMASK(0xFF) | BGFX_STENCIL_OP_FAIL_S_KEEP |
        BGFX_STENCIL_OP_FAIL_Z_DECR   | BGFX_STENCIL_OP_PASS_Z_KEEP;
 
    const uint32_t backStencil =
        BGFX_STENCIL_TEST_ALWAYS      | BGFX_STENCIL_FUNC_REF(0) |
        BGFX_STENCIL_FUNC_RMASK(0xFF) | BGFX_STENCIL_OP_FAIL_S_KEEP |
        BGFX_STENCIL_OP_FAIL_Z_INCR   | BGFX_STENCIL_OP_PASS_Z_KEEP;
 
    for (size_t i = 0; i < model->meshes.size(); ++i)
    {
        const roj::SkinnedMesh&         mesh    = model->meshes[i];
        const roj::ShadowVolumePrecomp& precomp = mesh.shadowVolumePrecomp;
 
		bool isHiddenMesh = false;
		for (const auto& hideName : finalMeshHideList)
		{
			if (StringHelper::StartsWith(mesh.name, hideName) || hideName == mesh.name)
			{
				isHiddenMesh = true;
				break;
			}
		}
		if (isHiddenMesh)
			continue;
 
        // A1. Caps (now uses index buffer)
        if (bgfx::isValid(precomp.capVbh) && precomp.capIndexCount > 0)
        {
            setCommonUniforms(capShader);
            bgfx::setStencil(frontStencil, backStencil);
            bgfx::setVertexBuffer(0, precomp.capVbh);
            bgfx::setIndexBuffer(precomp.capIbh);
            BgfxStateManager::Apply();
            capShader->Submit(viewId);
        }
 
        // A2. Edges
        if (bgfx::isValid(precomp.edgeVbh) && precomp.edgeIndexCount > 0)
        {
            setCommonUniforms(edgeShader);
            bgfx::setStencil(frontStencil, backStencil);
            bgfx::setVertexBuffer(0, precomp.edgeVbh);
            bgfx::setIndexBuffer(precomp.edgeIbh);
            BgfxStateManager::Apply();
            edgeShader->Submit(viewId);
        }
    }
 
    // ─────────────────────────────────────────────────────────────────────────
    // PASS B — SHADOW DARKENING (multiply blend, stencil != 0)
    // ─────────────────────────────────────────────────────────────────────────
 
    BgfxStateManager::SetCull(BgfxStateManager::Cull::CW);
    BgfxStateManager::SetWriteRGB(true);
    BgfxStateManager::SetWriteAlpha(true);
    BgfxStateManager::SetBlend(BgfxStateManager::Blend::Multiply);
    BgfxStateManager::SetWriteDepth(false);
	BgfxStateManager::SetMSAA(false);// no need to msaa fullscreen effect 
    BgfxStateManager::SetDepthTest(BgfxStateManager::DepthTest::None);
    BgfxStateManager::Apply();
 
    Shader* colorShader = ShaderManager::GetShaderProgram("vs_fullscreen", "fs_fullscreen_color");
    colorShader->UseProgram();
    colorShader->SetUniform("u_color",     vec4(shadowColor, 1.0f));
  
 
    bgfx::setStencil(
        BGFX_STENCIL_TEST_NOTEQUAL    | BGFX_STENCIL_FUNC_REF(0) |
        BGFX_STENCIL_FUNC_RMASK(0xFF) | BGFX_STENCIL_OP_FAIL_S_KEEP |
        BGFX_STENCIL_OP_FAIL_Z_KEEP   | BGFX_STENCIL_OP_PASS_Z_KEEP);
    BgfxStateManager::Apply();
	Renderer::Instance->RenderFullscreenQuad(colorShader);
 
    // ─────────────────────────────────────────────────────────────────────────
    // PASS C — STENCIL CLEAR (no colour/depth write, stencil != 0 → REPLACE 0)
    // ─────────────────────────────────────────────────────────────────────────
 
    BgfxStateManager::SetWriteRGB(false);
    BgfxStateManager::SetWriteAlpha(false);
    BgfxStateManager::SetWriteDepth(false);
    BgfxStateManager::Apply();
 
	bgfx::setStencil(
		BGFX_STENCIL_TEST_NOTEQUAL | BGFX_STENCIL_FUNC_REF(0) |
		BGFX_STENCIL_FUNC_RMASK(0xFF) | BGFX_STENCIL_OP_FAIL_S_KEEP |
		BGFX_STENCIL_OP_FAIL_Z_KEEP | BGFX_STENCIL_OP_PASS_Z_REPLACE);
	BgfxStateManager::Apply();
	Renderer::Instance->RenderFullscreenQuad(colorShader);
    
 
    BgfxStateManager::SetState(savedState);
    bgfx::setStencil(BGFX_STENCIL_DEFAULT);
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