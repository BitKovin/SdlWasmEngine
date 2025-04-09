#ifndef MODEL_HPP
#define MODEL_HPP
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "glm.h"

#include <unordered_map>
#include <vector>

#include "utils.hpp"

#include <string>

#include "VertexData.h"

namespace roj
{
	struct MeshTexture
	{
		aiTextureType type; std::string src;
	};

	struct Mesh
	{

		VertexBuffer* vertexBuffer;
		IndexBuffer* indexBuffer;

		std::vector<MeshTexture> textures;
		VertexArrayObject* VAO;

	};

	template<typename mesh_t>
	struct ModelType;

	struct SkinnedMesh; struct SkinnedModel;
	template<>
	struct ModelType<SkinnedMesh> { using type = SkinnedModel; };

	template<>
	struct ModelType<Mesh> { using type = std::vector<Mesh>; };

	template<typename mesh_t = Mesh>
	class ModelLoader
	{
	public:
		using model_t = typename ModelType<mesh_t>::type;
	private:
		Assimp::Importer m_import;
		model_t m_model;
		std::vector<MeshTexture> m_texCache;
		std::string m_infoLog;
		std::string m_relativeDir;

		std::vector<glm::vec3> vertexPositions;
		std::unordered_map<glm::vec3, glm::vec3> vertexNormals; // position and normal
		std::unordered_map<glm::vec3, float> vertexNormalsN; // position and normal num

	private:
		void resetLoader();
		void processNode(aiNode* node, const aiScene* scene);
		void processNodeVertices(aiNode* node, const aiScene* scene);
		mesh_t processMesh(aiMesh* mesh, const aiScene* scene);
		std::vector<VertexData>  getMeshVertices(aiMesh* mesh);
		std::vector<MeshTexture> getMeshTextures(aiMaterial* material, const aiScene* scene);
		std::vector<MeshTexture> loadTextureMap(aiMaterial* mat, aiTextureType type, const aiScene* scene);
	public:
		ModelLoader() = default;
		bool load(const std::string& path);
		model_t& get();
		const std::string& getInfoLog();
	};

	template<typename mesh_t>
	typename ModelLoader<mesh_t>::model_t& ModelLoader<mesh_t>::get()
	{
		return m_model;
	}

	template<typename mesh_t>
	const std::string& ModelLoader<mesh_t>::getInfoLog()
	{
		return m_infoLog;
	}

	template<typename mesh_t>
	std::vector<MeshTexture> ModelLoader<mesh_t>::getMeshTextures(aiMaterial* material, const aiScene* scene)
	{
		std::vector<MeshTexture> textures;

		std::vector<MeshTexture> diffuseMaps = loadTextureMap(material, aiTextureType_BASE_COLOR, scene);
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

		std::vector<MeshTexture> specularMaps = loadTextureMap(material, aiTextureType_SPECULAR, scene);
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

		std::vector<MeshTexture> normalMaps = loadTextureMap(material, aiTextureType_NORMALS, scene);
		textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

		std::vector<MeshTexture> heightMaps = loadTextureMap(material, aiTextureType_HEIGHT, scene);
		textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
		return textures;
	}

	template<typename mesh_t>
	std::vector<roj::MeshTexture> ModelLoader<mesh_t>::loadTextureMap(aiMaterial* mat, aiTextureType type, const aiScene* scene)
	{
		std::vector<MeshTexture> textures;

		for (uint32_t i = 0; i < mat->GetTextureCount(type); i++)
		{
			aiString texSrc;
			mat->GetTexture(type, i, &texSrc);

			std::string str = texSrc.C_Str();

			str = str.substr(1, str.size() - 1);

			int id = std::stoi(str);

			

			std::string fileName = scene->mTextures[id]->mFilename.C_Str();
			std::string fileExtension = scene->mTextures[id]->achFormatHint;

			textures.emplace_back(type, fileName + "." + fileExtension);
		}

		return textures;
	}

	template<typename mesh_t>
	void ModelLoader<mesh_t>::resetLoader()
	{
		m_texCache.clear();
		m_model.clear();
		m_infoLog.clear();
		m_relativeDir.clear();

		vertexPositions.clear();
		vertexNormals.clear();
		vertexNormalsN.clear();

	}
}

#endif //-MODEL_HPP