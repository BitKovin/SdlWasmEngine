#ifndef ANIMATOR_HPP
#define ANIMATOR_HPP
#include "skinned_model.hpp"
namespace roj
{
class Animator
{
private:
    std::vector<glm::mat4> m_boneMatrices;
    Animation* m_currAnim{nullptr};
    SkinnedModel* m_model;
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
public:
    Animator() = default;
	Animator(SkinnedModel& model);
    void play();
    void loop(bool enable);
    void set(const std::string& name);
    std::vector<std::string> get();
    std::vector<glm::mat4>& getBoneMatrices();
    void update(float dt);
    void reset();
    
};
}
#endif //-ANIMATOR_HPP
