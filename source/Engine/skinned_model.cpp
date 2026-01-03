#include "skinned_model.hpp"
#include "FileSystem/FileSystem.h"


#include "utils.hpp"

#include "Logger.hpp"

#include <unordered_set>
#include "Helpers/StringHelper.h"

#include "model_animator.hpp"

#define BAKED_FRAME_RATE 150.0f

using namespace utils::assimp;

static aiNode* findNode(const std::string& name, aiNode* node)
{
	if (node->mName.C_Str() == name) return node;
	for (unsigned i = 0; i < node->mNumChildren; i++) {
		aiNode* found = findNode(name, node->mChildren[i]);
		if (found) return found;
	}
	return nullptr;
}

static void extractBoneVertexData(std::vector<VertexData>& vertices, aiMesh* mesh, roj::SkinnedModel& model)
{
	for (unsigned int i = 0; i < mesh->mNumBones; ++i)
	{
		int boneId = -1;
		hashed_string boneName = mesh->mBones[i]->mName.C_Str();
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

static void extractBoneNodeAndBuildMap(roj::SkinnedModel& model, roj::BoneNode& bone, aiNode* src)
{
	bone.name = src->mName.data;

	bone.transform = toGlmMat4(src->mTransformation);


	for (unsigned int i = 0; i < src->mNumChildren; i++)
	{
		roj::BoneNode node;
		extractBoneNodeAndBuildMap(model, node, src->mChildren[i]);
		bone.children.push_back(node);
		model.parentMap[node.name] = bone.name;
	}

	model.boneNodesMap[bone.name] = bone;

}

static void extractAnimations(const aiScene* scene, roj::SkinnedModel& model)
{
	std::unordered_set<hashed_string> additionalBones;

	for (unsigned int i = 0; i < scene->mNumAnimations; i++)
	{
		aiAnimation* sceneAnim = scene->mAnimations[i];
		roj::Animation& animation = model.animations[sceneAnim->mName.C_Str()];
		extractBoneNode(animation.rootBone, scene->mRootNode);
		animation.ticksPerSec = sceneAnim->mTicksPerSecond;
		animation.duration = sceneAnim->mDuration;

		float durationSeconds = animation.duration / animation.ticksPerSec;

		// Count frames (sum of all keyframes in channel[0])
		int keyCount = sceneAnim->mChannels[0]->mNumPositionKeys
			+ sceneAnim->mChannels[0]->mNumRotationKeys
			+ sceneAnim->mChannels[0]->mNumScalingKeys;

		animation.frameTime = keyCount / durationSeconds;

		for (unsigned int j = 0; j < sceneAnim->mNumChannels; j++) {
			aiNodeAnim* channel = sceneAnim->mChannels[j];
			hashed_string boneName = channel->mNodeName.C_Str();
			if (model.boneInfoMap.find(boneName) == model.boneInfoMap.end()) {
				additionalBones.insert(boneName);
			}
			roj::FrameBoneTransform& track = animation.animationFrames[channel->mNodeName.C_Str()];

			for (int k = 0; k < channel->mNumPositionKeys; k++) {
				track.positionTimestamps.emplace_back(channel->mPositionKeys[k].mTime);
				track.positions.push_back(toGlmVec3(channel->mPositionKeys[k].mValue));
			}

			for (int k = 0; k < channel->mNumRotationKeys; k++) {
				track.rotationTimestamps.emplace_back(channel->mRotationKeys[k].mTime);
				track.rotations.push_back(toGlmQuat(channel->mRotationKeys[k].mValue));
			}

			for (int k = 0; k < channel->mNumScalingKeys; k++) {
				track.scaleTimestamps.emplace_back(channel->mScalingKeys[k].mTime);
				track.scales.push_back(toGlmVec3(channel->mScalingKeys[k].mValue));
			}


		}
	}

	// Add missing bones from animation channels
	for (const auto& boneName : additionalBones) {
		std::string nameStr = boneName.str();  // Assuming hashed_string has c_str() method
		aiNode* node = findNode(nameStr, scene->mRootNode);
		if (!node) continue;

		glm::mat4 globalTransform = glm::mat4(1.0f);
		aiNode* current = node;
		while (current) {
			globalTransform = toGlmMat4(current->mTransformation) * globalTransform;
			current = current->mParent;
		}

		glm::mat4 offset = glm::inverse(globalTransform);
		model.boneInfoMap[boneName] = roj::BoneInfo{ model.boneCount, offset };
		model.boneCount++;
	}
}


namespace roj
{



	template class ModelLoader<SkinnedMesh>;

	void SkinnedModel::clear()
	{

		for (auto mesh : meshes)
		{
			mesh.DestroyBuffers();
		}

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

			}
			else
				vertex.TextureCoordinate = glm::vec2(0.0f, 0.0f);

			vertex.Tangent = { mesh->mTangents[i].x,   mesh->mTangents[i].y,   mesh->mTangents[i].z };
			vertex.BiTangent = { mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z };
			int colorChannel = -1;
			for (unsigned c = 0; c < mesh->GetNumColorChannels(); c++)
			{
				if (mesh->mColors[c])
				{
					colorChannel = c;
					break;
				}
			}

			if (colorChannel >= 0)
			{
				vertex.Color = {
					mesh->mColors[colorChannel][i].r,
					mesh->mColors[colorChannel][i].g,
					mesh->mColors[colorChannel][i].b,
					mesh->mColors[colorChannel][i].a
				};
			}

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
			textures.push_back(MeshTexture{ aiTextureType::aiTextureType_BASE_COLOR, texture->mFilename.C_Str() });
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
		extractBoneVertexData(vertices, mesh, m_model);

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

		if (SkipVisual == false)
		{
			if (LoaderGlobalParams::MeshNameLimit == "" || name == LoaderGlobalParams::MeshNameLimit)
			{
				for (uint32_t i = 0; i < node->mNumMeshes; i++)
				{
					aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
					m_model.meshes.push_back(processMesh(mesh, scene));
				}
			}
		}
		else
		{

			if (LoaderGlobalParams::MeshNameLimit == "" || name == LoaderGlobalParams::MeshNameLimit)
			{
				for (uint32_t i = 0; i < node->mNumMeshes; i++)
				{
					aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
					for (unsigned int i = 0; i < mesh->mNumBones; ++i)
					{
						std::string boneName = mesh->mBones[i]->mName.C_Str();
						if (m_model.boneInfoMap.find(boneName) == m_model.boneInfoMap.end())
						{
							m_model.boneInfoMap[boneName] = roj::BoneInfo{ m_model.boneCount, toGlmMat4(mesh->mBones[i]->mOffsetMatrix) };
						}

					}
				}
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

		if (path.empty())
		{
			m_infoLog += "Skinned model loader: Empty path provided.\n";
			return false;
		}

		Assimp::Importer importer;

		const aiScene* m_scene = nullptr;

		// Check if the requested scene is the same as the cached one
		if (path == m_lastLoadedPath && m_cachedScene != nullptr) {
			// Reuse the cached scene
			m_scene = m_cachedScene;

		}
		else
		{

			std::vector<uint8_t> fileData = FileSystemEngine::ReadFileBinary(path);

			if (fileData.empty())
			{
				m_infoLog += "Skinned model loader: Failed to read file: " + path + "\n";
				return false;
			}

			m_scene = m_import.ReadFileFromMemory(fileData.data(), fileData.size(),
				aiProcess_Triangulate |
				aiProcess_GenSmoothNormals |
				aiProcess_FlipUVs |
				aiProcess_CalcTangentSpace |
				aiProcess_LimitBoneWeights |
				aiProcess_JoinIdenticalVertices |
				aiProcess_GlobalScale,
				path.c_str());
			m_cachedScene = m_scene;
			m_lastLoadedPath = path;
		}

		m_relativeDir = "GameData/";//static_cast<std::filesystem::path>(path).parent_path().string();

		if (!m_scene || m_scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !m_scene->mRootNode)
		{
			m_infoLog += m_import.GetErrorString();
			return false;
		}
		m_model.globalInversed = glm::inverse(toGlmMat4(m_scene->mRootNode->mTransformation));
		m_model.sceneCamera = (m_scene->HasCameras()) ? m_scene->mCameras[0] : nullptr;

		//processNodeVertices(scene->mRootNode, scene);

		for (auto& v : vertexNormals)
		{
			vertexNormals[v.first] = v.second / vertexNormalsN[v.first];
		}

		processNode(m_scene->mRootNode, m_scene);

		for (SkinnedMesh& mesh : m_model)
		{
			mesh.VAO = new VertexArrayObject(*mesh.vertices, *mesh.indices);
		}



		extractAnimations(m_scene, m_model);

		m_model.boundingSphere = BoundingSphere::FromPoints(vertexPositions);
		m_model.boundingBox = BoundingBox::FromPoints(vertexPositions);

		if (m_model.boneCount > 0)
		{
			m_model.boundingSphere.Radius *= 2.5;
			m_model.boundingSphere.offset *= 1.5f;

			extractBoneNodeAndBuildMap(m_model, m_model.defaultRoot, m_scene->mRootNode);

		}

		for (auto& mesh : m_model.meshes)
		{
			mesh.ProcessDefaultTextures();
		}

		std::sort(m_model.meshes.begin(), m_model.meshes.end(),
			[](SkinnedMesh& a, SkinnedMesh& b) {
				return b.transparentTexture;
			});


		roj::Animator animator(&m_model);

		
		for (auto& anim : m_model.animations)
		{

			animator.set(anim.first);
			animator.play();

			float animationStep = 1.0f / BAKED_FRAME_RATE;

			animator.update(0.0f); // Initial pose

			anim.second.bakedFrameInterval = animationStep;

			BakedFrameData bakedFrameData;


			bakedFrameData.totalRootMotionPosition = animator.totalRootMotionPosition;
			bakedFrameData.totalRootMotionRotation = animator.totalRootMotionRotation;
			bakedFrameData.modelTransform = animator.getBoneMatrices();
			bakedFrameData.boneTransforms = animator.currentPose;

			anim.second.bakedFrames.push_back(bakedFrameData);

			while (animator.m_playing)
			{
				animator.update(animationStep);

				bakedFrameData.totalRootMotionPosition = animator.totalRootMotionPosition;
				bakedFrameData.totalRootMotionRotation = animator.totalRootMotionRotation;
				bakedFrameData.modelTransform = animator.getBoneMatrices();
				bakedFrameData.boneTransforms = animator.currentPose;

				anim.second.bakedFrames.push_back(bakedFrameData);

			}

			animator.reset();

		}


		return true;
	}
	void SkinnedMesh::ProcessDefaultTextures()
	{

		for (auto texture : textures)
		{
			if (texture.type == aiTextureType_BASE_COLOR)
			{
				std::string baseTextureName = texture.src;

				if (StringHelper::Contains(baseTextureName, "_t.") || StringHelper::Contains(baseTextureName, "_m."))
				{
					transparentTexture = true;
				}
				break;
			}
		}
	}
}