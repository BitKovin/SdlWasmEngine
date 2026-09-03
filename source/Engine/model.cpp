#include "model.hpp"
#include <filesystem>

#include "AssetRegistry.h"
#include "Texture.hpp"

#include <Profiling/ResourceStatistics.hpp>

namespace roj
{

	std::string LoaderGlobalParams::MeshNameLimit;
	float LoaderGlobalParams::Size = 1;

	template<>
	std::vector<VertexData> ModelLoader<Mesh>::getMeshVertices(aiMesh* mesh)
	{
		std::vector<VertexData> vertices;
		for (uint32_t i = 0; i < mesh->mNumVertices; i++)
		{
			VertexData vertex;
			vertex.Position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
			if (mesh->HasNormals())
			{
				vertex.Normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };
			}
			if (mesh->mTextureCoords[0])
			{
				vertex.TextureCoordinate = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
				vertex.Tangent = { mesh->mTangents[i].x,   mesh->mTangents[i].y,   mesh->mTangents[i].z };
				vertex.BiTangent = { mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z };
			}
			else
				vertex.TextureCoordinate = glm::vec2(0.0f, 0.0f);

			vertices.push_back(vertex);
		}

		return vertices;
	}

	template<>
	Mesh ModelLoader<Mesh>::processMesh(aiMesh* mesh, const aiScene* scene)
	{
		std::vector<MeshTexture> textures = getMeshTextures(scene->mMaterials[mesh->mMaterialIndex], scene);
		std::vector<VertexData> vertices = getMeshVertices(mesh);
		std::vector<uint32_t> indices;
		for (uint32_t i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace& face = mesh->mFaces[i];
			indices.insert(indices.end(), face.mIndices, face.mIndices + face.mNumIndices);
		}

		Mesh m;

		m.layout = VertexData::Declaration();

		m.vertices = vertices;
		m.indices = indices;

		const bgfx::Memory* vbMem = bgfx::makeRef(m.vertices.data(), sizeof(VertexData) * m.vertices.size());
		m.vbh = bgfx::createVertexBuffer(vbMem, m.layout);

		if (bgfx::isValid(m.vbh))
		{
			ResourceStatistics::Instance().registerResource(
				ResourceType::VertexBuffer, m.vbh.idx,
				sizeof(VertexData) * m.vertices.size(), "Mesh VB");
		}

		const bgfx::Memory* ibMem = bgfx::makeRef(m.indices.data(), sizeof(uint32_t) * m.indices.size());
		m.ibh = bgfx::createIndexBuffer(ibMem, BGFX_BUFFER_INDEX32);

		if (bgfx::isValid(m.ibh))
		{
			ResourceStatistics::Instance().registerResource(
				ResourceType::IndexBuffer, m.ibh.idx,
				sizeof(uint32_t) * m.indices.size(), "Mesh IB");
		}

		m.numIndices = static_cast<uint32_t>(indices.size());
		m.textures = textures;

		return m;
	}

	template<>
	void ModelLoader<Mesh>::processNode(aiNode* node, const aiScene* scene)
	{
		for (uint32_t i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			m_model.push_back(processMesh(mesh, scene));
		}

		for (uint32_t i = 0; i < node->mNumChildren; i++)
		{
			processNode(node->mChildren[i], scene);
		}
	}

	template<>
	bool ModelLoader<Mesh>::load(const std::string& path)
	{
		resetLoader();
		const aiScene* scene = m_import.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
		m_relativeDir = "GameData/";// static_cast<std::filesystem::path>(path).parent_path().string();

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			m_infoLog += m_import.GetErrorString();
			return false;
		}

		processNode(scene->mRootNode, scene);

		return true;
	}

	template<>
	void ModelLoader<SkinnedMesh>::LoadTextureFromScene(const aiTexture* texture)
	{
		if (texture->mHeight == 0)
		{
			// Compressed (PNG/JPEG blob in memory)
			unsigned char* texData = reinterpret_cast<unsigned char*>(texture->pcData);
			size_t texSize = texture->mWidth; // size in bytes

			// texture->achFormatHint gives you "png", "jpg" etc.
			std::string format = texture->achFormatHint;

			std::string fileName = string(texture->mFilename.C_Str()) + "." + format;

			std::string texturePath = m_lastLoadedPath + "/" + fileName;

			if (!DeferGPUUpload)
			{
				Texture* newTexture = new Texture(texData, texSize);
				ResourceStatistics::Instance().setResourceName(ResourceType::Texture, newTexture->getID(), texturePath);
				::AssetRegistry::RegisterTexture(newTexture, texturePath);
			}
			else
			{
				// Decoding is pure CPU (safe here, on the Loader thread) - the
				// actual bgfx upload, and the cache-map registration, both
				// have to happen on the main thread, so both are pushed
				// through AssetRegistry's upload queue instead of running now.
				Texture::Decoded decoded = Texture::DecodeFromMemoryCompressed(texData, (size_t)texSize, true);
				size_t approxBytes = decoded.pixels.size();
				::AssetRegistry::EnqueuePendingUpload(
					[texturePath, decoded = std::move(decoded)]() mutable
					{
						Texture* newTexture = new Texture();
						::AssetRegistry::RegisterTexture(newTexture, texturePath);
						newTexture->UploadDecoded(std::move(decoded), texturePath);
					},
					approxBytes);
			}
		}
		else
		{
			// Uncompressed raw data (BGRA32 format)
			unsigned char* texData = reinterpret_cast<unsigned char*>(texture->pcData);
			int width = texture->mWidth;
			int height = texture->mHeight;

			if (!DeferGPUUpload)
			{
				Texture* newTexture = new Texture(texData, width, height);
				ResourceStatistics::Instance().setResourceName(ResourceType::Texture, newTexture->getID(), m_lastLoadedPath);
				::AssetRegistry::RegisterTexture(newTexture, m_lastLoadedPath);
			}
			else
			{
				Texture::Decoded decoded = Texture::WrapRawPixels(texData, width, height, true);
				size_t approxBytes = decoded.pixels.size();
				std::string path = m_lastLoadedPath;
				::AssetRegistry::EnqueuePendingUpload(
					[path, decoded = std::move(decoded)]() mutable
					{
						Texture* newTexture = new Texture();
						::AssetRegistry::RegisterTexture(newTexture, path);
						newTexture->UploadDecoded(std::move(decoded), path);
					},
					approxBytes);
			}
		}
	}

	template<>
	void ModelLoader<Mesh>::LoadTextureFromScene(const aiTexture* texture)
	{
		if (texture->mHeight == 0)
		{
			// Compressed (PNG/JPEG blob in memory)
			unsigned char* texData = reinterpret_cast<unsigned char*>(texture->pcData);
			size_t texSize = texture->mWidth; // size in bytes

			// texture->achFormatHint gives you "png", "jpg" etc.
			std::string format = texture->achFormatHint;

			Texture* newTexture = new Texture(texData, texSize);

			std::string fileName = string(texture->mFilename.C_Str()) + "." + format;

			std::string texturePath = m_lastLoadedPath + "/" + fileName;

			::AssetRegistry::RegisterTexture(newTexture, texturePath);


		}
		else
		{
			// Uncompressed raw data (BGRA32 format)
			unsigned char* texData = reinterpret_cast<unsigned char*>(texture->pcData);
			int width = texture->mWidth;
			int height = texture->mHeight;

			Texture* newTexture = new Texture(texData, width, height);
			::AssetRegistry::RegisterTexture(newTexture, m_lastLoadedPath);
		}
	}


}