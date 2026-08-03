#pragma once
#include "skinned_model.hpp"
#include "MathHelper.hpp"
#include <unordered_map>
#include <string>
#include <vector>

namespace roj
{

// ─────────────────────────────────────────────────────────────────────────────
// Animator — C++20 parallel hot path
//
// The three inner loops are:
//
//   Loop 1 — sample TRS (par_unseq):
//       Each bone is fully independent.  Runs in parallel + SIMD.
//       Template-dispatched on TLoop so the loop-wrap branch is a
//       compile-time constant, not a runtime check per bone.
//
//   Loop 2 — world pass (sequential):
//       parentIdx[i] < i is a hard data dependency chain.  Cannot be
//       parallelised.  The topological ordering means no branches are needed.
//
//   Loop 3 — skin pass (par_unseq):
//       Each write goes to a unique skinIdx slot.  No conflicts.
//       Runs in parallel + SIMD.
//
// currentPose (unordered_map) is populated lazily in GetBonePoseArray().
// It is never touched on the normal per-frame path.
// ─────────────────────────────────────────────────────────────────────────────

class Animator
{
public:
    // ── Public fields (kept 1:1 for SkeletalMesh.cpp) ────────────────────────
    bool  Loop               = false;
    float m_currTime         = 0.f;
    bool  m_playing          = false;

    bool  InterpolateRotation = true;
    bool  InterpolatePosition = true;
    bool  InterpolateScale    = true;

    MathHelper::Transform oldRootBoneTransform;
    MathHelper::Transform rootBoneTransform;

    glm::vec3 totalRootMotionPosition{};
    glm::vec3 totalRootMotionRotation{};

    bool UpdatePose           = true;
    bool UsePrecomputedFrames = false;

    Animation*  m_currAnim{ nullptr };  // legacy: SkeletalMesh reads duration/ticksPerSec
    std::string currentAnimationName;

    // Lazy snapshot. Not written on the normal per-frame path.
    std::unordered_map<hashed_string, glm::mat4> currentPose;

private:
    SkinnedModel*    m_model    = nullptr;
    EvaluatableClip* m_currClip = nullptr;

    glm::mat4* m_localPose = nullptr;   // [boneCount]
    glm::mat4* m_worldPose = nullptr;   // [boneCount]

    // Pre-allocated index list {0, 1, ..., boneCount-1}.
    // std::for_each(par_unseq) needs an iterable range; building this
    // vector inside evaluateClip each frame would be a heap allocation.
    // Stored once here and reused every frame.
    std::vector<uint16_t> m_boneIndices;

    std::vector<glm::mat4> m_boneMatrices;  // [MAX_SKINNED_BONES], GPU output
    bool m_localPoseDirty = true;

    // ── Private helpers ───────────────────────────────────────────────────────

    // TLoop is a compile-time constant: eliminates the loop-wrap branch
    // from every bone's TRS sample inside the parallel hot loop.
    template<bool TLoop>
    void evaluateClipImpl(float time);

    void evaluateClip(float time);  // dispatches to evaluateClipImpl<Loop>
    void applyBakedFrame(float time);
    void buildCurrentPose();
    void worldAndSkinPass();

public:
    Animator() = default;
    explicit Animator(SkinnedModel* model);

    Animator(const Animator&) = delete;
    Animator& operator=(const Animator&) = delete;
    Animator(Animator&&) noexcept;
    Animator& operator=(Animator&&) noexcept;
    ~Animator();

    void update(float dt);
    void play();
    void reset();
    void set(const hashed_string& name);

    std::vector<std::string> get();
    std::vector<glm::mat4>&  getBoneMatrices();

    const std::unordered_map<hashed_string, glm::mat4>& GetBonePoseArray();

    void ApplyBonePoseArray(const std::unordered_map<hashed_string, glm::mat4>& pose);
    void ApplyLocalSpacePoseArray(const std::unordered_map<hashed_string, glm::mat4>& pose,
                                  const std::unordered_map<hashed_string, glm::mat4>& overridePose);

    void UpdateAnimationPose();
    void updateRootMotion();

    void PopulateBonePoseArray(BoneNode& node, glm::mat4 offset,
                               std::unordered_map<hashed_string, glm::mat4>& outVector);
};

} // namespace roj
