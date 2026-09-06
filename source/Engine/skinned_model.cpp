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
#include <execution>    // std::execution::par - BakeAllClipsParallel
#ifndef GLM_ENABLE_EXPERIMENTAL
#define GLM_ENABLE_EXPERIMENTAL
#endif
#include <glm/gtx/matrix_decompose.hpp> // glm::decompose - backfilling missing glTF TRS channels
#include <numeric>      // std::iota - BakeAllClipsParallel
#include <glm/gtc/type_ptr.hpp> // glm::make_mat4 - reading cgltf's column-major float[16]

// Fast, bones-and-animations-only glTF/GLB reader for the Logic tier - see
// LoadLogicTierFromGLTF below for why this exists instead of just running
// assimp with SkipVisual again. Single-header library, vendored the same
// way stb_image is for Texture.hpp.
#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

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
    bone.name = src->mName.data;
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
    bone.name = src->mName.data;
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
        anim.duration = sceneAnim->mDuration;

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
    skel.bones = new roj::SkeletonBone[skel.boneCount];

    for (uint16_t i = 0; i < skel.boneCount; ++i)
    {
        auto& e = entries[i];
        auto& bone = skel.bones[i];
        bone.name = e.node->mName.C_Str();
        bone.parentIdx = e.parentIdx;
        bone.localBind = toGlmMat4(e.node->mTransformation);
        auto it = boneInfoMap.find(bone.name);
        if (it != boneInfoMap.end()) {
            bone.skinIdx = (int16_t)it->second.id;
            bone.invBind = it->second.offset;
        }
        else {
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
    const roj::Animation& anim,
    const std::string& name)
{
    auto* clip = new roj::EvaluatableClip();
    clip->name = name;
    clip->duration = anim.duration;
    clip->ticksPerSec = anim.ticksPerSec;
    clip->boneCount = skel.boneCount;
    clip->tracks = new roj::BoneTrack[skel.boneCount];

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
        }
        else {
            totalTimes += 3; totalVec3s += 2; totalQuats += 1;  // single bind-pose keyframe
        }
    }

    // Single allocation for all keyframe data in this clip
    clip->storage.times.resize(totalTimes);
    clip->storage.vec3s.resize(totalVec3s);
    clip->storage.quats.resize(totalQuats);

    float* tb = clip->storage.times.data();
    glm::vec3* vb = clip->storage.vec3s.data();
    glm::quat* qb = clip->storage.quats.data();
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
            std::memcpy(vb + vOff, f.positions.data(), f.positions.size() * sizeof(glm::vec3));
            track.posTimes = { tb + tOff, f.positionTimestamps.size() };
            track.posValues = { vb + vOff, f.positions.size() };
            tOff += f.positionTimestamps.size();
            vOff += f.positions.size();

            std::memcpy(tb + tOff, f.rotationTimestamps.data(), f.rotationTimestamps.size() * sizeof(float));
            std::memcpy(qb + qOff, f.rotations.data(), f.rotations.size() * sizeof(glm::quat));
            track.rotTimes = { tb + tOff, f.rotationTimestamps.size() };
            track.rotValues = { qb + qOff, f.rotations.size() };
            tOff += f.rotationTimestamps.size();
            qOff += f.rotations.size();

            std::memcpy(tb + tOff, f.scaleTimestamps.data(), f.scaleTimestamps.size() * sizeof(float));
            std::memcpy(vb + vOff, f.scales.data(), f.scales.size() * sizeof(glm::vec3));
            track.scaleTimes = { tb + tOff, f.scaleTimestamps.size() };
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
            track.posTimes = { tb + tOff, 1 }; track.posValues = { vb + vOff, 1 };
            ++tOff; ++vOff;

            tb[tOff] = 0.f; qb[qOff] = bp.RotationQuaternion;
            track.rotTimes = { tb + tOff, 1 }; track.rotValues = { qb + qOff, 1 };
            ++tOff; ++qOff;

            tb[tOff] = 0.f; vb[vOff] = bp.Scale;
            track.scaleTimes = { tb + tOff, 1 }; track.scaleValues = { vb + vOff, 1 };
            ++tOff; ++vOff;
        }
    }

    assert(tOff == totalTimes);
    assert(vOff == totalVec3s);
    assert(qOff == totalQuats);
    return clip;
}

