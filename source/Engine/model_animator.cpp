#include "model_animator.hpp"

#include "Level.hpp"

#include "glm.h"


int roj::Animator::getKeyTransformIdx(std::vector<float>& timestamps)
{
    if (timestamps.size() < 2)
        return -1;

    float adjustedTime = m_currTime;

    // Non-looping: clamp to last segment if time exceeds last timestamp
    if (!Loop && adjustedTime >= timestamps.back()) {
        return timestamps.size() - 2;
    }

    // Find segment containing current time
    for (int index = 0; index < timestamps.size() - 1; ++index) {
        if (adjustedTime < timestamps[index + 1]) {
            return index;
        }
    }

    return timestamps.size() - 2; // Fallback (shouldn't reach here for looping)
}

float roj::Animator::getScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime)
{
    if (m_currTime < lastTimeStamp) return 0.f;
    float midWayLength = m_currTime - lastTimeStamp;
    float framesDiff = nextTimeStamp - lastTimeStamp;
    return midWayLength / framesDiff;
}

glm::mat4 roj::Animator::interpolatePosition(roj::FrameBoneTransform& boneTransform)
{
    // if there are no position keys, return identity
    if (boneTransform.positionTimestamps.empty() || boneTransform.positions.empty())
        return glm::translate(glm::mat4(1.0f), glm::vec3(0.0f));

    int posIdx = getKeyTransformIdx(boneTransform.positionTimestamps);
    if (posIdx == -1) {
        // no earlier key — use first key (discrete)
        return glm::translate(glm::mat4(1.0f), boneTransform.positions[0]);
    }

    // If interpolation is disabled, return the discrete key value at posIdx
    if (!InterpolatePosition) {
        // posIdx will be valid index into positions
        return glm::translate(glm::mat4(1.0f), boneTransform.positions[posIdx]);
    }

    // Loop wrap interpolation (blend last -> first across animation boundary)
    if (Loop && posIdx == static_cast<int>(boneTransform.positionTimestamps.size()) - 1)
    {
        float lastTime = boneTransform.positionTimestamps.back();
        float nextTime = boneTransform.positionTimestamps[0] + m_currAnim->duration;
        float scaleFactor = (m_currTime - lastTime) / (nextTime - lastTime);
        glm::vec3 finalPosition = glm::mix(
            boneTransform.positions.back(),
            boneTransform.positions[0],
            scaleFactor
        );
        return glm::translate(glm::mat4(1.0f), finalPosition);
    }

    // Normal interpolation between posIdx and posIdx + 1
    float scaleFactor = getScaleFactor(
        boneTransform.positionTimestamps[posIdx],
        boneTransform.positionTimestamps[posIdx + 1],
        m_currTime
    );
    glm::vec3 finalPosition = glm::mix(
        boneTransform.positions[posIdx],
        boneTransform.positions[posIdx + 1],
        scaleFactor
    );
    return glm::translate(glm::mat4(1.0f), finalPosition);
}

glm::mat4 roj::Animator::interpolateRotation(roj::FrameBoneTransform& boneTransform)
{
    if (boneTransform.rotationTimestamps.empty() || boneTransform.rotations.empty())
        return glm::toMat4(glm::quat(1.0f, 0.0f, 0.0f, 0.0f)); // identity rotation

    int posIdx = getKeyTransformIdx(boneTransform.rotationTimestamps);
    if (posIdx == -1)
        return glm::toMat4(glm::normalize(boneTransform.rotations[0]));

    // If interpolation disabled, use the discrete rotation at posIdx
    if (!InterpolateRotation) {
        return glm::toMat4(glm::normalize(boneTransform.rotations[posIdx]));
    }

    if (Loop && posIdx == static_cast<int>(boneTransform.rotationTimestamps.size()) - 1)
    {
        float lastTime = boneTransform.rotationTimestamps.back();
        float nextTime = boneTransform.rotationTimestamps[0] + m_currAnim->duration;
        float scaleFactor = (m_currTime - lastTime) / (nextTime - lastTime);
        glm::quat finalRotation = glm::slerp(
            boneTransform.rotations.back(),
            boneTransform.rotations[0],
            scaleFactor
        );
        return glm::toMat4(glm::normalize(finalRotation));
    }

    float scaleFactor = getScaleFactor(
        boneTransform.rotationTimestamps[posIdx],
        boneTransform.rotationTimestamps[posIdx + 1],
        m_currTime
    );
    glm::quat finalRotation = glm::slerp(
        boneTransform.rotations[posIdx],
        boneTransform.rotations[posIdx + 1],
        scaleFactor
    );
    return glm::toMat4(glm::normalize(finalRotation));
}

