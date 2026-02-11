#include "StaticMesh.h"

#include "Renderer/Renderer.h"

#include "Entity.h"

#include "Helpers/StringHelper.h"

#include "LightSystem/LightManager.h"

LightVolPointData StaticMesh::GetLightVolData()
{

	if (OwnerEntity == nullptr || 
		model == nullptr||
		Level::Current->BspData.lightVols.size() == 0) return LightVolPointData{ vec3(0),vec3(1),vec3(0) };

	vec3 samplePos = OwnerEntity->Position + vec3(0,0.0,0);

	auto light = Level::Current->BspData.GetLightvolColorPoint(samplePos * MAP_SCALE, IsViewmodel);

	//DebugDraw::Line(samplePos, samplePos + light.direction, 0.01f);
	return light;

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


		int targetC = Level::Current->BspData.FindClusterAtPosition(Position + model->boundingSphere.offset / 2.0f);

		if (Level::Current->BspData.IsClusterVisible(cameraC, targetC) == false)
		{
			return false;
		}
	}

	return IsInFrustrum(Camera::frustum) && isVisible();
}

void StaticMesh::DrawForward(mat4x4 view, mat4x4 projection)
{

	if (model == nullptr) return;

	if (Transparent == false)
	{
		if (DepthWrite)
		{
			glDepthMask(GL_TRUE);
		}
		else
		{
			glDepthMask(GL_FALSE);
		}
	}


	if (TwoSided)
	{
		glDisable(GL_CULL_FACE);
	}
	else
	{
		glEnable(GL_CULL_FACE);
	}


	if (forward_shader_program == nullptr)
		forward_shader_program = ShaderManager::GetShaderProgram("default_vertex", PixelShader);

	forward_shader_program->UseProgram();


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
				if (mesh.transparentTexture)
				{
					glDepthMask(GL_FALSE);
				}
				else
				{
					glDepthMask(GL_TRUE);
				}
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

			if (EmissiveTexture)
			{
				forward_shader_program->SetTexture("u_textureEmissive", EmissiveTexture);
			}
			else
			{
				forward_shader_program->SetTexture("u_textureEmissive", EmissiveTextureId);
			}
		}

		mesh.VAO->Bind();

		if (mesh.VAO->IsInstanced())
		{
			glDrawElementsInstanced(GL_TRIANGLES, static_cast<unsigned int>(mesh.VAO->IndexCount), GL_UNSIGNED_INT, 0, mesh.VAO->GetInstanceCount());
		}
		else if (numInstances < 0)
		{
			glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(mesh.VAO->IndexCount), GL_UNSIGNED_INT, 0);
		}

		VertexArrayObject::Unbind();

	}

	glEnable(GL_CULL_FACE);

}

void StaticMesh::DrawDepth(mat4x4 view, mat4x4 projection)
{

	if (model == nullptr) return;

	if (DepthPrePath == false) return;

	if (TwoSided)
	{
		glDisable(GL_CULL_FACE);
	}
	else
	{
		glEnable(GL_CULL_FACE);
	}

	bool mask = Transparent || Masked;

	ShaderProgram* shader_program = ShaderManager::GetShaderProgram("default_vertex", mask ? "mask_pixel" : "empty_pixel");

	shader_program->UseProgram();

	mat4x4 world = finalizedWorld;

	shader_program->SetUniform("view", view);
	shader_program->SetUniform("projection", projection);
	shader_program->SetUniform("viewmodelScaleFactor", ViewmodelScaleFactor);

	shader_program->SetUniform("world", world);

	shader_program->SetUniform("isViewmodel", IsViewmodel);

	ApplyAdditionalShaderParams(shader_program);


	for (roj::SkinnedMesh& mesh : model->meshes)
	{

		if (finalMeshHideList.contains(mesh.name)) continue;

		if (mask)
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



				if (mesh.cachedBaseColor == nullptr)
				{
					const string textureRoot = TexturesLocation;

					mesh.cachedBaseColor = AssetRegistry::GetTextureFromFile(textureRoot + baseTextureName);
				}

				Texture* texture = mesh.cachedBaseColor;

				shader_program->SetTexture("u_texture", texture);
			}
			else
			{
				shader_program->SetTexture("u_texture", ColorTexture);
			}
		}

		mesh.VAO->Bind();

		if (mesh.VAO->IsInstanced())
		{
			glDrawElementsInstanced(GL_TRIANGLES, static_cast<unsigned int>(mesh.VAO->IndexCount), GL_UNSIGNED_INT, 0, mesh.VAO->GetInstanceCount());
		}
		else if (numInstances < 0)
		{
			glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(mesh.VAO->IndexCount), GL_UNSIGNED_INT, 0);
		}

		VertexArrayObject::Unbind();

	}
}