// ─────────────────────────────────────────────────────────────────────────────
// Fast Logic-tier glTF/GLB path — bones, skeleton, animation clips only.
//
// Why this exists: with SkipVisual=true, the assimp path above still calls
// m_import.ReadFileFromMemory() with the exact same postprocess flags as a
// full load (Triangulate, CalcTangentSpace, JoinIdenticalVertices, ...) —
// assimp decodes every vertex, normal, tangent, UV, material, and embedded
// image up front, and only *afterwards* do we throw most of it away in
// processNode()'s SkipVisual branch. That decode-then-discard pass is where
// ~90% of Logic-tier load time was going. (AI_CONFIG_PP_RVC_FLAGS further
// down was meant to trim this via aiProcess_RemoveComponent, but that flag
// was never actually added to the process-flags list passed to
// ReadFileFromMemory, so it silently did nothing — fixed below, but even
// correctly wired it only trims the already-fully-decoded aiScene, not the
// decode cost itself.)
//
// cgltf reads the glTF/GLB layout directly: we ask it for exactly the
// accessors a skin or animation channel points at (inverse-bind matrices,
// keyframe times/values) and touch nothing else — no triangulation, no
// tangent-space generation, no vertex dedup, no image decode. For the 99%
// case (a single-skin glb export), this is the entire cost difference.
//
// Every function below is read-only over the glTF data and writes only
// into the (private, not-yet-live) SkinnedModel being built — same
// threading contract as the assimp path.
// ─────────────────────────────────────────────────────────────────────────────

// cgltf gives local transforms as a column-major float[16]; glm::make_mat4
// expects exactly that layout, so this is a straight reinterpretation, not
// a conversion.
static glm::mat4 GLTFNodeLocalTransform(const cgltf_node* node)
{
    float m[16];
    cgltf_node_transform_local(node, m);
    return glm::make_mat4(m);
}

// World transform via the parent chain — only used for the rare "bone has
// an animation channel but isn't part of any skin" case, mirroring what
// extractAnimations() does for the assimp path via aiNode::mParent.
static glm::mat4 GLTFWorldTransform(const cgltf_node* node)
{
    glm::mat4 g(1.f);
    for (const cgltf_node* cur = node; cur; cur = cur->parent)
        g = GLTFNodeLocalTransform(cur) * g;
    return g;
}

// glTF can have multiple root nodes (no single mRootNode the way assimp
// always synthesizes one); this just collects whichever set the file
// actually uses.
static std::vector<cgltf_node*> GLTFSceneRoots(const cgltf_data* data)
{
    std::vector<cgltf_node*> roots;
    const cgltf_scene* scene = data->scene ? data->scene
        : (data->scenes_count > 0 ? &data->scenes[0] : nullptr);
    if (scene)
    {
        roots.assign(scene->nodes, scene->nodes + scene->nodes_count);
    }
    else
    {
        // No scene defined at all (unusual but valid glTF) - fall back to
        // every node with no parent.
        for (cgltf_size i = 0; i < data->nodes_count; ++i)
            if (!data->nodes[i].parent)
                roots.push_back(&data->nodes[i]);
    }
    return roots;
}