glm::mat4 roj::Animator::interpolateScaling(roj::FrameBoneTransform& boneTransform)
{
    if (boneTransform.scaleTimestamps.empty() || boneTransform.scales.empty())
        return glm::scale(glm::mat4(1.0f), glm::vec3(1.0f)); // identity scale

    int posIdx = getKeyTransformIdx(boneTransform.scaleTimestamps);
    if (posIdx == -1)
        return glm::scale(glm::mat4(1.0f), boneTransform.scales[0]);

    // If interpolation disabled, use discrete scale at posIdx
    if (!InterpolateScale) {
        return glm::scale(glm::mat4(1.0f), boneTransform.scales[posIdx]);
    }

    if (Loop && posIdx == static_cast<int>(boneTransform.scaleTimestamps.size()) - 1)
    {
        float lastTime = boneTransform.scaleTimestamps.back();
        float nextTime = boneTransform.scaleTimestamps[0] + m_currAnim->duration;
        float scaleFactor = (m_currTime - lastTime) / (nextTime - lastTime);
        glm::vec3 finalScale = glm::mix(
            boneTransform.scales.back(),
            boneTransform.scales[0],
            scaleFactor
        );
        return glm::scale(glm::mat4(1.0f), finalScale);
    }

    float scaleFactor = getScaleFactor(
        boneTransform.scaleTimestamps[posIdx],
        boneTransform.scaleTimestamps[posIdx + 1],
        m_currTime
    );
    glm::vec3 finalScale = glm::mix(
        boneTransform.scales[posIdx],
        boneTransform.scales[posIdx + 1],
        scaleFactor
    );

    return glm::scale(glm::mat4(1.0f), finalScale);
}


void roj::Animator::calcBoneTransform(BoneNode& node, glm::mat4 offset, bool stopAfterRoot) {
    // Check if we have cached FrameBoneTransform data for this bone
    if (node.id < cachedFrameBoneTransforms.size()) {
        auto& boneTransform = cachedFrameBoneTransforms[node.id];
        glm::mat4 translation = interpolatePosition(boneTransform);
        glm::mat4 rotation = interpolateRotation(boneTransform);
        glm::mat4 scale = interpolateScaling(boneTransform);

        currentPose[node.name] = translation * rotation * scale;
        offset *= currentPose[node.name];
    }
    else {
        // Fallback to node transform if no animation data
        currentPose[node.name] = node.transform;
        offset *= node.transform;
    }

    // Check if we have cached BoneInfo data for this bone
    if (node.id < cachedBoneInfos.size()) {
        auto& boneInfo = cachedBoneInfos[node.id];
        m_boneMatrices[boneInfo.id] = offset * boneInfo.offset;
    }

    // Handle root bone special case
    if (node.name == "root") {
        auto trans = MathHelper::DecomposeMatrix(offset);
        rootBoneTransform = trans;
        if (stopAfterRoot) return;
    }

    // Recursively process children
    for (roj::BoneNode& child : node.children) {
        calcBoneTransform(child, offset, stopAfterRoot);
    }
}

void roj::Animator::useBakedFrame(float time)
{

	int currentFrame = static_cast<int>(time / m_currAnim->ticksPerSec / m_currAnim->bakedFrameInterval);

    if (currentFrame >= m_currAnim->bakedFrames.size())
    {
        currentFrame = m_currAnim->bakedFrames.size() - 1;
	}

	auto& BakedFrameData = m_currAnim->bakedFrames[currentFrame];

    currentPose = BakedFrameData.boneTransforms;
    m_boneMatrices = BakedFrameData.modelTransform;
    totalRootMotionPosition = BakedFrameData.totalRootMotionPosition;
	totalRootMotionRotation = BakedFrameData.totalRootMotionRotation;

}