void StaticMesh::DrawCustomId(mat4x4 view, mat4x4 projection)
{
	if (model == nullptr) return;
	if (CustomId == 0) return;

	bool mask = Transparent;

	ShaderProgram* shader_program = ShaderManager::GetShaderProgram("default_vertex", "customId");

	shader_program->UseProgram();

	mat4x4 world = finalizedWorld;

	shader_program->SetUniform("view", view);
	shader_program->SetUniform("projection", projection);
	shader_program->SetUniform("viewmodelScaleFactor", ViewmodelScaleFactor);

	shader_program->SetUniform("world", world);

	shader_program->SetUniform("customId", CustomId);

	shader_program->SetUniform("isViewmodel", IsViewmodel);

	ApplyAdditionalShaderParams(shader_program);


	for (roj::SkinnedMesh& mesh : model->meshes)
	{

		if (finalMeshHideList.contains(mesh.name)) continue;

		if (mask)
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



				if (mesh.cachedBaseColor == nullptr)
				{
					const string textureRoot = TexturesLocation;

					mesh.cachedBaseColor = AssetRegistry::GetTextureFromFile(textureRoot + baseTextureName);
				}

				Texture* texture = mesh.cachedBaseColor;

				shader_program->SetTexture("u_texture", texture);
			}
			else
			{
				shader_program->SetTexture("u_texture", ColorTexture);
			}
		}
		mesh.VAO->Bind();

		if (mesh.VAO->IsInstanced())
		{
			glDrawElementsInstanced(GL_TRIANGLES, static_cast<unsigned int>(mesh.VAO->IndexCount), GL_UNSIGNED_INT, 0, mesh.VAO->GetInstanceCount());
		}
		else if (numInstances < 0)
		{
			glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(mesh.VAO->IndexCount), GL_UNSIGNED_INT, 0);
		}

		VertexArrayObject::Unbind();

	}
}

void StaticMesh::DrawShadow(mat4x4 view, mat4x4 projection)
{
	if (model == nullptr) return;
	bool mask = Transparent;

	ShaderProgram* shader_program = ShaderManager::GetShaderProgram("default_vertex", mask ? "mask_pixel" : "empty_pixel");

	shader_program->UseProgram();

	mat4x4 world = finalizedWorld;

	shader_program->SetUniform("view", view);
	shader_program->SetUniform("projection", projection);

	shader_program->SetUniform("world", world);

	shader_program->SetUniform("isViewmodel", false);

	ApplyAdditionalShaderParams(shader_program);


	for (roj::SkinnedMesh& mesh : model->meshes)
	{

		if (finalMeshHideList.contains(mesh.name)) continue;

		if (mask)
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



				if (mesh.cachedBaseColor == nullptr)
				{
					const string& textureRoot = TexturesLocation;

					mesh.cachedBaseColor = AssetRegistry::GetTextureFromFile(textureRoot + baseTextureName);
				}

				Texture* texture = mesh.cachedBaseColor;

				shader_program->SetTexture("u_texture", texture);
			}
			else
			{
				shader_program->SetTexture("u_texture", ColorTexture);
			}
		}
		mesh.VAO->Bind();

		if (mesh.VAO->IsInstanced())
		{
			glDrawElementsInstanced(GL_TRIANGLES, static_cast<unsigned int>(mesh.VAO->IndexCount), GL_UNSIGNED_INT, 0, mesh.VAO->GetInstanceCount());
		}
		else if (numInstances < 0)
		{
			glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(mesh.VAO->IndexCount), GL_UNSIGNED_INT, 0);
		}

		VertexArrayObject::Unbind();

	}
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