// Populates model.boneInfoMap from one skin's joints[] + inverse bind
// matrices.
//
// Bone IDs follow skin->joints[] order directly. This matters:
// AdoptVisualTierGeometry()'s debug safety check exists precisely because
// this ID assignment must line up with whatever order the Visual tier's
// (separate, assimp-based) mesh reparse assigns the same bones — and in
// practice, for a single-skin character export (the standard case this
// whole fast path targets), assimp's own glTF importer builds aiMesh::mBones
// straight from the same skin.joints[] array, so the two orders already
// match without this path needing to know anything about assimp's
// internals. Multi-skin files are the one case that could disagree; that's
// exactly what the debug check is there to catch.
static void extractBoneInfoFromGLTFSkin(const cgltf_skin* skin, roj::SkinnedModel& model)
{
    if (!skin || skin->joints_count == 0) return;

    std::vector<float> invBindFloats;
    const cgltf_accessor* ibmAccessor = skin->inverse_bind_matrices;
    if (ibmAccessor)
    {
        invBindFloats.resize((size_t)skin->joints_count * 16);
        cgltf_accessor_unpack_floats(ibmAccessor, invBindFloats.data(), invBindFloats.size());
    }

    for (cgltf_size i = 0; i < skin->joints_count; ++i)
    {
        const cgltf_node* joint = skin->joints[i];
        hashed_string boneName = joint->name ? joint->name : "";
        if (model.boneInfoMap.find(boneName) != model.boneInfoMap.end())
            continue;

        glm::mat4 offset = ibmAccessor ? glm::make_mat4(&invBindFloats[(size_t)i * 16]) : glm::mat4(1.f);
        model.boneInfoMap[boneName] = roj::BoneInfo{ model.boneCount, offset };
        model.boneCount++;
    }
}

