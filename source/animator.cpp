#include "animator.hpp"

#include "glm.h"


int roj::Animator::getKeyTransformIdx(std::vector<float>& timestamps)
{
    if (timestamps.size() < 2)
        return -1;
    
    for (int index = 0; index < timestamps.size() - 1; ++index)
    {
        if (m_currTime < timestamps[index + 1])
            return index;
    }

    return timestamps.size() - 2;
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
    int posIdx = getKeyTransformIdx(boneTransform.positionTimestamps);
    if (posIdx == -1)
        return glm::translate(glm::mat4(1.0f), boneTransform.positions[0]);

    float scaleFactor = getScaleFactor(boneTransform.positionTimestamps[posIdx],
        boneTransform.positionTimestamps[posIdx + 1], m_currTime);
    glm::vec3 finalPosition = glm::mix(boneTransform.positions[posIdx],
        boneTransform.positions[posIdx+1], scaleFactor);
    return glm::translate(glm::mat4(1.0f), finalPosition);

}

glm::mat4 roj::Animator::interpolateRotation(roj::FrameBoneTransform& boneTransform)
{
    int posIdx = getKeyTransformIdx(boneTransform.rotationTimestamps);
    if (posIdx == -1)
        return glm::toMat4(glm::normalize(boneTransform.rotations[0]));

    
    float scaleFactor = getScaleFactor(boneTransform.rotationTimestamps[posIdx],
        boneTransform.rotationTimestamps[posIdx + 1], m_currTime);

    glm::quat finalRotation = glm::slerp(boneTransform.rotations[posIdx],
        boneTransform.rotations[posIdx + 1], scaleFactor);

    return glm::toMat4(glm::normalize(finalRotation));
}

glm::mat4 roj::Animator::interpolateScaling( roj::FrameBoneTransform& boneTransform)
{

    int posIdx = getKeyTransformIdx(boneTransform.scaleTimestamps);
    if (posIdx == -1)
        return glm::scale(glm::mat4(1.0f), boneTransform.scales[0]);
    float scaleFactor = getScaleFactor(boneTransform.scaleTimestamps[posIdx],
        boneTransform.scaleTimestamps[posIdx + 1], m_currTime);
    glm::vec3 finalScale = glm::mix(boneTransform.scales[posIdx], boneTransform.scales[posIdx+1], scaleFactor);
    return glm::scale(glm::mat4(1.0f), finalScale);
}

void roj::Animator::calcBoneTransform(BoneNode& node, glm::mat4 offset)
{
    auto it = m_currAnim->animationFrames.find(node.name);
    if (it != m_currAnim->animationFrames.end())
    {
        auto& boneTransform = it->second;
        glm::mat4 translation = interpolatePosition(boneTransform);
        glm::mat4 rotation = interpolateRotation(boneTransform);
        glm::mat4 scale = interpolateScaling(boneTransform);
        offset *= translation * rotation * scale;
    }
    else
    {
        offset *= node.transform;
    }

    auto it2 = m_model.boneInfoMap.find(node.name);
    if (it2 != m_model.boneInfoMap.end())
    {
        auto& boneInfo = it2->second;
        m_boneMatrices[boneInfo.id] = offset * boneInfo.offset;
    }

    for (roj::BoneNode& child : node.children)
    {
        calcBoneTransform(child, offset);
    }
}

roj::Animator::Animator(SkinnedModel* model)
    : m_model(*model)
{
    m_boneMatrices.resize(model->boneInfoMap.size());

    for (int i = 0; i < m_boneMatrices.size(); i++)
    {
        m_boneMatrices[i] = identity<mat4>();
    }

}

void roj::Animator::set(const std::string& name)
{
    auto it = m_model.animations.find(name);
    if (it != m_model.animations.end()) {
        m_currAnim = &it->second;
        m_currTime = 0.0f;
    }
}
std::vector<std::string> roj::Animator::get()
{
    std::vector<std::string> animNames;
    animNames.reserve(m_model.animations.size());
    for (auto& anim : m_model.animations)
    {
        animNames.emplace_back(anim.first);
    }

    return animNames;
}

std::vector<glm::mat4>& roj::Animator::getBoneMatrices()
{
    return m_boneMatrices;
}

void roj::Animator::update(float dt)
{
    if (m_currAnim && m_playing) {
        m_playing  = (m_loopEnabled) ? true : (m_currTime < m_currAnim->duration);
        m_currTime = m_currTime + (m_currAnim->ticksPerSec * dt);
        if (m_currTime >= m_currAnim->duration)
            m_currTime -= m_currAnim->duration;

        calcBoneTransform(m_currAnim->rootBone, glm::mat4(1.0f));
    }
}

void roj::Animator::play()
{
    m_playing = true;
    m_currTime = 0.0f;
}


void roj::Animator::reset()
{
    m_currTime = 0.0f;
}