#ifndef SKINNED_MODEL_HPP
#define SKINNED_MODEL_HPP
#include "model.hpp"
#include <unordered_map>

#define MAX_BONE_INFLUENCE 4

namespace roj
{
	
	struct SkinnedMesh
	{

		VertexBuffer* vertices;
		IndexBuffer* indices;
		VertexArrayObject* VAO;

	};



	struct BoneNode
	{
		std::string name;
		glm::mat4 transform;
		mat4 finalLocalTransform;
		std::vector<BoneNode> children;
	};

	struct FrameBoneTransform {
		std::vector<float> positionTimestamps = {};
		std::vector<float> rotationTimestamps = {};
		std::vector<float> scaleTimestamps = {};

		std::vector<glm::vec3> positions = {};
		std::vector<glm::quat> rotations = {};
		std::vector<glm::vec3> scales = {};
	};

	struct Animation {
		float duration = 0.0f;
		float ticksPerSec = 1.0f;
		BoneNode rootBone;
		std::unordered_map<std::string, FrameBoneTransform> animationFrames = {};
	};


	struct BoneInfo
	{
		int id;
		glm::mat4 offset;
	};
	struct SkinnedModel
	{
		int boneCount{ 0 };
		aiCamera* sceneCamera;
		glm::mat4 globalInversed;
		std::vector<SkinnedMesh> meshes;
		std::unordered_map<std::string, BoneInfo> boneInfoMap;
		std::unordered_map<std::string, Animation> animations;
		std::vector<SkinnedMesh>::iterator begin();
		std::vector<SkinnedMesh>::iterator end();
		void clear();
	};


}
#endif //-SKINNED_MODEL_HPP

