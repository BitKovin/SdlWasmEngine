#ifndef SKINNED_MODEL_HPP
#define SKINNED_MODEL_HPP

#include <glm.h>

#include "model.hpp"
#include <unordered_map>
#include <vector>
#include <array>
#include <span>       // C++20
#include "Texture.hpp"
#include "BoundingSphere.hpp"
#include "BoundingBox.hpp"
#include "utility/hashed_string.hpp"

#define MAX_BONE_INFLUENCE  4
#define MAX_SKINNED_BONES   128

constexpr uint16_t INVALID_BONE_IDX = 0xFFFFu;

namespace roj
{

// ─────────────────────────────────────────────────────────────────────────────
// SkinnedMesh (unchanged)
// ─────────────────────────────────────────────────────────────────────────────

struct SkinnedMesh
{
    VertexBuffer*      vertices  = nullptr;
    IndexBuffer*       indices   = nullptr;
    VertexArrayObject* VAO       = nullptr;

    std::vector<MeshTexture>  textures;
    std::vector<VertexData>   vertexLocations;
    std::vector<uint32_t>     vertexIndices;

    std::string materialName;
    std::string name;

    Texture* cachedBaseColor     = nullptr;
    Texture* cachedEmissiveColor = nullptr;
    bool     transparentTexture  = false;

    void ProcessDefaultTextures();
    void DestroyBuffers();
};

// ─────────────────────────────────────────────────────────────────────────────
// Legacy structures — kept for SkeletalMesh.cpp compatibility only.
// Not touched on the hot animation evaluation path.
// ─────────────────────────────────────────────────────────────────────────────

struct BoneNode
{
    hashed_string         name;
    glm::mat4             transform;
    std::vector<BoneNode> children;
    uint16_t              id = 0;
};

struct FrameBoneTransform
{
    std::vector<float>      positionTimestamps;
    std::vector<float>      rotationTimestamps;
    std::vector<float>      scaleTimestamps;
    std::vector<glm::vec3>  positions;
    std::vector<glm::quat>  rotations;
    std::vector<glm::vec3>  scales;
};

struct BakedFrameData
{
    std::unordered_map<hashed_string, glm::mat4> boneTransforms;
    glm::vec3              totalRootMotionPosition{};
    glm::vec3              totalRootMotionRotation{};
    std::vector<glm::mat4> modelTransform;
};

struct Animation
{
    float    duration    = 0.f;
    float    ticksPerSec = 1.f;
    float    frameTime   = 0.f;
    BoneNode rootBone;
    std::unordered_map<hashed_string, FrameBoneTransform> animationFrames;
    std::vector<BakedFrameData> bakedFrames;
    float    bakedFrameInterval = 0.f;
};

struct BoneInfo
{
    int       id;
    glm::mat4 offset;
};

// ─────────────────────────────────────────────────────────────────────────────
// FlatSkeleton
//
// Bones in DFS pre-order: parentIdx[i] < i for all i > 0.
// This guarantees the world-transform forward pass is always correct with
// no branches and no extra checks.
// ─────────────────────────────────────────────────────────────────────────────

struct SkeletonBone
{
    hashed_string name;
    uint16_t      parentIdx;  // INVALID_BONE_IDX for root
    int16_t       skinIdx;    // GPU bone-matrix slot; -1 = not a skin bone
    glm::mat4     localBind;  // local transform when no animation channel exists
    glm::mat4     invBind;    // inverse bind-pose (= boneInfoMap[name].offset)
};

struct FlatSkeleton
{
    SkeletonBone* bones             = nullptr;
    uint16_t      boneCount         = 0;
    uint16_t      rootMotionBoneIdx = INVALID_BONE_IDX;

    // Setup-time only. Never accessed inside evaluateClip().
    std::unordered_map<hashed_string, uint16_t> nameToIdx;

    FlatSkeleton() = default;
    FlatSkeleton(const FlatSkeleton&) = delete;
    FlatSkeleton& operator=(const FlatSkeleton&) = delete;
    FlatSkeleton(FlatSkeleton&& o) noexcept
        : bones(o.bones), boneCount(o.boneCount),
          rootMotionBoneIdx(o.rootMotionBoneIdx),
          nameToIdx(std::move(o.nameToIdx))
    { o.bones = nullptr; o.boneCount = 0; }
    FlatSkeleton& operator=(FlatSkeleton&& o) noexcept
    {
        if (this != &o) {
            delete[] bones;
            bones = o.bones;                        o.bones     = nullptr;
            boneCount = o.boneCount;                o.boneCount = 0;
            rootMotionBoneIdx = o.rootMotionBoneIdx;
            nameToIdx = std::move(o.nameToIdx);
        }
        return *this;
    }
    ~FlatSkeleton() { delete[] bones; }

