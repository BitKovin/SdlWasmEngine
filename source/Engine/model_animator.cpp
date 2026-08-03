#include "model_animator.hpp"
#include "Level.hpp"
#include "glm.h"

#include <algorithm>    // std::lower_bound, std::for_each
#include <execution>    
#include <numeric>      // std::iota
#include <cstring>      // std::memcpy
#include <cassert>
#include <span>

#if defined(__EMSCRIPTEN__ )|| defined(__ANDROID__)

#define SUPPORTED_FOR_EXECUTION 

#define SUPPORTED_FOR_PAR_EXECUTION 

#else

#define SUPPORTED_FOR_EXECUTION std::execution::unseq, //when parallel logic will break stuff
#define SUPPORTED_FOR_PAR_EXECUTION std::execution::unseq, //par_unseq, 

#endif // __EMSCRIPTEN__



    
namespace roj
{

// ─────────────────────────────────────────────────────────────────────────────
// keyIdx — binary search over a span<const float>
//
// std::span carries both pointer and size as a unit.  The compiler knows the
// memory is contiguous (a guarantee raw pointers alone don't carry), enabling
// better alias analysis and auto-vectorisation of lower_bound when inlined.
// ─────────────────────────────────────────────────────────────────────────────

[[nodiscard]] static inline int keyIdx(std::span<const float> ts, float time, bool loop) noexcept
{
    const int n = static_cast<int>(ts.size());
    if (n < 2) [[unlikely]] return -1;

    if (!loop && time >= ts.back()) [[unlikely]]
        return n - 2;

    const auto it = std::lower_bound(ts.begin(), ts.end(), time);
    if (it == ts.end())   [[unlikely]] return n - 2;
    if (it == ts.begin()) [[unlikely]] return 0;
    return static_cast<int>(it - ts.begin()) - 1;
}

// ─────────────────────────────────────────────────────────────────────────────
// sampleBone<TLoop>
//
// TLoop is a compile-time constant. When false, the loop-wrap block is
// entirely dead code — no branches, no comparisons.
//
// The evaluateClipImpl caller is itself templated on TLoop, so this branch
// is resolved at compile time, once per translation unit, not once per
// bone per frame at runtime.
//
// All three TRS channels are sampled in the same call so the compiler keeps
// 'time', 'dur', and the span base pointers in registers across channels.
//
// Result is built directly from TRS — no intermediate mat4s, no multiplications.
// ─────────────────────────────────────────────────────────────────────────────

template<bool TLoop>
[[nodiscard]] static glm::mat4 sampleBone(
    const BoneTrack& t,
    float  time,
    float  dur,
    bool   interpPos,
    bool   interpRot,
    bool   interpScale) noexcept
{
    // ── Position ─────────────────────────────────────────────────────────────
    glm::vec3 pos(0.f);
    if (t.posValues.size() == 1) [[likely]]
    {
        pos = t.posValues[0];
    }
    else if (!t.posValues.empty()) [[likely]]
    {
        const int i = keyIdx(t.posTimes, time, TLoop);
        if (i < 0) [[unlikely]]
            pos = t.posValues[0];
        else if (!interpPos) [[unlikely]]
            pos = t.posValues[i];
        else if constexpr (TLoop)
        {
            if (i == static_cast<int>(t.posTimes.size()) - 1) [[unlikely]]
            {
                const float lt = t.posTimes.back();
                pos = glm::mix(t.posValues.back(), t.posValues[0],
                               (time - lt) / (t.posTimes[0] + dur - lt));
            }
            else [[likely]]
            {
                pos = glm::mix(t.posValues[i], t.posValues[i + 1],
                               (time - t.posTimes[i]) / (t.posTimes[i + 1] - t.posTimes[i]));
            }
        }
        else [[likely]]
        {
            pos = glm::mix(t.posValues[i], t.posValues[i + 1],
                           (time - t.posTimes[i]) / (t.posTimes[i + 1] - t.posTimes[i]));
        }
    }

    // ── Rotation ─────────────────────────────────────────────────────────────
    glm::quat rot(1.f, 0.f, 0.f, 0.f);
    if (t.rotValues.size() == 1) [[likely]]
    {
        rot = t.rotValues[0];
    }
    else if (!t.rotValues.empty()) [[likely]]
    {
        const int i = keyIdx(t.rotTimes, time, TLoop);
        if (i < 0) [[unlikely]]
            rot = glm::normalize(t.rotValues[0]);
        else if (!interpRot) [[unlikely]]
            rot = t.rotValues[i];
        else if constexpr (TLoop)
        {
            if (i == static_cast<int>(t.rotTimes.size()) - 1) [[unlikely]]
            {
                const float lt = t.rotTimes.back();
                rot = glm::normalize(glm::slerp(t.rotValues.back(), t.rotValues[0],
                                                (time - lt) / (t.rotTimes[0] + dur - lt)));
            }
            else [[likely]]
            {
                rot = glm::normalize(glm::slerp(t.rotValues[i], t.rotValues[i + 1],
                                                (time - t.rotTimes[i]) / (t.rotTimes[i + 1] - t.rotTimes[i])));
            }
        }
        else [[likely]]
        {
            rot = glm::normalize(glm::slerp(t.rotValues[i], t.rotValues[i + 1],
                                            (time - t.rotTimes[i]) / (t.rotTimes[i + 1] - t.rotTimes[i])));
        }
    }

    // ── Scale — single key is the overwhelmingly common case ─────────────────
    glm::vec3 scl(1.f);
    if (t.scaleValues.size() == 1) [[likely]]
    {
        scl = t.scaleValues[0];
    }
    else if (!t.scaleValues.empty()) [[unlikely]]
    {
        const int i = keyIdx(t.scaleTimes, time, TLoop);
        if (i < 0) [[unlikely]]
            scl = t.scaleValues[0];
        else if (!interpScale) [[unlikely]]
            scl = t.scaleValues[i];
        else if constexpr (TLoop)
        {
            if (i == static_cast<int>(t.scaleTimes.size()) - 1) [[unlikely]]
            {
                const float lt = t.scaleTimes.back();
                scl = glm::mix(t.scaleValues.back(), t.scaleValues[0],
                               (time - lt) / (t.scaleTimes[0] + dur - lt));
            }
            else [[likely]]
            {
                scl = glm::mix(t.scaleValues[i], t.scaleValues[i + 1],
                               (time - t.scaleTimes[i]) / (t.scaleTimes[i + 1] - t.scaleTimes[i]));
            }
        }
        else [[likely]]
        {
            scl = glm::mix(t.scaleValues[i], t.scaleValues[i + 1],
                           (time - t.scaleTimes[i]) / (t.scaleTimes[i + 1] - t.scaleTimes[i]));
        }
    }

    // ── TRS → mat4 in place — no intermediate matrices ───────────────────────
    glm::mat4 m = glm::toMat4(rot);
    m[0] *= scl.x;
    m[1] *= scl.y;
    m[2] *= scl.z;
    m[3]  = glm::vec4(pos, 1.f);
    return m;
}

// ─────────────────────────────────────────────────────────────────────────────
// worldAndSkinPass — loops 2 and 3
//
// Loop 2 (world accumulation): strict parent→child dependency chain.
//   parentIdx[i] < i by topological order, so the forward pass is always
//   correct with no branches.  Cannot be parallelised.
//
// Loop 3 (skinning output): each bone writes to a unique skinIdx slot.
//   par_unseq — parallel threads + SIMD.  mat4 * mat4 is 16 FMAs per bone,
//   which is exactly the kind of work execution::par_unseq benefits from.
// ─────────────────────────────────────────────────────────────────────────────

void Animator::worldAndSkinPass()
{
    const FlatSkeleton& skel = m_model->skeleton;
    const uint16_t n = skel.boneCount;

    if (n == 0)
    {
        Logger::Log("Trying to apply pose, but model doesn't have bones");
        return;
    }

    // Loop 2 — sequential
    m_worldPose[0] = m_localPose[0];
    for (uint16_t i = 1; i < n; ++i)
    {
        const uint16_t p = skel.bones[i].parentIdx;
        m_worldPose[i] = m_worldPose[p] * m_localPose[i];
    }

    // Loop 3 — par_unseq: independent writes to unique skinIdx slots
    glm::mat4*          boneMatrices = m_boneMatrices.data();
    const glm::mat4*    worldPose    = m_worldPose;
    const SkeletonBone* bones        = skel.bones;

    std::for_each(
        SUPPORTED_FOR_PAR_EXECUTION
        m_boneIndices.begin(), m_boneIndices.end(),
        [boneMatrices, worldPose, bones](uint16_t i)
        {
            const int16_t si = bones[i].skinIdx;
            if (si >= 0)
                [[likely]] boneMatrices[si] = worldPose[i] * bones[i].invBind;
        });

    if (skel.rootMotionBoneIdx != INVALID_BONE_IDX) [[likely]]
        rootBoneTransform = MathHelper::DecomposeMatrix(m_worldPose[skel.rootMotionBoneIdx]);
}

// ─────────────────────────────────────────────────────────────────────────────
// evaluateClipImpl<TLoop>
//
// Loop 1 — par_unseq: each bone's TRS sample is fully independent.
// The lambda captures raw pointers (not 'this') so the execution policy can
// dispatch it on any thread without touching the Animator object itself.
//
// 'Loop' is hoisted out of the lambda as the compile-time TLoop parameter.
// ─────────────────────────────────────────────────────────────────────────────

template<bool TLoop>
void Animator::evaluateClipImpl(float time)
{
    const float         dur    = m_currClip->duration;
    const bool          iPos   = InterpolatePosition;
    const bool          iRot   = InterpolateRotation;
    const bool          iScl   = InterpolateScale;
    const BoneTrack*    tracks = m_currClip->tracks;
    glm::mat4*          lp     = m_localPose;

    std::for_each(
        SUPPORTED_FOR_PAR_EXECUTION
        m_boneIndices.begin(), m_boneIndices.end(),
        [lp, tracks, time, dur, iPos, iRot, iScl](uint16_t i)
        {
            lp[i] = sampleBone<TLoop>(tracks[i], time, dur, iPos, iRot, iScl);
        });

    m_localPoseDirty = true;
    worldAndSkinPass();
}

void Animator::evaluateClip(float time)
{
    // Single branch per frame — not per bone.
    if (Loop) evaluateClipImpl<true>(time);
    else      evaluateClipImpl<false>(time);
}

// ─────────────────────────────────────────────────────────────────────────────
// applyBakedFrame
// ─────────────────────────────────────────────────────────────────────────────

void Animator::applyBakedFrame(float time)
{
    int frame = static_cast<int>(time / m_currClip->ticksPerSec / m_currClip->bakedFrameInterval);
    const int maxFrame = static_cast<int>(m_currClip->bakedFrames.size()) - 1;
    frame = glm::clamp(frame, 0, maxFrame);

    const auto& bf = m_currClip->bakedFrames[frame];


    const size_t copyCount = std::min(m_boneMatrices.size(), bf.skinMatrices.size());
    std::memcpy(m_boneMatrices.data(), bf.skinMatrices.data(),
        copyCount * sizeof(glm::mat4));

    const uint16_t n = m_model->skeleton.boneCount;
    if (static_cast<uint16_t>(bf.localPoses.size()) == n)
        std::memcpy(m_localPose, bf.localPoses.data(), n * sizeof(glm::mat4));

    m_localPoseDirty = true;
    totalRootMotionPosition = bf.rootMotionPos;
    totalRootMotionRotation = bf.rootMotionRot;
}

// ─────────────────────────────────────────────────────────────────────────────
// buildCurrentPose — lazy, par_unseq
//
// Called only when SkeletalMesh needs a pose snapshot (blend-out, ragdoll).
// currentPose keys were pre-inserted at construction so no rehashing occurs.
// ─────────────────────────────────────────────────────────────────────────────

void Animator::buildCurrentPose()
{
    if (!m_localPoseDirty) return;

    const SkeletonBone* bones = m_model->skeleton.bones;
    const glm::mat4*    lp    = m_localPose;

    for (uint16_t i : m_boneIndices)
        currentPose[bones[i].name] = lp[i];

    m_localPoseDirty = false;
}

// ─────────────────────────────────────────────────────────────────────────────
// update
// ─────────────────────────────────────────────────────────────────────────────

void Animator::update(float dt)
{
    if (!m_model || !m_currClip || !m_playing) [[unlikely]] return;

    m_currTime += m_currClip->ticksPerSec * dt;

    if (Loop)
    {
        if (m_currTime >= m_currClip->duration && UpdatePose)
        {
            // Phase 1: accumulate motion from current baseline to end of clip
            evaluateClip(m_currClip->duration);
            totalRootMotionPosition += rootBoneTransform.Position - oldRootBoneTransform.Position;
            totalRootMotionRotation += MathHelper::ToYawPitchRoll(
                glm::inverse(oldRootBoneTransform.RotationQuaternion) *
                rootBoneTransform.RotationQuaternion);

            // Phase 2: seed oldRootBoneTransform from frame 0 so the next
            // updateRootMotion() computes delta from start, not from end —
            // preventing the backward snap on wrap
            evaluateClip(0.f);
            oldRootBoneTransform = rootBoneTransform;
        }

        m_currTime = std::fmod(m_currTime, m_currClip->duration);
        if (m_currTime < 0.f) m_currTime += m_currClip->duration;
    }
    else
    {
        m_currTime = glm::min(m_currTime, m_currClip->duration);
    }

    m_playing = Loop || (m_currTime < m_currClip->duration);

    if (UsePrecomputedFrames) [[unlikely]]
        applyBakedFrame(m_currTime);
    else
    {
        evaluateClip(m_currTime);
        if (UpdatePose) [[likely]]
            updateRootMotion(); // now computes P_curr - P_start correctly
    }
}

void Animator::updateRootMotion()
{
    const glm::vec3 motionPos =
        rootBoneTransform.Position - oldRootBoneTransform.Position;
    const glm::vec3 motionRot = MathHelper::ToYawPitchRoll(
        glm::inverse(oldRootBoneTransform.RotationQuaternion) *
        rootBoneTransform.RotationQuaternion);

    totalRootMotionPosition += motionPos;
    totalRootMotionRotation += motionRot;
    oldRootBoneTransform     = rootBoneTransform;
}

// ─────────────────────────────────────────────────────────────────────────────
// ApplyBonePoseArray — par_unseq fill, then world+skin pass
// ─────────────────────────────────────────────────────────────────────────────

void Animator::ApplyBonePoseArray(const std::unordered_map<hashed_string, glm::mat4>& pose)
{
    const SkeletonBone* bones = m_model->skeleton.bones;
    glm::mat4*          lp   = m_localPose;

    std::for_each(
        SUPPORTED_FOR_EXECUTION
        m_boneIndices.begin(), m_boneIndices.end(),
        [lp, bones, &pose](uint16_t i)
        {
            const auto it = pose.find(bones[i].name);
            lp[i] = (it != pose.end()) ? it->second : bones[i].localBind;
        });

    m_localPoseDirty = true;
    worldAndSkinPass();
}

// ─────────────────────────────────────────────────────────────────────────────
// ApplyLocalSpacePoseArray
// Step 1 (fill): par_unseq.  Step 2 (world+snap): sequential.
// Step 3 (skin): par_unseq inside worldAndSkinPass → extracted inline here
// because the override path writes m_worldPose[] mid-loop.
// ─────────────────────────────────────────────────────────────────────────────

void Animator::ApplyLocalSpacePoseArray(
    const std::unordered_map<hashed_string, glm::mat4>& pose,
    const std::unordered_map<hashed_string, glm::mat4>& overridePose)
{
    const FlatSkeleton& skel = m_model->skeleton;
    const uint16_t      n    = skel.boneCount;
    const SkeletonBone* bones = skel.bones;
    glm::mat4*          lp   = m_localPose;

    if (n == 0)
    {
        Logger::Log("Trying to apply pose, but model doesn't have bones");
        return;
    }

    // Step 1: fill local pose — par_unseq
    std::for_each(
        SUPPORTED_FOR_EXECUTION
        m_boneIndices.begin(), m_boneIndices.end(),
        [lp, bones, &pose](uint16_t i)
        {
            const auto it = pose.find(bones[i].name);
            lp[i] = (it != pose.end()) ? it->second : bones[i].localBind;
        });

    // Step 2: world pass with world-space snaps — sequential
    m_worldPose[0] = lp[0];
    {
        const auto it = overridePose.find(bones[0].name);
        if (it != overridePose.end())
            m_worldPose[0] = lp[0] = it->second;
    }

    for (uint16_t i = 1; i < n; ++i)
    {
        const uint16_t p = bones[i].parentIdx;
        const auto it = overridePose.find(bones[i].name);
        if (it != overridePose.end()) [[unlikely]]
        {
            m_worldPose[i] = it->second;
            lp[i] = glm::inverse(m_worldPose[p]) * m_worldPose[i];
        }
        else [[likely]]
        {
            m_worldPose[i] = m_worldPose[p] * lp[i];
        }
    }

    // Step 3: skin pass — par_unseq
    glm::mat4*       boneMatrices = m_boneMatrices.data();
    const glm::mat4* worldPose    = m_worldPose;

    std::for_each(
        SUPPORTED_FOR_PAR_EXECUTION
        m_boneIndices.begin(), m_boneIndices.end(),
        [boneMatrices, worldPose, bones](uint16_t i)
        {
            const int16_t si = bones[i].skinIdx;
            if (si >= 0) [[likely]]
                boneMatrices[si] = worldPose[i] * bones[i].invBind;
        });

    if (skel.rootMotionBoneIdx != INVALID_BONE_IDX)
        rootBoneTransform = MathHelper::DecomposeMatrix(m_worldPose[skel.rootMotionBoneIdx]);

    m_localPoseDirty = true;
}

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
//
// m_boneIndices is filled with iota once and reused every frame.
// currentPose is pre-populated so buildCurrentPose never rehashes.
// ─────────────────────────────────────────────────────────────────────────────

Animator::Animator(SkinnedModel* model) : m_model(model)
{
    const uint16_t n = model->skeleton.boneCount;

    m_localPose = new glm::mat4[n];
    m_worldPose = new glm::mat4[n];

    for (uint16_t i = 0; i < n; ++i)
    {
        m_localPose[i] = model->skeleton.bones[i].localBind;
        m_worldPose[i] = glm::mat4(1.f);
    }

    m_boneIndices.resize(n);
    std::iota(m_boneIndices.begin(), m_boneIndices.end(), uint16_t{0});

    currentPose.reserve(n);
    for (uint16_t i = 0; i < n; ++i)
        currentPose[model->skeleton.bones[i].name] = model->skeleton.bones[i].localBind;

    m_boneMatrices.assign(MAX_SKINNED_BONES, glm::identity<glm::mat4>());
}

Animator::Animator(Animator&& o) noexcept
    : m_model              (o.m_model)
    , m_currClip           (o.m_currClip)
    , m_localPose          (o.m_localPose)
    , m_worldPose          (o.m_worldPose)
    , m_boneIndices        (std::move(o.m_boneIndices))
    , m_boneMatrices       (std::move(o.m_boneMatrices))
    , m_localPoseDirty     (o.m_localPoseDirty)
    , Loop                 (o.Loop)
    , m_currTime           (o.m_currTime)
    , m_playing            (o.m_playing)
    , InterpolateRotation  (o.InterpolateRotation)
    , InterpolatePosition  (o.InterpolatePosition)
    , InterpolateScale     (o.InterpolateScale)
    , oldRootBoneTransform (o.oldRootBoneTransform)
    , rootBoneTransform    (o.rootBoneTransform)
    , totalRootMotionPosition(o.totalRootMotionPosition)
    , totalRootMotionRotation(o.totalRootMotionRotation)
    , UpdatePose           (o.UpdatePose)
    , UsePrecomputedFrames (o.UsePrecomputedFrames)
    , m_currAnim           (o.m_currAnim)
    , currentAnimationName (std::move(o.currentAnimationName))
    , currentPose          (std::move(o.currentPose))
{
    o.m_model     = nullptr;
    o.m_localPose = nullptr;
    o.m_worldPose = nullptr;
    o.m_currClip  = nullptr;
    o.m_currAnim  = nullptr;
}

Animator& Animator::operator=(Animator&& o) noexcept
{
    if (this != &o)
    {
        delete[] m_localPose;
        delete[] m_worldPose;
        m_model            = o.m_model;             o.m_model      = nullptr;
        m_currClip         = o.m_currClip;          o.m_currClip   = nullptr;
        m_localPose        = o.m_localPose;         o.m_localPose  = nullptr;
        m_worldPose        = o.m_worldPose;         o.m_worldPose  = nullptr;
        m_boneIndices      = std::move(o.m_boneIndices);
        m_boneMatrices     = std::move(o.m_boneMatrices);
        m_localPoseDirty   = o.m_localPoseDirty;
        Loop               = o.Loop;
        m_currTime         = o.m_currTime;
        m_playing          = o.m_playing;
        InterpolateRotation  = o.InterpolateRotation;
        InterpolatePosition  = o.InterpolatePosition;
        InterpolateScale     = o.InterpolateScale;
        oldRootBoneTransform    = o.oldRootBoneTransform;
        rootBoneTransform       = o.rootBoneTransform;
        totalRootMotionPosition = o.totalRootMotionPosition;
        totalRootMotionRotation = o.totalRootMotionRotation;
        UpdatePose              = o.UpdatePose;
        UsePrecomputedFrames    = o.UsePrecomputedFrames;
        m_currAnim              = o.m_currAnim;     o.m_currAnim   = nullptr;
        currentAnimationName    = std::move(o.currentAnimationName);
        currentPose             = std::move(o.currentPose);
    }
    return *this;
}

Animator::~Animator()
{
    delete[] m_localPose;
    delete[] m_worldPose;
}

void Animator::set(const hashed_string& name)
{
    auto it = m_model->clips.find(name);
    if (it == m_model->clips.end()) return;
    m_currClip = it->second;
    m_currTime = 0.f;
    currentAnimationName = name.str();
    auto legacyIt = m_model->animations.find(name);
    if (legacyIt != m_model->animations.end())
        m_currAnim = &legacyIt->second;

    if (m_currAnim == nullptr)
        assert(false);

}

void Animator::play()  { m_playing = true; m_currTime = 0.f; }

void Animator::reset()
{
    m_currTime = 0.f;
    m_playing  = false;
    totalRootMotionPosition = glm::vec3(0.f);
    totalRootMotionRotation = glm::vec3(0.f);
    m_localPoseDirty = true;
}

void Animator::UpdateAnimationPose()
{
    if (m_currClip) evaluateClip(m_currTime);
}

std::vector<std::string> Animator::get()
{
    std::vector<std::string> names;
    names.reserve(m_model->clips.size());
    for (const auto& [k, _] : m_model->clips)
        names.emplace_back(k.str());
    return names;
}

std::vector<glm::mat4>& Animator::getBoneMatrices() { return m_boneMatrices; }

// Returned by const reference on purpose: currentPose is a persistent member,
// so returning it by value was an unconditional full hash-map copy (node
// alloc + relink + rehash) on every call, whether the caller needed to own
// a snapshot or was just going to read a value out of it. Callers that
// genuinely need to own an independent copy (e.g. to survive across later
// mutating calls) should copy explicitly at the call site.
const std::unordered_map<hashed_string, glm::mat4>& Animator::GetBonePoseArray()
{
    buildCurrentPose();
    return currentPose;
}

void Animator::PopulateBonePoseArray(BoneNode& node, glm::mat4,
                                     std::unordered_map<hashed_string, glm::mat4>& out)
{
    buildCurrentPose();
    const auto it = currentPose.find(node.name);
    out[node.name] = (it != currentPose.end()) ? it->second : glm::mat4(1.f);
    for (auto& child : node.children)
        PopulateBonePoseArray(child, glm::mat4(1.f), out);
}

} // namespace roj
