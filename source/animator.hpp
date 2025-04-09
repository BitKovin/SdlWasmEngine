#ifndef ANIMATOR_HPP
#define ANIMATOR_HPP
#include "skinned_model.hpp"

#include <unordered_map>
#include <string>

namespace roj
{
class Animator
{

public : 

    bool Loop = false;

private:

    std::vector<glm::mat4> m_boneMatrices;
    Animation* m_currAnim{nullptr};
    SkinnedModel m_model;
    float m_currTime{0.0f};
    bool m_playing = false;
    bool m_loopEnabled = false;
private:
    int getKeyTransformIdx(std::vector<float>& timestamps);
    float getScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime);
    glm::mat4 interpolatePosition(FrameBoneTransform& boneTransform);
    glm::mat4 interpolateRotation(FrameBoneTransform& boneTransform);
    glm::mat4 interpolateScaling(FrameBoneTransform& boneTransform);
    void calcBoneTransform(BoneNode& node, glm::mat4 offset);

    void ApplyNodePose(BoneNode& node, glm::mat4 offset, std::unordered_map<std::string, mat4>& pose);

public:
    Animator() = default;
	Animator(SkinnedModel* model);
    void play();
    void set(const std::string& name);
    std::vector<std::string> get();
    std::vector<glm::mat4>& getBoneMatrices();

    std::unordered_map<std::string, mat4> GetBonePoseArray();

    void ApplyBonePoseArray(std::unordered_map<std::string, mat4> pose);


    void PopulateBonePoseArray(BoneNode& node, glm::mat4 offset, std::unordered_map<std::string, mat4>& outVector);


    void update(float dt);
    void reset();
    
};
}
#endif //-ANIMATOR_HPP