    uint16_t findBone(const hashed_string& name) const
    {
        auto it = nameToIdx.find(name);
        return (it != nameToIdx.end()) ? it->second : INVALID_BONE_IDX;
    }
};

// ─────────────────────────────────────────────────────────────────────────────
// BoneTrack
//
// Per-bone view into ClipStorage's contiguous arrays.
//
// std::span<const T> replaces the previous (T*, uint16_t) pairs for two
// concrete reasons:
//
//   1. The compiler sees pointer and size together as a unit.  With separate
//      fields it has to prove they describe the same allocation; with span
//      that's a given.  This unlocks auto-vectorisation of std::lower_bound
//      and the interpolation lerp/slerp calls.
//
//   2. span knows the memory is contiguous — a guarantee raw pointers alone
//      don't carry.  Execution policies (par_unseq) in the caller can
//      exploit this for SIMD reads of the timestamp arrays.
//
// Non-owning: all data lives in ClipStorage. Constructing a span is just
// {ptr, count} — zero overhead vs raw pointer + count.
// ─────────────────────────────────────────────────────────────────────────────

struct BoneTrack
{
    std::span<const float>      posTimes;
    std::span<const glm::vec3>  posValues;

    std::span<const float>      rotTimes;
    std::span<const glm::quat>  rotValues;

    std::span<const float>      scaleTimes;
    std::span<const glm::vec3>  scaleValues;
};

// Owns the data all BoneTracks in one clip point into.
// Allocated exactly once at load time; never reallocated.
struct ClipStorage
{
    std::vector<float>      times;  // all timestamp channels packed end-to-end
    std::vector<glm::vec3>  vec3s;  // pos then scale values, per bone
    std::vector<glm::quat>  quats;  // rot values, per bone
};

// ─────────────────────────────────────────────────────────────────────────────
// EvaluatableClip
// ─────────────────────────────────────────────────────────────────────────────

struct EvaluatableClip
{
    std::string  name;
    float        duration     = 0.f;
    float        ticksPerSec  = 1.f;
    uint16_t     boneCount    = 0;

    BoneTrack*  tracks  = nullptr;  // [boneCount], spans into storage
    ClipStorage storage;

    float bakedFrameInterval = 0.f;
    struct BakedFrame
    {
        std::array<glm::mat4, MAX_SKINNED_BONES> skinMatrices;
        std::vector<glm::mat4>                   localPoses;
        glm::vec3 rootMotionPos{};
        glm::vec3 rootMotionRot{};
    };
    std::vector<BakedFrame> bakedFrames;

    EvaluatableClip() = default;
    EvaluatableClip(const EvaluatableClip&) = delete;
    EvaluatableClip& operator=(const EvaluatableClip&) = delete;
    ~EvaluatableClip() { delete[] tracks; }
};

// ─────────────────────────────────────────────────────────────────────────────
// SkinnedModel
// ─────────────────────────────────────────────────────────────────────────────

struct SkinnedModel
{
    std::vector<SkinnedMesh> meshes;

    // ── Hot-path ─────────────────────────────────────────────────────────────
    FlatSkeleton skeleton;
    std::unordered_map<hashed_string, EvaluatableClip*> clips;

    // ── Legacy — SkeletalMesh.cpp reads these directly ────────────────────────
    int                                               boneCount    { 0 };
    aiCamera*                                         sceneCamera  = nullptr;
    glm::mat4                                         globalInversed { 1.f };
    std::unordered_map<hashed_string, BoneInfo>       boneInfoMap;
    std::unordered_map<hashed_string, Animation>      animations;
    BoneNode                                          defaultRoot;
    std::unordered_map<hashed_string, BoneNode>       boneNodesMap;
    std::unordered_map<hashed_string, hashed_string>  parentMap;

    BoundingSphere boundingSphere;
    BoundingBox    boundingBox;

    std::vector<SkinnedMesh>::iterator begin() { return meshes.begin(); }
    std::vector<SkinnedMesh>::iterator end()   { return meshes.end();   }
    void clear();

    SkinnedModel() = default;
    SkinnedModel(SkinnedModel&&) = default;
    SkinnedModel& operator=(SkinnedModel&&) = default;

    ~SkinnedModel();
};

} // namespace roj
#endif // SKINNED_MODEL_HPP
