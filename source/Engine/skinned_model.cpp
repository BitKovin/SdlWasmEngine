#include "skinned_model.hpp"
#include "FileSystem/FileSystem.h"
#include "utils.hpp"
#include "Logger.hpp"
#include "Helpers/StringHelper.h"
#include "model_animator.hpp"

#include <Profiling/ResourceStatistics.hpp>

#include <Helpers/Mesh/shadow_volume.hpp>

#include <unordered_set>
#include <cassert>
#include <span>         // C++20

#define BAKED_FRAME_RATE 30.0f

using namespace utils::assimp;

// ─────────────────────────────────────────────────────────────────────────────
// File-local helpers (unchanged logic from original)
// ─────────────────────────────────────────────────────────────────────────────

static aiNode* findNode(const std::string& name, aiNode* node)
{
    if (node->mName.C_Str() == name) return node;
    for (unsigned i = 0; i < node->mNumChildren; ++i)
        if (aiNode* found = findNode(name, node->mChildren[i])) return found;
    return nullptr;
}

static void extractBoneVertexData(std::vector<VertexData>& vertices,
                                   aiMesh* mesh, roj::SkinnedModel& model)
{
    for (unsigned int i = 0; i < mesh->mNumBones; ++i)
    {
        int boneId = -1;
        hashed_string boneName = mesh->mBones[i]->mName.C_Str();
        if (model.boneInfoMap.find(boneName) == model.boneInfoMap.end())
        {
            model.boneInfoMap[boneName] = roj::BoneInfo{
                model.boneCount, toGlmMat4(mesh->mBones[i]->mOffsetMatrix) };
            boneId = model.boneCount++;
        }
        else boneId = model.boneInfoMap[boneName].id;

        auto weights = mesh->mBones[i]->mWeights;
        for (int j = 0; j < (int)mesh->mBones[i]->mNumWeights; ++j)
        {
            auto& vertex = vertices[weights[j].mVertexId];
            for (int k = 0; k < MAX_BONE_INFLUENCE; ++k)
            {
                if (vertex.BlendWeights[k] == 0.f)
                {
                    vertex.BlendWeights[k] = weights[j].mWeight;
                    vertex.BlendIndices[k] = boneId;
                    break;
                }
            }
        }
    }
}

static void extractBoneNode(roj::BoneNode& bone, aiNode* src)
{
    bone.name      = src->mName.data;
    bone.transform = toGlmMat4(src->mTransformation);
    for (unsigned i = 0; i < src->mNumChildren; ++i)
    {
        roj::BoneNode node;
        extractBoneNode(node, src->mChildren[i]);
        bone.children.push_back(node);
    }
}

static void extractBoneNodeAndBuildMap(roj::SkinnedModel& model,
                                        roj::BoneNode& bone, aiNode* src)
{
    bone.name      = src->mName.data;
    bone.transform = toGlmMat4(src->mTransformation);
    for (unsigned i = 0; i < src->mNumChildren; ++i)
    {
        roj::BoneNode node;
        extractBoneNodeAndBuildMap(model, node, src->mChildren[i]);
        bone.children.push_back(node);
        model.parentMap[node.name] = bone.name;
    }
    model.boneNodesMap[bone.name] = bone;
}

