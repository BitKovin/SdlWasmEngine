#pragma once


#include <vector>
#include <string>

#include "ShaderManager.h"

#include "IDrawMesh.h"

#include "VertexData.h"

#include "MathHelper.hpp"

#include "skinned_model.hpp"

#include "glm.h"

#include "model_animator.hpp"

#include "StaticMesh.h"

#include "Time.hpp"

#include "json.hpp"
#include "Helpers/JsonHelper.hpp"

#include "Physics.h"



using namespace std;

struct AnimationPose 
{
	std::unordered_map<hashed_string, mat4> boneTransforms;

	roj::BoneNode* boneTreeRoot = nullptr;

	static AnimationPose Lerp(const AnimationPose& a,
		const AnimationPose& b, float progress);

	static AnimationPose LayeredLerp(const hashed_string& startBoneName,
		roj::BoneNode* rootNode,
		const AnimationPose& poseA,
		const AnimationPose& poseB,
		float UseWorldSpaceRotation,
		float progress);

	static AnimationPose ApplyFABRIK(
		const hashed_string& chainRootName,
		const hashed_string& endEffectorName,
		roj::BoneNode* rootNode,
		const AnimationPose& inputPose,
		const glm::vec3& targetPosition, // model space position
		const glm::quat& targetRotation, // model space rotation
		bool snapRotation = true,
		int maxIterations = 15,
		float tolerance = 0.001f);

	static glm::mat4 GetModelSpaceTransform(
		const hashed_string& boneName,
		roj::BoneNode* rootNode,
		const AnimationPose& pose);

	void SetBoneTransform(hashed_string bone, mat4 transform)
	{
		boneTransforms[bone] = transform;
	}

	void SetBoneTransform(hashed_string bone, MathHelper::Transform transform)
	{
		boneTransforms[bone] = transform.ToMatrix();
	}

	void SetBoneTransformEuler(hashed_string bone, MathHelper::Transform transform)
	{
		boneTransforms[bone] = transform.ToMatrixEuler();
	}

	MathHelper::Transform GetBoneTransform(std::string bone)
	{
		auto res = boneTransforms.find(bone);
		if (res != boneTransforms.end())
		{
			return MathHelper::DecomposeMatrix(res->second);
		}

		return MathHelper::Transform();
	}


	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(AnimationPose, boneTransforms)

};

struct AnimationState
{
	bool playing = false;
	string animationName = "";
	float animationTime = 0;
	float oldAnimationEventTime = 0;
	bool looping = false;


	bool inRagdoll = false;
	std::unordered_map<string, vec3> ragdollHitboxPositions = {};
	std::unordered_map<string, quat> ragdollHitboxRotations = {};

	std::unordered_map<string, vec3> ragdollHitboxLinearVelocty = {};
	std::unordered_map<string, vec3> ragdollHitboxAngularVelocty = {};

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(AnimationState, playing, animationName, animationTime, oldAnimationEventTime,looping, 
		inRagdoll, ragdollHitboxPositions, ragdollHitboxRotations, ragdollHitboxLinearVelocty, ragdollHitboxAngularVelocty)

};

struct AnimationEvent
{
	string eventName;
	float time;
	string userData1;
	string userData2;
	string userData3;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(AnimationEvent, eventName, time, userData1, userData2, userData3)
};

struct AnimationData
{
	string animationName;
	vector<AnimationEvent> animationEvents;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(AnimationData, animationName, animationEvents)
};

struct HitboxData
{
	string boneName = "";
	vec3 size = vec3(0.2);
	vec3 rotation = vec3(0);
	vec3 position = vec3(0);
	float damageMultiplier = 1;

	string parentBone = "";

	vec3 twistParameters = vec3();
	vec3 constraintRotation = vec3();

	float mass = 5;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(HitboxData, boneName, size, rotation, position, damageMultiplier, parentBone, twistParameters, constraintRotation, mass)
};

struct SkeletalMeshMetaData
{
	vector<HitboxData> hitboxes;
	vector<AnimationData> animations;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(SkeletalMeshMetaData, hitboxes, animations)


};

class Entity;

class SkeletalMesh : public StaticMesh
{
private:

	roj::Animator animator;

	std::vector<mat4> boneTransforms;

	std::vector<mat4> finalizedBoneTransforms;

	std::unordered_map<string, vec3> boneLinearVel;
	std::unordered_map<string, vec3> boneAngularVel;

	double blendStartTime = 0;
	double blendEndTime = 0;

	bool firstAnimation = true;

	vec3 oldRootMotionPos = vec3();
	vec3 oldRootMotionRot = vec3();

	float oldAnimTime = 0;

	string filePath = "";

	quat rootMotionBasisQuat;

	float oldAnimationEventTime = 0;

	
	AnimationPose blendStartPose;


	std::unordered_map<hashed_string, TwoBodyConstraint*> hitboxConstraints;
	std::unordered_map<hashed_string, vec3> defaultBoneScale;

	std::recursive_mutex hitboxMutex;

	std::vector<AnimationEvent> pendingAnimationEvents;

protected:

	void ApplyAdditionalShaderParams(ShaderProgram* shader_program)
	{
		shader_program->SetUniform("finalBonesMatrices", finalizedBoneTransforms);
	}

