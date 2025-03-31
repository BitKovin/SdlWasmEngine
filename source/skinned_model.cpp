#include "skinned_model.hpp"
#include <filesystem>

#include "gl.h"

#include "utils.hpp"

using namespace utils::assimp;

static void extractBoneData(std::vector<roj::SkinnedMesh::Vertex>& vertices, aiMesh* mesh, roj::SkinnedModel& model)
{
	for (unsigned int i = 0; i < mesh->mNumBones; ++i)
	{
		int boneId = -1;
		std::string boneName = mesh->mBones[i]->mName.C_Str();
		if (model.boneInfoMap.find(boneName) == model.boneInfoMap.end())
		{
			model.boneInfoMap[boneName] = roj::BoneInfo{ model.boneCount, toGlmMat4(mesh->mBones[i]->mOffsetMatrix) };
			boneId = model.boneCount++;
		}
		else
		{
			boneId = model.boneInfoMap[boneName].id;
		}

		auto weights = mesh->mBones[i]->mWeights;
		for (int j = 0; j < mesh->mBones[i]->mNumWeights; ++j)
		{
			auto& vertex = vertices[weights[j].mVertexId];
			for (int k = 0; k < MAX_BONE_INFLUENCE; ++k)
			{
				if (vertex.weights[k] == 0.0f)
				{
					vertex.weights[k] = weights[j].mWeight;
					vertex.boneIds[k] = boneId;
					break;
				}
			}
		}
	}
}
static void extractBoneNode(roj::BoneNode& bone, aiNode* src)
{
	bone.name = src->mName.data;

	bone.transform = toGlmMat4(src->mTransformation);


	for (unsigned int i = 0; i < src->mNumChildren; i++)
	{
		roj::BoneNode node;
		extractBoneNode(node, src->mChildren[i]);
		bone.children.push_back(node);
	}
}

static void extractAnimations(const aiScene* scene, roj::SkinnedModel& model)
{
	for (unsigned int i = 0; i < scene->mNumAnimations; i++)
	{
		aiAnimation* sceneAnim = scene->mAnimations[i];
		roj::Animation& animation = model.animations[sceneAnim->mName.C_Str()];
		extractBoneNode(animation.rootBone, scene->mRootNode);
		animation.ticksPerSec = sceneAnim->mTicksPerSecond;
		animation.duration = sceneAnim->mDuration;

		for (unsigned int i = 0; i < sceneAnim->mNumChannels; i++) {
			aiNodeAnim* channel = sceneAnim->mChannels[i];
			roj::FrameBoneTransform& track = animation.animationFrames[channel->mNodeName.C_Str()];

			for (int j = 0; j < channel->mNumPositionKeys; j++) {
				track.positionTimestamps.emplace_back(channel->mPositionKeys[j].mTime);
				track.positions.push_back(toGlmVec3(channel->mPositionKeys[j].mValue));
			}

			for (int j = 0; j < channel->mNumRotationKeys; j++) {
				track.rotationTimestamps.emplace_back(channel->mRotationKeys[j].mTime);
				track.rotations.push_back(toGlmQuat(channel->mRotationKeys[j].mValue));
			}

			for (int j = 0; j < channel->mNumScalingKeys; j++) {
				track.scaleTimestamps.emplace_back(channel->mScalingKeys[j].mTime);
				track.scales.push_back(toGlmVec3(channel->mScalingKeys[j].mValue));
			}


		}
	}
}

namespace roj
{

	template class ModelLoader<SkinnedMesh>;

	void SkinnedMesh::setup()
	{
		unsigned int VBO, EBO;
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));

		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));

		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, bitangent));

		glEnableVertexAttribArray(5);
		glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, boneIds));

		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, weights));
		glBindVertexArray(0);
	}
	void SkinnedModel::clear()
	{
		meshes.clear();
		boneInfoMap.clear();
	}
	std::vector<SkinnedMesh>::iterator SkinnedModel::begin() { return meshes.begin(); }
	std::vector<SkinnedMesh>::iterator SkinnedModel::end() { return meshes.end(); }

	template<>
	std::vector<SkinnedMesh::Vertex> ModelLoader<SkinnedMesh>::getMeshVertices(aiMesh* mesh)
	{

		std::vector<SkinnedMesh::Vertex> vertices;
		for (uint32_t i = 0; i < mesh->mNumVertices; i++)
		{
			SkinnedMesh::Vertex vertex;
			vertex.position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
			if (mesh->HasNormals())
			{
				vertex.normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };
			}
			if (mesh->mTextureCoords[0])
			{
				vertex.texCoords = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
				vertex.tangent = { mesh->mTangents[i].x,   mesh->mTangents[i].y,   mesh->mTangents[i].z };
				vertex.bitangent = { mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z };
			}
			else
				vertex.texCoords = glm::vec2(0.0f, 0.0f);
			std::fill(vertex.boneIds, vertex.boneIds + MAX_BONE_INFLUENCE, 0);
			std::fill(vertex.weights, vertex.weights + MAX_BONE_INFLUENCE, 0.0f);
			vertices.push_back(vertex);
		}

		return vertices;
	}

	template<>
	SkinnedMesh ModelLoader<SkinnedMesh>::processMesh(aiMesh* mesh, const aiScene* scene)
	{
		std::vector<MeshTexture> textures = getMeshTextures(scene->mMaterials[mesh->mMaterialIndex]);
		std::vector<SkinnedMesh::Vertex> vertices = getMeshVertices(mesh);
		std::vector<uint32_t> indices;
		for (uint32_t i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace& face = mesh->mFaces[i];
			indices.insert(indices.end(), face.mIndices, face.mIndices + face.mNumIndices);
		}
		extractBoneData(vertices, mesh, m_model);

		SkinnedMesh skinMesh;

		skinMesh.vertices = vertices;
		skinMesh.indices = indices;
		skinMesh.textures = textures;

		return skinMesh;
	}

	template<>
	void ModelLoader<SkinnedMesh>::processNode(aiNode* node, const aiScene* scene)
	{
		for (uint32_t i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			m_model.meshes.push_back(processMesh(mesh, scene));
		}
		for (uint32_t i = 0; i < node->mNumChildren; i++)
		{
			processNode(node->mChildren[i], scene);
		}
	}

	template<>
	bool ModelLoader<SkinnedMesh>::load(const std::string& path)
	{
		resetLoader();

		Assimp::Importer importer;

		const aiScene* scene = m_import.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
		m_relativeDir = "GameData/";// static_cast<std::filesystem::path>(path).parent_path().string();

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			m_infoLog += m_import.GetErrorString();
			return false;
		}
		m_model.globalInversed = glm::inverse(toGlmMat4(scene->mRootNode->mTransformation));
		m_model.sceneCamera = (scene->HasCameras()) ? scene->mCameras[0] : nullptr;
		processNode(scene->mRootNode, scene);
		for (SkinnedMesh& mesh : m_model)
		{
			mesh.setup();
		}

		extractAnimations(scene, m_model);
		return true;
	}
}