static void extractAnimations(const aiScene* scene, roj::SkinnedModel& model)
{
    std::unordered_set<hashed_string> additionalBones;

    for (unsigned int i = 0; i < scene->mNumAnimations; ++i)
    {
        aiAnimation* sceneAnim = scene->mAnimations[i];
        roj::Animation& anim = model.animations[sceneAnim->mName.C_Str()];
        extractBoneNode(anim.rootBone, scene->mRootNode);
        anim.ticksPerSec = sceneAnim->mTicksPerSecond;
        anim.duration    = sceneAnim->mDuration;

        if (sceneAnim->mNumChannels > 0)
        {
            const float dur = anim.duration / anim.ticksPerSec;
            const int kc = sceneAnim->mChannels[0]->mNumPositionKeys
                         + sceneAnim->mChannels[0]->mNumRotationKeys
                         + sceneAnim->mChannels[0]->mNumScalingKeys;
            anim.frameTime = kc / dur;
        }

        for (unsigned int j = 0; j < sceneAnim->mNumChannels; ++j)
        {
            aiNodeAnim* ch = sceneAnim->mChannels[j];
            hashed_string boneName = ch->mNodeName.C_Str();
            if (model.boneInfoMap.find(boneName) == model.boneInfoMap.end())
                additionalBones.insert(boneName);

            roj::FrameBoneTransform& track = anim.animationFrames[boneName];

            track.positionTimestamps.reserve(ch->mNumPositionKeys);
            track.positions.reserve(ch->mNumPositionKeys);
            for (unsigned k = 0; k < ch->mNumPositionKeys; ++k) {
                track.positionTimestamps.emplace_back((float)ch->mPositionKeys[k].mTime);
                track.positions.push_back(toGlmVec3(ch->mPositionKeys[k].mValue));
            }

            track.rotationTimestamps.reserve(ch->mNumRotationKeys);
            track.rotations.reserve(ch->mNumRotationKeys);
            for (unsigned k = 0; k < ch->mNumRotationKeys; ++k) {
                track.rotationTimestamps.emplace_back((float)ch->mRotationKeys[k].mTime);
                track.rotations.push_back(toGlmQuat(ch->mRotationKeys[k].mValue));
            }

            track.scaleTimestamps.reserve(ch->mNumScalingKeys);
            track.scales.reserve(ch->mNumScalingKeys);
            for (unsigned k = 0; k < ch->mNumScalingKeys; ++k) {
                track.scaleTimestamps.emplace_back((float)ch->mScalingKeys[k].mTime);
                track.scales.push_back(toGlmVec3(ch->mScalingKeys[k].mValue));
            }
        }
    }

    // Bones referenced by animation channels but absent from mesh skinning
    for (const auto& boneName : additionalBones)
    {
        aiNode* node = findNode(boneName.str(), scene->mRootNode);
        if (!node) continue;
        glm::mat4 g(1.f);
        for (aiNode* cur = node; cur; cur = cur->mParent)
            g = toGlmMat4(cur->mTransformation) * g;
        model.boneInfoMap[boneName] = roj::BoneInfo{ model.boneCount, glm::inverse(g) };
        model.boneCount++;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// buildFlatSkeleton
//
// Iterative DFS pre-order: parentIdx[i] < i for all i > 0.
// This invariant is what makes the world-transform forward pass correct with
// no branches and no guards — the parent is always already computed.
// ─────────────────────────────────────────────────────────────────────────────

static void buildFlatSkeleton(roj::FlatSkeleton& skel, aiNode* rootNode,
    const std::unordered_map<hashed_string, roj::BoneInfo>& boneInfoMap)
{
    struct Entry { aiNode* node; uint16_t parentIdx; };
    std::vector<Entry> entries;
    entries.reserve(256);

    std::vector<Entry> stack;
    stack.reserve(64);
    stack.push_back({ rootNode, INVALID_BONE_IDX });

    while (!stack.empty())
    {
        auto [node, parentIdx] = stack.back();
        stack.pop_back();
        const uint16_t myIdx = (uint16_t)entries.size();
        entries.push_back({ node, parentIdx });
        for (int i = (int)node->mNumChildren - 1; i >= 0; --i)
            stack.push_back({ node->mChildren[i], myIdx });
    }

    skel.boneCount = (uint16_t)entries.size();
    skel.bones     = new roj::SkeletonBone[skel.boneCount];

    for (uint16_t i = 0; i < skel.boneCount; ++i)
    {
        auto& e    = entries[i];
        auto& bone = skel.bones[i];
        bone.name      = e.node->mName.C_Str();
        bone.parentIdx = e.parentIdx;
        bone.localBind = toGlmMat4(e.node->mTransformation);
        auto it = boneInfoMap.find(bone.name);
        if (it != boneInfoMap.end()) {
            bone.skinIdx = (int16_t)it->second.id;
            bone.invBind = it->second.offset;
        } else {
            bone.skinIdx = -1;
            bone.invBind = glm::mat4(1.f);
        }
        skel.nameToIdx[bone.name] = i;
        if (bone.name == hashed_string("root")) skel.rootMotionBoneIdx = i;
    }

#ifdef _DEBUG
    for (uint16_t i = 1; i < skel.boneCount; ++i)
        assert(skel.bones[i].parentIdx < i && "Skeleton not in topological order");
#endif
}

// ─────────────────────────────────────────────────────────────────────────────
// buildEvaluatableClip
//
// BoneTrack fields are std::span<const T> — constructed from {ptr, count}.
// Zero overhead vs raw pointer + count, but the compiler sees contiguity
// explicitly, which improves alias analysis inside sampleBone<TLoop>.
//
// Two-pass build: size first, allocate once, fill with memcpy.
// ─────────────────────────────────────────────────────────────────────────────

static roj::EvaluatableClip* buildEvaluatableClip(
    const roj::FlatSkeleton& skel,
    const roj::Animation&    anim,
    const std::string&       name)
{
    auto* clip        = new roj::EvaluatableClip();
    clip->name        = name;
    clip->duration    = anim.duration;
    clip->ticksPerSec = anim.ticksPerSec;
    clip->boneCount   = skel.boneCount;
    clip->tracks      = new roj::BoneTrack[skel.boneCount];

    // Pass 1: count total storage needed
    size_t totalTimes = 0, totalVec3s = 0, totalQuats = 0;
    for (uint16_t i = 0; i < skel.boneCount; ++i)
    {
        auto it = anim.animationFrames.find(skel.bones[i].name);
        if (it != anim.animationFrames.end()) {
            const auto& f = it->second;
            totalTimes += f.positionTimestamps.size() + f.rotationTimestamps.size() + f.scaleTimestamps.size();
            totalVec3s += f.positions.size() + f.scales.size();
            totalQuats += f.rotations.size();
        } else {
            totalTimes += 3; totalVec3s += 2; totalQuats += 1;  // single bind-pose keyframe
        }
    }

    // Single allocation for all keyframe data in this clip
    clip->storage.times.resize(totalTimes);
    clip->storage.vec3s.resize(totalVec3s);
    clip->storage.quats.resize(totalQuats);

    float*      tb = clip->storage.times.data();
    glm::vec3*  vb = clip->storage.vec3s.data();
    glm::quat*  qb = clip->storage.quats.data();
    size_t tOff = 0, vOff = 0, qOff = 0;

    // Pass 2: copy data and construct spans pointing into storage
    for (uint16_t i = 0; i < skel.boneCount; ++i)
    {
        roj::BoneTrack& track = clip->tracks[i];
        auto it = anim.animationFrames.find(skel.bones[i].name);

        if (it != anim.animationFrames.end())
        {
            const auto& f = it->second;

            // span{ptr, count} — same cost as storing a raw pointer + uint16_t,
            // but carries the contiguity guarantee that enables better codegen.
            std::memcpy(tb + tOff, f.positionTimestamps.data(), f.positionTimestamps.size() * sizeof(float));
            std::memcpy(vb + vOff, f.positions.data(),          f.positions.size()          * sizeof(glm::vec3));
            track.posTimes  = { tb + tOff, f.positionTimestamps.size() };
            track.posValues = { vb + vOff, f.positions.size() };
            tOff += f.positionTimestamps.size();
            vOff += f.positions.size();

            std::memcpy(tb + tOff, f.rotationTimestamps.data(), f.rotationTimestamps.size() * sizeof(float));
            std::memcpy(qb + qOff, f.rotations.data(),          f.rotations.size()          * sizeof(glm::quat));
            track.rotTimes  = { tb + tOff, f.rotationTimestamps.size() };
            track.rotValues = { qb + qOff, f.rotations.size() };
            tOff += f.rotationTimestamps.size();
            qOff += f.rotations.size();

            std::memcpy(tb + tOff, f.scaleTimestamps.data(), f.scaleTimestamps.size() * sizeof(float));
            std::memcpy(vb + vOff, f.scales.data(),          f.scales.size()          * sizeof(glm::vec3));
            track.scaleTimes  = { tb + tOff, f.scaleTimestamps.size() };
            track.scaleValues = { vb + vOff, f.scales.size() };
            tOff += f.scaleTimestamps.size();
            vOff += f.scales.size();
        }
        else
        {
            // No animation channel for this bone: constant bind-pose keyframe.
            // sampleBone will always hit the size()==1 [[likely]] branch — zero
            // binary-search work for unanimated bones.
            const auto bp = MathHelper::DecomposeMatrix(skel.bones[i].localBind);

            tb[tOff] = 0.f; vb[vOff] = bp.Position;
            track.posTimes  = { tb + tOff, 1 }; track.posValues = { vb + vOff, 1 };
            ++tOff; ++vOff;

            tb[tOff] = 0.f; qb[qOff] = bp.RotationQuaternion;
            track.rotTimes  = { tb + tOff, 1 }; track.rotValues = { qb + qOff, 1 };
            ++tOff; ++qOff;

            tb[tOff] = 0.f; vb[vOff] = bp.Scale;
            track.scaleTimes  = { tb + tOff, 1 }; track.scaleValues = { vb + vOff, 1 };
            ++tOff; ++vOff;
        }
    }

    assert(tOff == totalTimes);
    assert(vOff == totalVec3s);
    assert(qOff == totalQuats);
    return clip;
}

// ─────────────────────────────────────────────────────────────────────────────
// bakeClipFrames — runs at load time only, not on the hot path
// ─────────────────────────────────────────────────────────────────────────────

static void bakeClipFrames(roj::EvaluatableClip& clip, roj::SkinnedModel& model)
{
    const float frameInterval = 1.f / BAKED_FRAME_RATE;
    clip.bakedFrameInterval   = frameInterval;

    roj::Animator animator(&model);
    animator.set(hashed_string(clip.name.c_str()));
    animator.play();
    animator.update(0.f);

    const uint16_t n = model.skeleton.boneCount;

    auto captureFrame = [&]() -> roj::EvaluatableClip::BakedFrame
    {
        roj::EvaluatableClip::BakedFrame bf;
        bf.rootMotionPos = animator.totalRootMotionPosition;
        bf.rootMotionRot = animator.totalRootMotionRotation;

        const auto& mats = animator.getBoneMatrices();
        std::memcpy(bf.skinMatrices.data(), mats.data(),
                    std::min(mats.size(), (size_t)MAX_SKINNED_BONES) * sizeof(glm::mat4));

        auto pose = animator.GetBonePoseArray();
        bf.localPoses.resize(n);
        for (uint16_t i = 0; i < n; ++i)
        {
            auto it = pose.find(model.skeleton.bones[i].name);
            bf.localPoses[i] = (it != pose.end()) ? it->second : model.skeleton.bones[i].localBind;
        }
        return bf;
    };

    clip.bakedFrames.push_back(captureFrame());
    while (animator.m_playing) {
        animator.update(frameInterval);
        clip.bakedFrames.push_back(captureFrame());
    }
    animator.reset();
}

// ─────────────────────────────────────────────────────────────────────────────
// SkinnedModel
// ─────────────────────────────────────────────────────────────────────────────

namespace roj
{

template class ModelLoader<SkinnedMesh>;

SkinnedModel::~SkinnedModel()
{
    for (auto& [_, clip] : clips) delete clip;
}

// ─────────────────────────────────────────────────────────────────────────────
// PrecomputeBoneBounds
//
// Expands a per-bone AABB (in bind-pose MODEL space) over every vertex whose
// weight for that bone exceeds weightThreshold.
//
// Storing the full AABB — not just the single farthest point — is essential:
// a spine bone influences vertices in front AND behind the torso; discarding
// all but the farthest in one direction silently drops the opposite extreme.
//
// No glm::inverse() needed here; we no longer need the bone's world origin.
// BlendWeights is glm::vec4; glm::value_ptr gives float* for uniform k-indexing.
// ─────────────────────────────────────────────────────────────────────────────

void SkinnedModel::PrecomputeBoneBounds(float weightThreshold)
{
    // ── Mark which skinIdx slots have a live skin bone ────────────────────────
    std::vector<bool> isSkinBone(MAX_SKINNED_BONES, false);
    for (uint16_t i = 0; i < skeleton.boneCount; ++i)
    {
        const SkeletonBone& sb = skeleton.bones[i];
        if (sb.skinIdx >= 0 && sb.skinIdx < MAX_SKINNED_BONES)
            isSkinBone[sb.skinIdx] = true;
    }

    boneBounds.assign(MAX_SKINNED_BONES, BoneBound{});   // reset to sentinel min/max

    // ── Expand each bone's AABB with every vertex it influences ──────────────
    for (const auto& mesh : meshes)
    {
        for (const auto& v : mesh.vertices)
        {
            const float* w = glm::value_ptr(v.BlendWeights);   // x,y,z,w → [0..3]

            for (int k = 0; k < MAX_BONE_INFLUENCE; ++k)
            {
                if (w[k] < weightThreshold) continue;

                const int s = v.BlendIndices[k];
                if (s < 0 || s >= MAX_SKINNED_BONES) continue;
                if (!isSkinBone[s]) continue;

                BoneBound& bb = boneBounds[s];
                bb.bbMin   = glm::min(bb.bbMin, v.Position);
                bb.bbMax   = glm::max(bb.bbMax, v.Position);
                bb.hasData = true;
            }
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// expandAABBWithBoneBound  —  file-local helper
//
// Transforms all 8 corners of bone b's bind-pose AABB by M = worldMatrix *
// skinMatrix[s] and expands (bmin, bmax) with each result.
//
// Why 8 corners?  After an arbitrary rotation+scale the axis-aligned extremes
// of the transformed box are not just the two transformed corner points — any
// of the 8 corners can be extreme along any world axis.
// ─────────────────────────────────────────────────────────────────────────────

static inline void expandAABBWithBoneBound(
    glm::vec3&        bmin,
    glm::vec3&        bmax,
    const glm::mat4&  M,
    const BoneBound&  b)
{
    const glm::vec3& lo = b.bbMin;
    const glm::vec3& hi = b.bbMax;

    for (int corner = 0; corner < 8; ++corner)
    {
        const glm::vec3 c{
            (corner & 1) ? hi.x : lo.x,
            (corner & 2) ? hi.y : lo.y,
            (corner & 4) ? hi.z : lo.z
        };
        const glm::vec3 wp = glm::vec3(M * glm::vec4(c, 1.f));
        bmin = glm::min(bmin, wp);
        bmax = glm::max(bmax, wp);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// ComputeAnimatedBounds  —  O(B * 8) per frame
//
//   worldPos = worldMatrix * skinMatrix[s] * vec4(corner, 1)
//
// skinMatrix[s]: model-space skin matrix (bind-pose → model space).
// worldMatrix  : model → world.
// Falls back to the static boundingBox if no bone data is available.
// ─────────────────────────────────────────────────────────────────────────────

BoundingBox SkinnedModel::ComputeAnimatedBounds(
    const glm::mat4&            worldMatrix,
    std::span<const glm::mat4>  skinMatrices) const
{
    if (boneBounds.empty()) return boundingBox;

    glm::vec3 bmin( std::numeric_limits<float>::max());
    glm::vec3 bmax(-std::numeric_limits<float>::max());
    bool      any = false;

    const int count = static_cast<int>(
        std::min(skinMatrices.size(), boneBounds.size()));

    for (int s = 0; s < count; ++s)
    {
        if (!boneBounds[s].hasData) continue;
        expandAABBWithBoneBound(bmin, bmax, worldMatrix * skinMatrices[s], boneBounds[s]);
        any = true;
    }

    if (!any) return boundingBox;

    const std::vector<glm::vec3> corners{ bmin, bmax };
    return BoundingBox::FromPoints(corners);
}

// ─────────────────────────────────────────────────────────────────────────────
// ComputeAnimatedSphere  —  derived from the animated AABB (same pass)
// Center = AABB midpoint; radius = half-diagonal (sphere contains the AABB).
// ─────────────────────────────────────────────────────────────────────────────

BoundingSphere SkinnedModel::ComputeAnimatedSphere(
    const glm::mat4&            worldMatrix,
    std::span<const glm::mat4>  skinMatrices) const
{
    if (boneBounds.empty()) return boundingSphere;

    glm::vec3 bmin( std::numeric_limits<float>::max());
    glm::vec3 bmax(-std::numeric_limits<float>::max());
    bool      any = false;

    const int count = static_cast<int>(
        std::min(skinMatrices.size(), boneBounds.size()));

    for (int s = 0; s < count; ++s)
    {
        if (!boneBounds[s].hasData) continue;
        expandAABBWithBoneBound(bmin, bmax, worldMatrix * skinMatrices[s], boneBounds[s]);
        any = true;
    }

    if (!any) return boundingSphere;

    const glm::vec3 center = (bmin + bmax) * 0.5f;
    return BoundingSphere{ center, glm::length(bmax - center) };
}


void SkinnedModel::clear()
{
    for (auto& mesh : meshes) mesh.DestroyBuffers();
    meshes.clear();
    boneInfoMap.clear();
    for (auto& [_, clip] : clips) delete clip;
    clips.clear();
}

template<>
std::vector<VertexData> ModelLoader<SkinnedMesh>::getMeshVertices(aiMesh* mesh)
{
    std::vector<VertexData> vertices;
    vertices.reserve(mesh->mNumVertices);
    for (uint32_t i = 0; i < mesh->mNumVertices; ++i)
    {
        VertexData v;
        v.Position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z)
                     * LoaderGlobalParams::Size;
        if (mesh->HasNormals())
            v.Normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };
        if (mesh->mTextureCoords[0])
            v.TextureCoordinate = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
        if (mesh->mTextureCoords[1])
            v.TextureCoordinate2 = { mesh->mTextureCoords[1][i].x, mesh->mTextureCoords[1][i].y };
        v.Tangent   = { mesh->mTangents[i].x,   mesh->mTangents[i].y,   mesh->mTangents[i].z };
        v.BiTangent = { mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z };
        int cc = -1;
        for (unsigned c = 0; c < mesh->GetNumColorChannels(); ++c)
            if (mesh->mColors[c]) { cc = c; break; }
        if (cc >= 0)
            v.Color = { mesh->mColors[cc][i].r, mesh->mColors[cc][i].g,
                        mesh->mColors[cc][i].b, mesh->mColors[cc][i].a };

        std::fill(v.BlendIndices, v.BlendIndices + MAX_BONE_INFLUENCE, 0);

        v.BlendWeights = glm::vec4(0.f);
        vertexPositions.push_back(v.Position);
        vertices.push_back(v);
    }
    return vertices;
}

template<>
SkinnedMesh ModelLoader<SkinnedMesh>::processMesh(aiMesh* mesh, const aiScene* scene)
{
    std::vector<VertexData> vertices = getMeshVertices(mesh);
    std::vector<uint32_t>   indices;
    indices.reserve(mesh->mNumFaces * 3);
    for (uint32_t i = 0; i < mesh->mNumFaces; ++i)
    {
        aiFace& face = mesh->mFaces[i];
        indices.insert(indices.end(), face.mIndices, face.mIndices + face.mNumIndices);
    }
    extractBoneVertexData(vertices, mesh, m_model);

    SkinnedMesh skinMesh = SkinnedMesh();

    skinMesh.name = mesh->mName.C_Str();

    std::vector<MeshTexture> textures = getMeshTextures(scene->mMaterials[mesh->mMaterialIndex], scene);

    // OBJ files rarely embed texture paths; fall back to <materialName>.png as diffuse.
    if (m_model.m_isOBJ)
    {
        const bool hasDiffuse = std::any_of(textures.begin(), textures.end(),
            [](const MeshTexture& t) {
                return t.type == aiTextureType_BASE_COLOR || t.type == aiTextureType_DIFFUSE;
            });

        if (!hasDiffuse)
        {
            const std::string matName = scene->mMaterials[mesh->mMaterialIndex]->GetName().C_Str();
            MeshTexture fallback;
            fallback.type = aiTextureType_BASE_COLOR;
            fallback.src = matName + ".png";
            textures.insert(textures.begin(), fallback);   // insert first so ProcessDefaultTextures sees it
        }
    }

    skinMesh.materialName = scene->mMaterials[mesh->mMaterialIndex]->GetName().C_Str();

    skinMesh.textures = textures;

    skinMesh.indices = indices;
    skinMesh.vertices = vertices;

    skinMesh.layout = VertexData::Declaration();

    if (!DeferGPUUpload)
        UploadSkinnedMeshGPUBuffers(skinMesh);

    return skinMesh;
}

void UploadSkinnedMeshGPUBuffers(SkinnedMesh& skinMesh)
{
    const bgfx::Memory* vbMem = bgfx::copy(skinMesh.vertices.data(), sizeof(VertexData) * skinMesh.vertices.size());
    skinMesh.vbh = bgfx::createVertexBuffer(vbMem, skinMesh.layout);

    if(bgfx::isValid(skinMesh.vbh) == false)
    {
		throw std::runtime_error("Failed to create vertex buffer for mesh: " + skinMesh.name);
    }

    ResourceStatistics::Instance().registerResource(
        ResourceType::VertexBuffer, skinMesh.vbh.idx,
        sizeof(VertexData) * skinMesh.vertices.size(),
        skinMesh.name.empty() ? "SkinnedMesh VB" : (skinMesh.name + " (VB)"));

    const bgfx::Memory* ibMem = bgfx::copy(skinMesh.indices.data(), sizeof(uint32_t) * skinMesh.indices.size());
    skinMesh.ibh = bgfx::createIndexBuffer(ibMem, BGFX_BUFFER_INDEX32);

    if (bgfx::isValid(skinMesh.ibh))
    {
        ResourceStatistics::Instance().registerResource(
            ResourceType::IndexBuffer, skinMesh.ibh.idx,
            sizeof(uint32_t) * skinMesh.indices.size(),
            skinMesh.name.empty() ? "SkinnedMesh IB" : (skinMesh.name + " (IB)"));
    }

    skinMesh.shadowVolumePrecomp = BuildShadowVolumePrecomp(skinMesh);
}

template<>
void ModelLoader<SkinnedMesh>::processNodeVertices(aiNode* node, const aiScene* scene)
{
    for (uint32_t i = 0; i < node->mNumChildren; ++i)
        processNodeVertices(node->mChildren[i], scene);
}

template<>
void ModelLoader<SkinnedMesh>::processNode(aiNode* node, const aiScene* scene)
{
    const std::string name = node->mName.C_Str();
    if (!SkipVisual)
    {
        if (LoaderGlobalParams::MeshNameLimit.empty() || name == LoaderGlobalParams::MeshNameLimit)
            for (uint32_t i = 0; i < node->mNumMeshes; ++i)
                m_model.meshes.push_back(processMesh(scene->mMeshes[node->mMeshes[i]], scene));
    }
    else
    {
        if (LoaderGlobalParams::MeshNameLimit.empty() || name == LoaderGlobalParams::MeshNameLimit)
        {
            for (uint32_t i = 0; i < node->mNumMeshes; ++i)
            {
                aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
                for (unsigned b = 0; b < mesh->mNumBones; ++b)
                {
                    const std::string bn = mesh->mBones[b]->mName.C_Str();
                    if (m_model.boneInfoMap.find(bn) == m_model.boneInfoMap.end())
                    {
                        m_model.boneInfoMap[bn] = roj::BoneInfo{
                            m_model.boneCount, toGlmMat4(mesh->mBones[b]->mOffsetMatrix) };
                        m_model.boneCount++;
                    }
                }
            }
        }
    }
    for (uint32_t i = 0; i < node->mNumChildren; ++i)
        processNode(node->mChildren[i], scene);
}

template<>
bool ModelLoader<SkinnedMesh>::load(const std::string& path)
{
    resetLoader();
    if (path.empty()) { m_infoLog += "Empty path.\n"; return false; }

    const aiScene* m_scene = nullptr;
    if (path == m_lastLoadedPath && m_cachedScene)
    {
        m_scene = m_cachedScene;
    }
    else
    {

        if (SkipVisual)
        {
            // 1. Define which components to completely discard during import
            unsigned int removeFlags = aiComponent_MATERIALS |
                aiComponent_TEXTURES |
                aiComponent_LIGHTS |
                aiComponent_CAMERAS |
                aiComponent_COLORS |
                aiComponent_TEXCOORDS |
                aiComponent_NORMALS |
                aiComponent_TANGENTS_AND_BITANGENTS;

            // 2. Pass the configuration to the importer
            m_import.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, removeFlags);
        }



        std::vector<uint8_t> fileData = FileSystemEngine::ReadFileBinary(path);
        if (fileData.empty()) { m_infoLog += "Failed to read: " + path + "\n"; return false; }
        m_scene = m_import.ReadFileFromMemory(fileData.data(), fileData.size(),
            aiProcess_Triangulate | aiProcess_FlipUVs |
            aiProcess_CalcTangentSpace | aiProcess_LimitBoneWeights |
            aiProcess_JoinIdenticalVertices | aiProcess_GlobalScale, path.c_str());
        m_cachedScene    = m_scene;
        m_lastLoadedPath = path;
    }

    // after: m_relativeDir = "GameData/";
    m_model.m_isOBJ = path.size() >= 4 &&
        StringHelper::ToLower(path.substr(path.size() - 4)) == ".obj";

    if (!m_scene || (m_scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !m_scene->mRootNode)
    {
        m_infoLog += m_import.GetErrorString();
        return false;
    }

    m_relativeDir          = "GameData/";
    m_model.globalInversed = glm::inverse(toGlmMat4(m_scene->mRootNode->mTransformation));
    m_model.sceneCamera    = m_scene->HasCameras() ? m_scene->mCameras[0] : nullptr;

    // Step 1: build meshes + populate boneInfoMap
    processNode(m_scene->mRootNode, m_scene);


    // Step 2: extract animations into legacy map (for m_currAnim->duration etc.)
    extractAnimations(m_scene, m_model);

    // Step 3: bounding volumes
    m_model.boundingSphere = BoundingSphere::FromPoints(vertexPositions);
    m_model.boundingBox    = BoundingBox::FromPoints(vertexPositions);
    if (m_model.boneCount > 0)
    {
        m_model.boundingSphere.Radius *= 2.5f;
        m_model.boundingSphere.offset *= 1.5f;
        extractBoneNodeAndBuildMap(m_model, m_model.defaultRoot, m_scene->mRootNode);
    }

    // Step 4: build FlatSkeleton — after boneInfoMap is fully populated
    buildFlatSkeleton(m_model.skeleton, m_scene->mRootNode, m_model.boneInfoMap);

    // Step 4.5: pre-bake per-bone reach data for animated bounds queries.
    // Must run after the skeleton (needs skinIdx + invBind) and while
    // mesh.vertices are still populated (before any DestroyBuffers call).
    if (m_model.boneCount > 0)
        m_model.PrecomputeBoneBounds();

    // Step 5: build EvaluatableClips — one per animation, per model instance
    // Cross-model: each model gets its own clips built from its own skeleton,
    // so two models sharing animation names but different bone sets each get
    // correct independent clips with bind-pose defaults for missing bones.
    for (auto& [animName, anim] : m_model.animations)
        m_model.clips[animName] = buildEvaluatableClip(m_model.skeleton, anim, animName.str());

    // Step 6: bake frames for UsePrecomputedFrames mode
    for (auto& [_, clip] : m_model.clips)
        bakeClipFrames(*clip, m_model);

    // Step 7: texture processing + opaque-before-transparent sort
    for (auto& mesh : m_model.meshes)
        mesh.ProcessDefaultTextures();
    std::sort(m_model.meshes.begin(), m_model.meshes.end(),
        [](const SkinnedMesh& a, const SkinnedMesh& b) {
            return !a.transparentTexture && b.transparentTexture; });
    
    if (SkipVisual == false)
    {
        for (size_t i = 0; i < m_scene->mNumTextures; ++i)
            LoadTextureFromScene(m_scene->mTextures[i]);
    }


    PrewarmSkeletonTopology(&m_model.defaultRoot);

    return true;
}

void SkinnedMesh::ProcessDefaultTextures()
{
    for (const auto& tex : textures)
    {
        if (tex.type == aiTextureType_BASE_COLOR)
        {
            if (StringHelper::Contains(tex.src, "_t.") || StringHelper::Contains(tex.src, "_m."))
                transparentTexture = true;
            break;
        }
    }
}

} // namespace roj