void roj::Animator::ApplyNodePose(BoneNode& node, glm::mat4 offset, std::unordered_map<hashed_string, mat4>& pose)
{

    auto poseRes = pose.find(node.name);
    if (poseRes != pose.end())
    {
        currentPose[node.name] = pose[node.name];
    }
    else
    {
        currentPose[node.name] = node.transform;
    }
    
    offset *= currentPose[node.name];

    auto it2 = m_model->boneInfoMap.find(node.name);
    if (it2 != m_model->boneInfoMap.end())
    {
        auto& boneInfo = it2->second;

        m_boneMatrices[boneInfo.id] = offset * boneInfo.offset;
    }

    for (roj::BoneNode& child : node.children)
    {
        ApplyNodePose(child, offset, pose);
    }
}

void roj::Animator::ApplyNodePoseLocalSpace(BoneNode& node, glm::mat4 offset, std::unordered_map<hashed_string, mat4>& pose, std::unordered_map<hashed_string, mat4>& overrideBones)
{

    bool hasLocalPose = false;

    mat4 localPose;

    auto overRes = overrideBones.find(node.name);
    
    if (overRes != overrideBones.end())
    {

        localPose = overRes->second;

        hasLocalPose = true;

    }

    auto poseRes = pose.find(node.name);
    if (poseRes != pose.end())
    {
        currentPose[node.name] = pose[node.name];
    }
    else
    {
        currentPose[node.name] = node.transform;
    }


    if (hasLocalPose)
    {

        currentPose[node.name] = inverse(offset) * localPose;

        offset = localPose;
        

    }
    else
    {
        offset *= currentPose[node.name];
    }

    



    auto it2 = m_model->boneInfoMap.find(node.name);
    if (it2 != m_model->boneInfoMap.end())
    {
        auto& boneInfo = it2->second;

        m_boneMatrices[boneInfo.id] = offset * boneInfo.offset;
    }

    for (roj::BoneNode& child : node.children)
    {
        ApplyNodePoseLocalSpace(child, offset, pose, overrideBones);
    }
}

roj::Animator::Animator(SkinnedModel* model)
    : m_model(model)
{
    m_boneMatrices.resize(model->boneInfoMap.size());

    for (int i = 0; i < m_boneMatrices.size(); i++)
    {
        m_boneMatrices[i] = glm::identity<mat4>();
    }

}

roj::Animator::~Animator()
{

}

void roj::Animator::set(const hashed_string& name)
{

    auto it = m_model->animations.find(name);
    if (it != m_model->animations.end()) {
        m_currAnim = &it->second;
        m_currTime = 0.0f;
        currentAnimationName = name.str();

        precacheAnimation();

    }
}
std::vector<std::string> roj::Animator::get()
{
    std::vector<std::string> animNames;
    animNames.reserve(m_model->animations.size());
    for (auto& anim : m_model->animations)
    {
        animNames.emplace_back(anim.first.str());
    }

    return animNames;
}

std::vector<glm::mat4>& roj::Animator::getBoneMatrices()
{
    return m_boneMatrices;
}

std::unordered_map<hashed_string, mat4> roj::Animator::GetBonePoseArray()
{

    return currentPose;
    /*
    std::unordered_map<hashed_string, mat4> outVector = std::unordered_map<hashed_string, mat4>();

    if (m_currAnim) 
    {
        PopulateBonePoseArray(m_currAnim->rootBone, glm::mat4(1.0f), outVector);
    }

    return outVector;
    */
}

void roj::Animator::ApplyBonePoseArray(std::unordered_map<hashed_string, mat4> pose)
{

    ApplyNodePose(m_model->defaultRoot, glm::identity<mat4>(), pose);
}

void roj::Animator::ApplyLocalSpacePoseArray(std::unordered_map<hashed_string, mat4> pose, std::unordered_map<hashed_string, mat4> overridePose)
{

    ApplyNodePoseLocalSpace(m_model->defaultRoot, glm::identity<mat4>(), pose, overridePose);

}

