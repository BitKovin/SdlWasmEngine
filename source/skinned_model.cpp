#include "skinned_model.hpp"
#include <filesystem>

#include "gl.h"

#include "utils.hpp"

#include "Logger.hpp"

using namespace utils::assimp;

static void extractBoneData(std::vector<VertexData>& vertices, aiMesh* mesh, roj::SkinnedModel& model)
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
				if (vertex.BlendWeights[k] == 0.0f)
				{
					vertex.BlendWeights[k] = weights[j].mWeight;
					vertex.BlendIndices[k] = boneId;


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

	void SkinnedModel::clear()
	{
		meshes.clear();
		boneInfoMap.clear();
	}
	std::vector<SkinnedMesh>::iterator SkinnedModel::begin() { return meshes.begin(); }
	std::vector<SkinnedMesh>::iterator SkinnedModel::end() { return meshes.end(); }

	template<>
	std::vector<VertexData> ModelLoader<SkinnedMesh>::getMeshVertices(aiMesh* mesh)
	{

		std::vector<VertexData> vertices;
		for (uint32_t i = 0; i < mesh->mNumVertices; i++)
		{
			VertexData vertex;
			vertex.Position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
			vertex.Position *= LoaderGlobalParams::Size;
			if (mesh->HasNormals())
			{
				vertex.Normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };
				//vertex.SmoothNormal = vertexNormals[vertex.Position];
			}
			if (mesh->mTextureCoords[0])
			{
				vertex.TextureCoordinate = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
				vertex.Tangent = { mesh->mTangents[i].x,   mesh->mTangents[i].y,   mesh->mTangents[i].z };
				vertex.BiTangent = { mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z };
			}
			else
				vertex.TextureCoordinate = glm::vec2(0.0f, 0.0f);

			vertexPositions.push_back(vertex.Position);

			std::fill(vertex.BlendIndices, vertex.BlendIndices + MAX_BONE_INFLUENCE, 0);



			vertices.push_back(vertex);
		}

		return vertices;
	}

	std::vector<MeshTexture> GetTextures(const aiScene* scene, const aiMaterial* material)
	{

		std::vector<MeshTexture> textures;

		int n = material->GetTextureCount(aiTextureType::aiTextureType_BASE_COLOR);

		for (int i = 0; i < n; i++)
		{
			auto texture = scene->mTextures[i];
			textures.push_back(MeshTexture{ aiTextureType::aiTextureType_BASE_COLOR, texture->mFilename.C_Str()});
		}

		return textures;
		
	}

	template<>
	SkinnedMesh ModelLoader<SkinnedMesh>::processMesh(aiMesh* mesh, const aiScene* scene)
	{

		std::vector<VertexData> vertices = getMeshVertices(mesh);
		std::vector<uint32_t> indices;
		for (uint32_t i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace& face = mesh->mFaces[i];
			indices.insert(indices.end(), face.mIndices, face.mIndices + face.mNumIndices);
		}
		extractBoneData(vertices, mesh, m_model);

		SkinnedMesh skinMesh = SkinnedMesh();

		skinMesh.name = mesh->mName.C_Str();

		skinMesh.vertices = new VertexBuffer(vertices, VertexData::Declaration());

		skinMesh.indices = new IndexBuffer(indices);

		std::vector<MeshTexture> textures = getMeshTextures(scene->mMaterials[mesh->mMaterialIndex], scene);

		skinMesh.materialName = scene->mMaterials[mesh->mMaterialIndex]->GetName().C_Str();

		skinMesh.textures = textures;

		skinMesh.vertexIndices = indices;
		skinMesh.vertexLocations = vertices;


		return skinMesh;
	}

	template<>
	void ModelLoader<SkinnedMesh>::processNodeVertices(aiNode* node, const aiScene* scene)
	{

		for (uint32_t i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			for (uint32_t i = 0; i < mesh->mNumFaces; i++)
			{
				for (uint32_t i = 0; i < mesh->mNumVertices; i++)
				{
					VertexData vertex;
					vertex.Position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
					/*
					if (mesh->HasNormals())
					{
						vertex.Normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };
					}

					if (vertexNormals.contains(vertex.Position))
					{
						vertexNormals[vertex.Position] += vertex.Normal;
						vertexNormalsN[vertex.Position]++;
					}
					else
					{
						vertexNormals[vertex.Position] = vertex.Normal;
						vertexNormalsN[vertex.Position] = 1;
					}
					*/
					

				}

			}
		}

		for (uint32_t i = 0; i < node->mNumChildren; i++)
		{
			processNodeVertices(node->mChildren[i], scene);
		}

	}

	template<>
	void ModelLoader<SkinnedMesh>::processNode(aiNode* node, const aiScene* scene)
	{


		string name = node->mName.C_Str();

		
		if (LoaderGlobalParams::MeshNameLimit == "" || name == LoaderGlobalParams::MeshNameLimit)
		{
			for (uint32_t i = 0; i < node->mNumMeshes; i++)
			{
				aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
				m_model.meshes.push_back(processMesh(mesh, scene));
			}
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
		m_relativeDir = "GameData/";//static_cast<std::filesystem::path>(path).parent_path().string();

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			m_infoLog += m_import.GetErrorString();
			return false;
		}
		m_model.globalInversed = glm::inverse(toGlmMat4(scene->mRootNode->mTransformation));
		m_model.sceneCamera = (scene->HasCameras()) ? scene->mCameras[0] : nullptr;

		//processNodeVertices(scene->mRootNode, scene);

		for (auto& v : vertexNormals)
		{
			vertexNormals[v.first] = v.second / vertexNormalsN[v.first];
		}

		processNode(scene->mRootNode, scene);

		for (SkinnedMesh& mesh : m_model)
		{
			mesh.VAO = new VertexArrayObject(*mesh.vertices, *mesh.indices);
		}



		extractAnimations(scene, m_model);

		m_model.boundingSphere = BoudingSphere::FromPoints(vertexPositions);

		if (m_model.boneCount > 2)
		{
			m_model.boundingSphere.Radius *= 2.5;
			m_model.boundingSphere.offset *= 1.5f;
		}

		return true;
	}
}