	static inline unordered_map<string, SkeletalMeshMetaData> loaded_metas;
	static inline set<string> invalid_meta_files;

	bool dirtyPose = true;

	AnimationPose lastPose;

public:

	SkeletalMeshMetaData metaData;
	AnimationData* currentAnimationData = nullptr;

	bool UpdatePose = true;
	bool UpdatePoseOnlyWhenRendered = false;

	bool InRagdoll = false;
	float RagdollPoseFollowStrength = 0;

	bool UpdateRagdollPose = false;

	bool UsePrecomputedFrames = false;

	static inline int skelMeshes = 0;

	std::vector<Body*> hitboxBodies;

	SkeletalMesh(Entity* owner) : StaticMesh(owner)
	{
		CastDetailShadows = true;
		skelMeshes++;
	}

	~SkeletalMesh()
	{
		ClearHitboxes();
		skelMeshes--;
		
	}

	AnimationPose GetAnimationPose();

	void PasteAnimationPose(AnimationPose pose)
	{

		if (model == nullptr)
		{

			cerr << "model is null" << endl;

			return;
		}

		dirtyPose = true;

		animator.ApplyBonePoseArray(pose.boneTransforms);
		boneTransforms = animator.getBoneMatrices();
	}

	void ApplyWorldSpaceBoneTransforms(std::unordered_map<hashed_string, mat4>& pose);

	void PlayAnimation(std::string name, bool Loop = false, float interpIn = 0.12);
	void StopAnimation();

	std::string GetAnimationName();

	void SetAnimationPaused(bool value);

	bool GetAnimationPaused();

	bool IsCameraVisible();

	BoundingBox GetBoundingBox();

	bool IsInFrustrum(Frustum frustrum);

	LightVolPointData GetLightVolData();

	void FinalizeFrameData()
	{
		StaticMesh::FinalizeFrameData();
		finalizedBoneTransforms = boneTransforms;
	}

	MathHelper::Transform PullRootMotion();

	float GetBlendInProgress()
	{
		double currentTime = Time::GameTime;

		if (blendEndTime <= blendStartTime)
			return 1.0f; // Avoid division by zero or invalid range

		if (currentTime <= blendStartTime)
			return 0.0f;

		if (currentTime >= blendEndTime)
			return 1.0f;

		double progress = (currentTime - blendStartTime) / (blendEndTime - blendStartTime);
		return static_cast<float>(progress);
	}

	void PlayAnimation(float interpIn = 0.12)
	{

		if (model == nullptr) return;

		if (firstAnimation || animator.m_currAnim == nullptr)
		{
			interpIn = 0;
			blendStartTime = -1;
			blendEndTime = 0;
			firstAnimation = false;
		}

		if (interpIn > 0.01)
		{
			blendStartPose = GetAnimationPose();
			blendStartTime = Time::GameTime;
			blendEndTime = Time::GameTime + interpIn;
		}
		else
		{
			blendStartPose = GetAnimationPose();
			blendStartTime = Time::GameTime;
			blendEndTime = Time::GameTime - 1;
		}

		if (animator.m_currAnim == nullptr)
		{

			if (model->animations.size() > 0)
			{
				hashed_string name = model->animations.begin()->first;

				animator.set(name);
			}

		}

		animator.play();
	}

	bool IsAnimationPlaying();

	void Update(float timeScale = 1);

	void SetLooped(bool looped)
	{
		animator.Loop = looped;
	}

	bool GetLooped()
	{
		return animator.Loop;
	}

	float GetAnimationDuration();
	float GetAnimationTime();
	void SetAnimationTime(float time);

	roj::BoneNode* GetNodeFromName(const hashed_string& name);
	roj::BoneNode* GetRootNode();
	std::unordered_map<hashed_string, hashed_string>* GetNodeParentMap();

	void StartedRendering();

	void LoadFromFile(string path)
	{
		
		if (TexturesLocation == "")
		{
			TexturesLocation = path + "/";
		}

		StaticMesh::LoadFromFile(path);
		filePath = path;

		boneTransforms.resize(model->boneInfoMap.size());

		for (int i = 0; i < boneTransforms.size(); i++)
		{
			boneTransforms[i] = glm::identity<mat4>();
		}

		animator = roj::Animator(model);

		LoadMetaFromFile();
	}

	mat4 GetWorldMatrixNoOffsets();

	mat4 GetBoneMatrix(string boneName);

	mat4 GetBoneMatrixWorld(string boneName);

	float GetHitboxDamageMultiplier(string boneName);

	void StartRagdoll();
	void StopRagdoll();

	void ClearHitboxes();

	void CreateHitbox(Entity* owner, HitboxData data);

	void CreateHitboxes(Entity* owner);

	void ApplyImpulseToAllHitboxes(vec3 impulse, bool scaleWithMass = false);

	Constraint* GetConstraintByHitboxName(string name);

	Body* FindHitboxByName(string name);

	void UpdateHitboxes();

	void UpdateAnimationEvents();
	vector<AnimationEvent> PullAnimationEvents();

	void SaveMetaToFile();
	void LoadMetaFromFile();
	void LoadMetaFromFile(const std::string& path);

	static void ClearMetaDataCache();

	AnimationData* GetAnimationDataFromName(std::string name);

	void SetAnimationState(const AnimationState& animationState);



	AnimationState GetAnimationState();



};