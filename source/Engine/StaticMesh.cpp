#include "StaticMesh.h"

#include "Renderer/Renderer.h"

#include "Entity.h"

#include "Helpers/StringHelper.h"

#include "LightSystem/LightManager.h"

#include <BgfxStateManager.h>
#include <Renderer/Abstractions/ViewIdManager.h>

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

}

bool StaticMesh::IsInFrustrum(Frustum frustrum)
{
	if (model == nullptr) return false;

	auto sphere = model->boundingSphere.Transform(Position, Rotation, Scale);

	//DebugDraw::Bounds(sphere.offset - vec3(sphere.Radius), sphere.offset + vec3(sphere.Radius), 0.01f);

	return frustrum.IsSphereVisible(sphere.offset, sphere.Radius);
}

BoundingBox StaticMesh::GetBoundingBox()
{

	return model->boundingBox.Transform(finalizedWorld);

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

	if (Transparent == false)
	{

		BgfxStateManager::SetWriteDepth(DepthWrite);

	}


	BgfxStateManager::SetCull(TwoSided ? BgfxStateManager::Cull::None : BgfxStateManager::Cull::CW);


	if (forward_shader_program == nullptr)
		forward_shader_program = ShaderManager::GetShaderProgram("vs_default", PixelShader);

	forward_shader_program->UseProgram();

	bool flipedNormals = finalizedScale.x * finalizedScale.y * finalizedScale.z < 0;

	forward_shader_program->SetUniform("masked", Masked);

	Renderer::SetSurfaceShaderUniforms(forward_shader_program, Brightness);

	mat4x4 world = finalizedWorld;

	auto lightData = GetLightVolData();

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
	forward_shader_program->SetUniform("flipedNormals", flipedNormals);

	ApplyAdditionalShaderParams(forward_shader_program);


	auto bounds = GetBoundingBox();

	LightManager::ApplyPointLightToShader(forward_shader_program, bounds.Min, bounds.Max);

	for (roj::SkinnedMesh& mesh : model->meshes)
	{

		if (finalMeshHideList.contains(mesh.name)) continue;

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

			if (Transparent)
			{

				BgfxStateManager::SetWriteDepth(mesh.transparentTexture == false);

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
		if (finalMeshHideList.contains(mesh.name)) continue;

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
		if (finalMeshHideList.contains(mesh.name)) continue;

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
		if (finalMeshHideList.contains(mesh.name)) continue;

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