// Extracts every animation's per-bone keyframe tracks into the same
// legacy roj::Animation/FrameBoneTransform structures the assimp path
// fills in, so buildEvaluatableClip() below works completely unmodified
// regardless of which parser produced the data.
static void extractAnimationsFromGLTF(const cgltf_data* data, roj::SkinnedModel& model)
{
    for (cgltf_size a = 0; a < data->animations_count; ++a)
    {
        const cgltf_animation& srcAnim = data->animations[a];
        std::string animName = srcAnim.name ? srcAnim.name : ("Animation" + std::to_string(a));
        roj::Animation& anim = model.animations[animName];

        // glTF sampler input times are already in seconds - representing
        // that as "ticksPerSec = 1" keeps duration/ticksPerSec == seconds,
        // the same convention the assimp path uses, so nothing downstream
        // (Animator, bakeClipFrames) needs to know which parser was used.
        anim.ticksPerSec = 1.f;

        float maxTime = 0.f;
        int totalKeys = 0;

        // Tracks which TRS components an actual channel provided for each
        // bone touched by this animation, plus the node pointer - needed
        // to backfill whichever component(s) turn out missing once this
        // channel loop finishes. See the comment below the loop for why.
        struct SeenFlags { bool pos = false, rot = false, scale = false; const cgltf_node* node = nullptr; };
        std::unordered_map<hashed_string, SeenFlags> seen;

        for (cgltf_size c = 0; c < srcAnim.channels_count; ++c)
        {
            const cgltf_animation_channel& channel = srcAnim.channels[c];
            if (!channel.target_node || !channel.sampler) continue;
            if (channel.target_path == cgltf_animation_path_type_weights)
                continue; // morph target weights - not a bone property, irrelevant to the Logic tier

            hashed_string boneName = channel.target_node->name ? channel.target_node->name : "";
            roj::FrameBoneTransform& track = anim.animationFrames[boneName];
            SeenFlags& flags = seen[boneName];
            flags.node = channel.target_node;

            const cgltf_animation_sampler& sampler = *channel.sampler;
            const cgltf_accessor* inputAcc = sampler.input;
            const cgltf_accessor* outputAcc = sampler.output;
            if (!inputAcc || !outputAcc) continue;

            const cgltf_size keyCount = inputAcc->count;
            std::vector<float> times(keyCount);
            cgltf_accessor_unpack_floats(inputAcc, times.data(), keyCount);
            if (keyCount > 0) maxTime = std::max(maxTime, times.back());
            totalKeys += (int)keyCount;

            // Cubic-spline samplers store (in-tangent, value, out-tangent)
            // per keyframe; this reads only the value and treats every
            // interpolation mode as linear/step, same simplification the
            // consumer (roj::Animator) already makes for the assimp path
            // (assimp only ever exposes linear-interpolated keys too).
            // Good enough for the vast majority of glTF exports.
            const cgltf_size stride = (sampler.interpolation == cgltf_interpolation_type_cubic_spline) ? 3 : 1;
            const cgltf_size valueOffset = (stride == 3) ? 1 : 0;

            switch (channel.target_path)
            {
            case cgltf_animation_path_type_translation:
            {
                std::vector<float> values((size_t)keyCount * stride * 3);
                cgltf_accessor_unpack_floats(outputAcc, values.data(), values.size());
                track.positionTimestamps = times;
                track.positions.resize(keyCount);
                for (cgltf_size k = 0; k < keyCount; ++k) {
                    const float* v = &values[(size_t)(k * stride + valueOffset) * 3];
                    track.positions[k] = glm::vec3(v[0], v[1], v[2]);
                }
                flags.pos = true;
                break;
            }
            case cgltf_animation_path_type_rotation:
            {
                std::vector<float> values((size_t)keyCount * stride * 4);
                cgltf_accessor_unpack_floats(outputAcc, values.data(), values.size());
                track.rotationTimestamps = times;
                track.rotations.resize(keyCount);
                for (cgltf_size k = 0; k < keyCount; ++k) {
                    const float* v = &values[(size_t)(k * stride + valueOffset) * 4];
                    // glTF quaternions are (x, y, z, w); glm::quat's
                    // constructor takes (w, x, y, z).
                    track.rotations[k] = glm::quat(v[3], v[0], v[1], v[2]);
                }
                flags.rot = true;
                break;
            }
            case cgltf_animation_path_type_scale:
            {
                std::vector<float> values((size_t)keyCount * stride * 3);
                cgltf_accessor_unpack_floats(outputAcc, values.data(), values.size());
                track.scaleTimestamps = times;
                track.scales.resize(keyCount);
                for (cgltf_size k = 0; k < keyCount; ++k) {
                    const float* v = &values[(size_t)(k * stride + valueOffset) * 3];
                    track.scales[k] = glm::vec3(v[0], v[1], v[2]);
                }
                flags.scale = true;
                break;
            }
            default: break;
            }

            if (model.boneInfoMap.find(boneName) == model.boneInfoMap.end())
            {
                // Bone driven by an animation channel but not part of any
                // skin (e.g. a prop bone) - give it a slot too, same as
                // extractAnimations()'s "additionalBones" pass does for
                // the assimp path.
                model.boneInfoMap[boneName] = roj::BoneInfo{
                    model.boneCount, glm::inverse(GLTFWorldTransform(channel.target_node)) };
                model.boneCount++;
            }
        }

        // glTF omits a TRS channel entirely for any component that doesn't
        // change - no reason to export redundant constant keys. assimp's
        // importer papers over this: it always synthesizes a full
        // position+rotation+scale triplet per animated node, holding the
        // node's own bind-pose value for whichever component the source
        // file didn't actually animate. cgltf does no such synthesis - the
        // loop above only ever fills in whatever channels literally exist.
        // Left alone, a bone animated in rotation/translation but not
        // scale would leave its scale track empty, and whatever an empty
        // track evaluates to downstream isn't necessarily this bone's
        // actual bind scale. For a root bone with a non-1.0 bind scale (a
        // common way to bake a unit/size correction into a rig), that
        // shows up as the whole model snapping to the wrong size the
        // moment it starts animating - correct at rest, wrong the instant
        // this clip plays. A single "hold" key from the node's own local
        // bind transform is exactly what assimp already synthesizes here.
        for (auto& [boneName, flags] : seen)
        {
            if (flags.pos && flags.rot && flags.scale) continue;

            glm::vec3 bindPos, bindScale, skew;
            glm::quat bindRot;
            glm::vec4 perspective;
            glm::decompose(GLTFNodeLocalTransform(flags.node), bindScale, bindRot, bindPos, skew, perspective);

            roj::FrameBoneTransform& track = anim.animationFrames[boneName];
            if (!flags.pos)
            {
                track.positionTimestamps = { 0.f };
                track.positions = { bindPos };
            }
            if (!flags.rot)
            {
                track.rotationTimestamps = { 0.f };
                track.rotations = { bindRot };
            }
            if (!flags.scale)
            {
                track.scaleTimestamps = { 0.f };
                track.scales = { bindScale };
            }
        }

        anim.duration = maxTime; // seconds, and ticksPerSec == 1 above, so this doubles as "ticks"
        // Legacy/informational field only (SkeletalMesh.cpp) - approximates
        // the assimp path's "channel 0 key count / duration" with an
        // all-channels average; nothing in the hot path depends on it.
        anim.frameTime = maxTime > 0.f ? (float)totalKeys / maxTime : 0.f;
    }
}

