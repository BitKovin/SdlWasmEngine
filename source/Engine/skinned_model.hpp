#ifndef SKINNED_MODEL_HPP
#define SKINNED_MODEL_HPP

#include <glm.h>

#include "model.hpp"
#include <unordered_map>
#include <vector>
#include <array>
#include <span>
#include <limits>
#include "Texture.hpp"
#include "BoundingSphere.hpp"
#include "BoundingBox.hpp"
#include "utility/hashed_string.hpp"

#include <Helpers/Mesh/shadow_volume.hpp>

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

        bgfx::VertexBufferHandle vbh = BGFX_INVALID_HANDLE;
        bgfx::IndexBufferHandle  ibh = BGFX_INVALID_HANDLE;

        struct roj::ShadowVolumePrecomp shadowVolumePrecomp;

        bgfx::VertexLayout layout;

        std::vector<MeshTexture> textures;

        std::vector<VertexData> vertices;
        std::vector<uint32_t> indices;

        string materialName;
        string name;

        Texture* cachedBaseColor = nullptr;
        Texture* cachedEmissiveColor = nullptr;

        bool transparentTexture = false;

        void ProcessDefaultTextures();

        void DestroyBuffers()
        {
            // Destroy GPU buffers if valid
            if (bgfx::isValid(vbh))
            {
                bgfx::destroy(vbh);
                vbh = BGFX_INVALID_HANDLE;
            }

            if (bgfx::isValid(ibh))
            {
                bgfx::destroy(ibh);
                ibh = BGFX_INVALID_HANDLE;
            }

            shadowVolumePrecomp = ShadowVolumePrecomp();

            // Clear CPU-side vectors
            vertices.clear();
            vertices.shrink_to_fit();
            indices.clear();
            indices.shrink_to_fit();
            textures.clear();
            textures.shrink_to_fit();

            // Material and name cleanup
            materialName.clear();
            materialName.shrink_to_fit();
            name.clear();
            name.shrink_to_fit();
        }

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
// BoneBound
//
// Precomputed once at load time.  Stores the bind-pose model-space position of
// the vertex farthest from each skin bone's origin, weighted above a threshold.
//
// Why model-space position and not a local offset?
//   Skin matrices already encode  worldBoneTransform * invBind, so they map
//   any bind-pose model-space position directly to world space:
//
//     worldPos = worldMatrix * skinMatrix[s] * vec4(corner_of_bbMin_bbMax, 1)
//
//   Storing in model space means zero extra math at query time — just one
//   mat4 * vec4 per bone.
//
// Indexed by skinIdx (= BoneInfo::id = BlendIndices value).
// ─────────────────────────────────────────────────────────────────────────────

struct BoneBound
{
    // Bind-pose model-space AABB of all vertices influenced by this bone
    // above the weight threshold.  Both corners are needed: a single
    // "farthest" point drops all vertices that extend far in other directions
    // (e.g. a spine bone covers front AND back of the torso).
    //
    // At query time all 8 corners are transformed by the skin matrix so the
    // world AABB is always tight regardless of bone orientation or scale.
    glm::vec3 bbMin{ std::numeric_limits<float>::max()};
    glm::vec3 bbMax{-std::numeric_limits<float>::max()};
    bool      hasData = false;   // false → no skinned vertices above threshold
};

// ─────────────────────────────────────────────────────────────────────────────
// SkinnedModel
// ─────────────────────────────────────────────────────────────────────────────

struct SkinnedModel
{
    std::vector<SkinnedMesh> meshes;

    bool m_isOBJ = false;

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

    // ── Per-bone reach data for animated bounds queries ───────────────────────
    // Populated by PrecomputeBoneBounds() at load time.
    // Indexed by skinIdx (= BoneInfo::id).  Size == MAX_SKINNED_BONES.
    std::vector<BoneBound> boneBounds;

    // Call once after load, while mesh.vertices are still alive.
    // weightThreshold: influences below this value are ignored to avoid
    // near-zero weights pulling the bound to distant auxiliary bones.
    void PrecomputeBoneBounds(float weightThreshold = 0.05f);

    // O(B) animated bounding box.
    // skinMatrices: model-space skin matrix palette (bind-pose → model space),
    //               indexed by skinIdx.  Accepts span so callers can pass
    //               BakedFrame::skinMatrices or a runtime-evaluated array.
    // worldMatrix:  model → world transform of this instance.
    BoundingBox    ComputeAnimatedBounds(const glm::mat4&           worldMatrix,
                                        std::span<const glm::mat4> skinMatrices) const;

    // Derived sphere from the animated AABB — cheap second pass.
    BoundingSphere ComputeAnimatedSphere(const glm::mat4&           worldMatrix,
                                        std::span<const glm::mat4> skinMatrices) const;

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
