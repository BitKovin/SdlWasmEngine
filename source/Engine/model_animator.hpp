#pragma once
#include "skinned_model.hpp"
#include "MathHelper.hpp"
#include <unordered_map>
#include <string>

namespace roj
{
class Animator
{

public : 

    bool Loop = false;

    float m_currTime{ 0.0f };

    bool m_playing = false;

    bool InterpolateRotation = true;
	bool InterpolatePosition = true;
	bool InterpolateScale = true;

    MathHelper::Transform oldRootBoneTransform;
    MathHelper::Transform rootBoneTransform;

    vec3 totalRootMotionPosition = vec3();
    vec3 totalRootMotionRotation = vec3();

	bool UsePrecomputedFrames = false;

private:

    std::vector<glm::mat4> m_boneMatrices;
    
    SkinnedModel* m_model = nullptr;

    std::vector<roj::FrameBoneTransform> cachedFrameBoneTransforms; // Cached animation frame data
    std::vector<roj::BoneInfo> cachedBoneInfos;                    // Cached bone info data

    void precacheAnimation();

    bool m_loopEnabled = false;
private:
    int getKeyTransformIdx(std::vector<float>& timestamps);
    float getScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime);
    glm::mat4 interpolatePosition(FrameBoneTransform& boneTransform);
    glm::mat4 interpolateRotation(FrameBoneTransform& boneTransform);
    glm::mat4 interpolateScaling(FrameBoneTransform& boneTransform);
    void calcBoneTransform(BoneNode& node, glm::mat4 offset, bool stopAfterRoot);
	void useBakedFrame(float time);

    void ApplyNodePose(BoneNode& node, glm::mat4 offset, std::unordered_map<hashed_string, mat4>& pose);

    void ApplyNodePoseLocalSpace(BoneNode& node, glm::mat4 offset, std::unordered_map<hashed_string, mat4>& pose, std::unordered_map<hashed_string, mat4>& overrideBones);

public:

    Animation* m_currAnim{ nullptr };

    string currentAnimationName = "";

    Animator() = default;
	Animator(SkinnedModel* model);
    ~Animator();
    void play();
    void set(const hashed_string& name);

    bool UpdatePose = true;

    std::vector<std::string> get();
    std::vector<glm::mat4>& getBoneMatrices();

    std::unordered_map<hashed_string, glm::mat4> currentPose;

    std::unordered_map<hashed_string, mat4> GetBonePoseArray();

    void ApplyBonePoseArray(std::unordered_map<hashed_string, mat4> pose);

    void ApplyLocalSpacePoseArray(std::unordered_map<hashed_string, mat4> pose, std::unordered_map<hashed_string, mat4> overridePose);

    void PopulateBonePoseArray(BoneNode& node, glm::mat4 offset, std::unordered_map<hashed_string, mat4>& outVector);

    void UpdateAnimationPose();

    void update(float dt);

    void updateRootMotion();

    void reset();
    
};
}