// glTF equivalent of extractBoneNodeAndBuildMap() - single-root recursive
// case, identical shape to the assimp version, just walking cgltf_node
// instead of aiNode.
static void extractBoneNodeAndBuildMapGLTF(roj::SkinnedModel& model,
    roj::BoneNode& bone, const cgltf_node* src)
{
    bone.name = src->name ? src->name : "";
    bone.transform = GLTFNodeLocalTransform(src);
    for (cgltf_size i = 0; i < src->children_count; ++i)
    {
        roj::BoneNode node;
        extractBoneNodeAndBuildMapGLTF(model, node, src->children[i]);
        bone.children.push_back(node);
        model.parentMap[node.name] = bone.name;
    }
    model.boneNodesMap[bone.name] = bone;
}

// Builds model.defaultRoot, wrapping every glTF scene root node under one
// synthetic identity-transform root - mirrors the single root node assimp
// always synthesizes for a glTF import, so defaultRoot means the same thing
// (and bone index 0 lines up the same way) regardless of which parser ran.
static void buildDefaultRootFromGLTF(roj::SkinnedModel& model, const std::vector<cgltf_node*>& sceneRoots)
{
    roj::BoneNode& root = model.defaultRoot;
    root.name = "RootNode";
    root.transform = glm::mat4(1.f);
    for (cgltf_node* r : sceneRoots)
    {
        roj::BoneNode child;
        extractBoneNodeAndBuildMapGLTF(model, child, r);
        root.children.push_back(child);
        model.parentMap[child.name] = root.name;
    }
    model.boneNodesMap[root.name] = root;
}