void roj::Animator::PopulateBonePoseArray(BoneNode& node, glm::mat4 offset, std::unordered_map<hashed_string, mat4>& outVector)
{
    
    outVector[node.name] = currentPose[node.name];

    for (roj::BoneNode& child : node.children)
    {
        PopulateBonePoseArray(child, offset, outVector);
    }
}

void roj::Animator::UpdateAnimationPose()
{
    if (m_currAnim)
    {
        calcBoneTransform(m_currAnim->rootBone, glm::identity<mat4>(), false);
    }
    
}

void roj::Animator::update(float dt)
{
    if (m_model == nullptr) return;

    if (m_currAnim && m_playing)
    {
        m_currTime += m_currAnim->ticksPerSec * dt;

        // Simplified time management
        if (Loop) 
        {



            if (m_currTime > m_currAnim->duration)
            {
                m_currTime -= m_currAnim->duration;// -m_currAnim->frameTime;
            }

        }
        else {
            m_currTime = glm::min(m_currTime, m_currAnim->duration);
        }

        m_playing = Loop || (m_currTime < m_currAnim->duration);

        if (UsePrecomputedFrames)
        {
			useBakedFrame(m_currTime);
        }
        else
        {
            calcBoneTransform(m_currAnim->rootBone, glm::identity<mat4>(), UpdatePose == false);
            updateRootMotion();
        }


    }
}

void roj::Animator::updateRootMotion()
{

    vec3 motionPos = rootBoneTransform.Position - oldRootBoneTransform.Position;
    vec3 motionRot = MathHelper::ToYawPitchRoll(inverse(oldRootBoneTransform.RotationQuaternion) * rootBoneTransform.RotationQuaternion);


    totalRootMotionPosition += motionPos;
    totalRootMotionRotation += motionRot;

    oldRootBoneTransform = rootBoneTransform;
    

}

void roj::Animator::play()
{
    m_playing = true;

    if (m_currAnim && Loop && false)
    {
        m_currTime = m_currAnim->frameTime;
    }
    else
    {
        m_currTime = 0;
    }


}


void roj::Animator::reset()
{
    m_currTime = 0.0f;
    currentPose.clear();
	totalRootMotionPosition = vec3();
	totalRootMotionRotation = vec3();
	m_playing = false;
}

void roj::Animator::precacheAnimation() {
    if (!m_currAnim) return;

    cachedFrameBoneTransforms.clear();
    cachedBoneInfos.clear();

    cachedFrameBoneTransforms = std::vector<roj::FrameBoneTransform>();
	cachedFrameBoneTransforms.reserve(m_model->boneInfoMap.size());
	cachedBoneInfos = std::vector<roj::BoneInfo>();
	cachedBoneInfos.reserve(m_model->boneInfoMap.size());

    std::function<void(BoneNode&, uint16_t&)> assignIdsAndCache =
        [&](BoneNode& node, uint16_t& currentId) {
        node.id = currentId++;

        auto frameIt = m_currAnim->animationFrames.find(node.name);
        if (frameIt != m_currAnim->animationFrames.end()) {
            cachedFrameBoneTransforms.push_back(frameIt->second);
        }
        else {
            roj::FrameBoneTransform defaultTransform;
            auto defaultTrans = MathHelper::DecomposeMatrix(node.transform);
            defaultTransform.positionTimestamps = { 0.0f };
            defaultTransform.positions = { defaultTrans.Position };
            defaultTransform.rotationTimestamps = { 0.0f };
            defaultTransform.rotations = { defaultTrans.RotationQuaternion };
            defaultTransform.scaleTimestamps = { 0.0f };
            defaultTransform.scales = { defaultTrans.Scale };
            cachedFrameBoneTransforms.push_back(defaultTransform);
        }

        auto boneIt = m_model->boneInfoMap.find(node.name);
        if (boneIt != m_model->boneInfoMap.end()) {
            cachedBoneInfos.push_back(boneIt->second);
        }
        else {
            cachedBoneInfos.push_back(roj::BoneInfo());
        }

        for (BoneNode& child : node.children) {
            assignIdsAndCache(child, currentId);
        }
        };

    uint16_t idCounter = 0;
    assignIdsAndCache(m_currAnim->rootBone, idCounter);
}