// glTF equivalent of buildFlatSkeleton() - same iterative DFS pre-order
// (parentIdx[i] < i for all i > 0), just seeded from every glTF scene root
// node under one synthetic index-0 root instead of a single aiNode.
static void buildFlatSkeletonFromGLTF(roj::FlatSkeleton& skel,
    const std::vector<cgltf_node*>& sceneRoots,
    const std::unordered_map<hashed_string, roj::BoneInfo>& boneInfoMap)
{
    struct Entry { cgltf_node* node; uint16_t parentIdx; glm::mat4 localBind; hashed_string name; };
    std::vector<Entry> entries;
    entries.reserve(256);

    // Synthetic identity root at index 0 - see buildDefaultRootFromGLTF.
    entries.push_back({ nullptr, INVALID_BONE_IDX, glm::mat4(1.f), hashed_string("RootNode") });

    struct StackEntry { cgltf_node* node; uint16_t parentIdx; };
    std::vector<StackEntry> stack;
    stack.reserve(64);
    for (auto it = sceneRoots.rbegin(); it != sceneRoots.rend(); ++it)
        stack.push_back({ *it, 0 });

    while (!stack.empty())
    {
        auto [node, parentIdx] = stack.back();
        stack.pop_back();
        const uint16_t myIdx = (uint16_t)entries.size();
        entries.push_back({ node, parentIdx, GLTFNodeLocalTransform(node),
                             hashed_string(node->name ? node->name : "") });
        for (int i = (int)node->children_count - 1; i >= 0; --i)
            stack.push_back({ node->children[i], myIdx });
    }

    skel.boneCount = (uint16_t)entries.size();
    skel.bones = new roj::SkeletonBone[skel.boneCount];

    for (uint16_t i = 0; i < skel.boneCount; ++i)
    {
        auto& e = entries[i];
        auto& bone = skel.bones[i];
        bone.name = e.name;
        bone.parentIdx = e.parentIdx;
        bone.localBind = e.localBind;
        auto it = boneInfoMap.find(bone.name);
        if (it != boneInfoMap.end()) {
            bone.skinIdx = (int16_t)it->second.id;
            bone.invBind = it->second.offset;
        }
        else {
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

// Parses `data`/`fileData` and points every cgltf_buffer's `data` at its
// actual bytes - the embedded chunk for a GLB, or an external .bin/base64
// data URI for a plain .gltf. `fileData` must outlive `outData` - for a
// GLB, cgltf references its bytes directly rather than copying them.
static bool ParseGLTFFast(const std::string& path, const std::vector<uint8_t>& fileData, cgltf_data*& outData)
{
    outData = nullptr;
    cgltf_options options{};
    if (cgltf_parse(&options, fileData.data(), fileData.size(), &outData) != cgltf_result_success)
        return false;

    // Required for GLB too, not just plain .gltf: cgltf_parse only reads
    // the JSON structure (node names/hierarchy, skin and animation
    // *counts*) - it records the GLB's embedded binary chunk in
    // data->bin/data->bin_size but does NOT wire that pointer into
    // data->buffers[]->data itself. Every accessor read below (inverse-bind
    // matrices, animation keyframe values) dereferences that buffer
    // pointer, so skipping this call doesn't fail loudly - it reads
    // through a null/stale buffer and produces garbage bone poses, which
    // is worse than an outright failure. cgltf_load_buffers is what
    // actually assigns buffer->data = data->bin for a GLB's buffer (a
    // pointer fixup, no file I/O); it only touches disk for a plain
    // .gltf's external relative-path buffers, which is the only case
    // `path` matters for here.
    if (cgltf_load_buffers(&options, outData, path.c_str()) != cgltf_result_success)
    {
        cgltf_free(outData);
        outData = nullptr;
        return false;
    }
    return true;
}

// Top-level fast Logic-tier loader: bones, skeleton hierarchy, and
// animation clips only. No meshes, no materials, no images - see the
// banner comment at the top of this section for why. Returns false (and
// leaves `model` however far it got) for anything this path doesn't
// handle; the caller falls back to the normal assimp path in that case, so
// this never has to be exhaustive to be safe.
static bool LoadLogicTierFromGLTF(const std::string& path, roj::SkinnedModel& model, std::string& infoLog)
{
    std::vector<uint8_t> fileData = FileSystemEngine::ReadFileBinary(path);
    if (fileData.empty()) { infoLog += "Failed to read: " + path + "\n"; return false; }

    cgltf_data* data = nullptr;
    if (!ParseGLTFFast(path, fileData, data))
        return false;

    model.m_isOBJ = false;
    model.globalInversed = glm::mat4(1.f); // synthetic root is always identity - see buildDefaultRootFromGLTF
    model.sceneCamera = nullptr;        // cameras aren't part of the Logic tier

    std::vector<cgltf_node*> sceneRoots = GLTFSceneRoots(data);
    if (sceneRoots.empty())
    {
        infoLog += "glTF fast path: no scene root nodes in " + path + "\n";
        cgltf_free(data);
        return false;
    }

    for (cgltf_size i = 0; i < data->skins_count; ++i)
        extractBoneInfoFromGLTFSkin(&data->skins[i], model);

    extractAnimationsFromGLTF(data, model);

    if (model.boneCount > 0)
        buildDefaultRootFromGLTF(model, sceneRoots);

    buildFlatSkeletonFromGLTF(model.skeleton, sceneRoots, model.boneInfoMap);

    // boundingSphere/boundingBox/boneBounds intentionally left at their
    // defaults here, same as the assimp SkipVisual path (which computes
    // them from an empty vertex list when meshes aren't parsed) - the
    // Visual tier overwrites all three with real values via
    // AdoptVisualTierGeometry().
    for (auto& [animName, anim] : model.animations)
        model.clips[animName] = buildEvaluatableClip(model.skeleton, anim, animName.str());

    cgltf_free(data);
    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// BakeClipFramesInto — runs off the hot path, in parallel across clips.
//
// Same simulation as before, just writing into a caller-owned `out` vector
// instead of `clip.bakedFrames` directly. That's what lets
// roj::BakeAllClipsParallel() (below, in the roj namespace with the rest of
// SkinnedModel's public surface) call this once per clip, concurrently,
// without any two calls ever touching the same memory: each call only reads
// `model` (shared, but read-only for the whole parallel batch) and writes
// its own distinct `out`. Nothing here ends up on the live model until
// roj::ApplyBakedAnimationFrames() runs later, on the main thread.
// ─────────────────────────────────────────────────────────────────────────────

static void BakeClipFramesInto(const roj::EvaluatableClip& clip, const roj::SkinnedModel& model,
    float frameInterval, std::vector<roj::EvaluatableClip::BakedFrame>& out)
{
    // Animator's constructor takes a non-const SkinnedModel* - this call
    // only ever reads model.skeleton/model.clips (every clip gets its own
    // local Animator instance, reset and discarded at the end of this
    // function), so treating `model` as read-only for the duration of a
    // parallel bake batch is safe.
    roj::Animator animator(const_cast<roj::SkinnedModel*>(&model));
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

    out.push_back(captureFrame());
    while (animator.m_playing) {
        animator.update(frameInterval);
        out.push_back(captureFrame());
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
    // BakeAllClipsParallel / ApplyBakedAnimationFrames
    // See the banner comment on BakedClipsResult in skinned_model.hpp for the
    // rationale behind the two-step split.
    // ─────────────────────────────────────────────────────────────────────────────

    BakedClipsResult BakeAllClipsParallel(const SkinnedModel& model, float frameRate)
    {
        BakedClipsResult result;
        result.frameInterval = 1.f / frameRate;
        if (model.clips.empty()) return result;

        struct Job { hashed_string name; const EvaluatableClip* clip; };
        std::vector<Job> jobs;
        jobs.reserve(model.clips.size());
        for (auto& [name, clip] : model.clips)
            jobs.push_back({ name, clip });

        // One output slot per job, indexed by position - each parallel task
        // writes only to its own slot, so no two tasks ever touch the same
        // memory (not even map-bucket-adjacent, unlike writing into a shared
        // unordered_map from multiple threads would be).
        std::vector<std::vector<EvaluatableClip::BakedFrame>> baked(jobs.size());

        std::vector<size_t> indices(jobs.size());
        std::iota(indices.begin(), indices.end(), 0);

        std::for_each(std::execution::par, indices.begin(), indices.end(),
            [&](size_t i)
            {
                BakeClipFramesInto(*jobs[i].clip, model, result.frameInterval, baked[i]);
            });

        // Single-threaded merge into the map the caller gets back - happens
        // after for_each has fully joined, so this isn't concurrent with
        // anything above.
        result.frames.reserve(jobs.size());
        for (size_t i = 0; i < jobs.size(); ++i)
            result.frames.emplace(jobs[i].name, std::move(baked[i]));

        return result;
    }

    void ApplyBakedAnimationFrames(SkinnedModel& model, BakedClipsResult&& baked)
    {
        for (auto& [name, frames] : baked.frames)
        {
            auto it = model.clips.find(name);
            if (it == model.clips.end()) continue; // clip no longer exists (e.g. UnloadAll() ran) - just drop it
            it->second->bakedFrames = std::move(frames);
            it->second->bakedFrameInterval = baked.frameInterval;
        }
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
                    bb.bbMin = glm::min(bb.bbMin, v.Position);
                    bb.bbMax = glm::max(bb.bbMax, v.Position);
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
        glm::vec3& bmin,
        glm::vec3& bmax,
        const glm::mat4& M,
        const BoneBound& b)
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
        const glm::mat4& worldMatrix,
        std::span<const glm::mat4>  skinMatrices) const
    {
        if (boneBounds.empty()) return boundingBox;

        glm::vec3 bmin(std::numeric_limits<float>::max());
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
        const glm::mat4& worldMatrix,
        std::span<const glm::mat4>  skinMatrices) const
    {
        if (boneBounds.empty()) return boundingSphere;

        glm::vec3 bmin(std::numeric_limits<float>::max());
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
            v.Tangent = { mesh->mTangents[i].x,   mesh->mTangents[i].y,   mesh->mTangents[i].z };
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

        if (bgfx::isValid(skinMesh.vbh) == false)
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

        if (SkipVisual)
        {
            // Fast path: skip assimp entirely for a glTF/GLB Logic-tier load -
            // see the banner comment above LoadLogicTierFromGLTF for why. Only
            // short-circuits on success; anything this fast path doesn't
            // handle (wrong extension, a parse it can't make sense of) falls
            // straight through to the normal assimp path below, unchanged.
            const bool looksLikeGLTF = StringHelper::EndsWith(path, ".glb") || StringHelper::EndsWith(path, ".gltf");
            if (looksLikeGLTF && LoadLogicTierFromGLTF(path, m_model, m_infoLog))
            {
                PrewarmSkeletonTopology(&m_model.defaultRoot);
                return true;
            }
        }

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
            // aiProcess_RemoveComponent actually has to be in this flag list for
            // AI_CONFIG_PP_RVC_FLAGS above to do anything - without it, that
            // config was silently ignored and every SkipVisual load paid full
            // price for materials/textures/normals/tangents/etc it was trying
            // to skip. Now reached only by non-glTF formats, or a glTF the fast
            // path above couldn't parse, since GLB/glTF short-circuits before
            // this point - see LoadLogicTierFromGLTF.
            unsigned int postProcess = aiProcess_Triangulate | aiProcess_FlipUVs |
                aiProcess_CalcTangentSpace | aiProcess_LimitBoneWeights |
                aiProcess_JoinIdenticalVertices | aiProcess_GlobalScale;
            if (SkipVisual)
                postProcess |= aiProcess_RemoveComponent;
            m_scene = m_import.ReadFileFromMemory(fileData.data(), fileData.size(),
                postProcess, path.c_str());
            m_cachedScene = m_scene;
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

        m_relativeDir = "GameData/";
        m_model.globalInversed = glm::inverse(toGlmMat4(m_scene->mRootNode->mTransformation));
        m_model.sceneCamera = m_scene->HasCameras() ? m_scene->mCameras[0] : nullptr;

        // Step 1: build meshes + populate boneInfoMap
        processNode(m_scene->mRootNode, m_scene);


        // Step 2: extract animations into legacy map (for m_currAnim->duration etc.)
        extractAnimations(m_scene, m_model);

        // Step 3: bounding volumes
        m_model.boundingSphere = BoundingSphere::FromPoints(vertexPositions);
        m_model.boundingBox = BoundingBox::FromPoints(vertexPositions);
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

        // Baking for UsePrecomputedFrames mode used to happen right here,
        // unconditionally, for every load - including this SkipVisual path,
        // synchronously on whichever thread called it (the game thread, for a
        // default-tier load - see AssetRegistry::LoadLogicTierNow), and again,
        // redundantly, on every Visual-tier reparse whose clips just get
        // discarded by AdoptVisualTierGeometry(). Baking only needs skeleton +
        // clip track data - it doesn't need mesh data and doesn't need to run
        // on this thread at all - so it's now the caller's job: see
        // roj::BakeAllClipsParallel()/roj::ApplyBakedAnimationFrames() in
        // skinned_model.hpp, meant to be driven from AssetRegistry once per
        // load, off the game thread, regardless of which tier's parse produced
        // these clips.

        // Step 6: texture processing + opaque-before-transparent sort
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