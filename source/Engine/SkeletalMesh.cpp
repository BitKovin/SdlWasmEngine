#include "SkeletalMesh.hpp"
#include <shared_mutex>
#include <algorithm>
#include "Level.hpp"
#include "EngineMain.h"

AnimationPose AnimationPose::Lerp(const AnimationPose& a,
const AnimationPose& b,
	float progress)
{
	// Fast paths
	if (progress > 0.995f)
		return b;

	if (progress < 0.005f)
	{
		AnimationPose result = a;

		// Root always comes from B, even at progress=0
		auto bRootIt = b.boneTransforms.find("root");
		if (bRootIt != b.boneTransforms.end())
			result.boneTransforms["root"] = bRootIt->second;

		return result;
	}

	AnimationPose result;
	auto& out = result.boneTransforms;

	// Start by copying B (so bones only in B are preserved)
	out = b.boneTransforms;

	for (const auto& [bone, aMat] : a.boneTransforms)
	{
		auto bIt = b.boneTransforms.find(bone);

		// 🔹 Bone only in A → keep A
		if (bIt == b.boneTransforms.end())
		{
			out[bone] = aMat;
			continue;
		}

		const mat4& bMat = bIt->second;

		// 🔹 Root always from B
		if (bone == "root")
		{
			out[bone] = bMat;
			continue;
		}

		// ---------- Decompose A ----------
		glm::vec3 aTrans = glm::vec3(aMat[3]);

		glm::vec3 aScale;
		for (int i = 0; i < 3; i++)
			aScale[i] = glm::length(aMat[i]);

		glm::mat3 aRotMat;
		for (int i = 0; i < 3; i++)
			aRotMat[i] = aMat[i] / aScale[i];

		glm::quat aQuat = glm::quat_cast(aRotMat);

		// ---------- Decompose B ----------
		glm::vec3 bTrans = glm::vec3(bMat[3]);

		glm::vec3 bScale;
		for (int i = 0; i < 3; i++)
			bScale[i] = glm::length(bMat[i]);

		glm::mat3 bRotMat;
		for (int i = 0; i < 3; i++)
			bRotMat[i] = bMat[i] / bScale[i];

		glm::quat bQuat = glm::quat_cast(bRotMat);

		// ---------- Interpolate ----------
		glm::vec3 trans = glm::mix(aTrans, bTrans, progress);
		glm::vec3 scale = glm::mix(aScale, bScale, progress);
		glm::quat quat = glm::slerp(aQuat, bQuat, progress);

		// ---------- Rebuild ----------
		glm::mat3 rotMat = glm::mat3_cast(quat);
		glm::mat3 scaleMat(
			scale.x, 0, 0,
			0, scale.y, 0,
			0, 0, scale.z);

		glm::mat3 rsMat = rotMat * scaleMat;

		mat4 resultMat = glm::mat4(rsMat);
		resultMat[3] = glm::vec4(trans, 1.0f);

		out[bone] = resultMat;
	}

	return result;
}

static glm::mat4 ComputeGlobalTransform(
    const std::unordered_map<hashed_string, glm::mat4>& pose,
    const hashed_string& bone,
    const std::unordered_map<hashed_string, hashed_string>* parentMap)
{
    // if no parent map, assume local == global
    if (!parentMap) {
        auto it = pose.find(bone);
        return (it != pose.end()) ? it->second : glm::mat4(1.0f);
    }

    // accumulate from bone up to root: global = parentLocal * ... * boneLocal
    glm::mat4 global(1.0f);
    hashed_string cur = bone;
    auto it = pose.find(cur);
    if (it == pose.end()) return glm::mat4(1.0f);

    // start with local
    global = it->second;

    // walk up while we have parents in the map and in the pose
    while (true)
    {
        auto pit = parentMap->find(cur);
        if (pit == parentMap->end()) break;
        hashed_string parent = pit->second;
        auto parentPoseIt = pose.find(parent);
        if (parentPoseIt == pose.end()) break;
        // parentLocal * currentGlobal
        global = parentPoseIt->second * global;
        cur = parent;
    }
    return global;
}

// Static caches keyed by root node pointer. Simple maps + shared mutex.
static std::unordered_map<roj::BoneNode*, std::vector<hashed_string>> s_traversalMap;
static std::unordered_map<roj::BoneNode*, std::unordered_map<hashed_string, int>> s_indexMap;
static std::unordered_map<roj::BoneNode*, std::vector<int>> s_parentIndexMap;
static std::unordered_map<roj::BoneNode*, std::vector<std::vector<int>>> s_childrenMap;
static std::shared_mutex s_cacheMutex; // readers/writers (shared for readers, unique for writers)


AnimationPose AnimationPose::LayeredLerp(
	const hashed_string& startBoneName,
	roj::BoneNode* rootNode,
	const AnimationPose& poseA,
	const AnimationPose& poseB,
	float UseWorldSpaceRotation,
	float progress)
{
	using glm::mat4;
	using glm::vec3;
	using glm::quat;

	if (!rootNode) return poseA;
	if (progress <= 0.005f) {
		AnimationPose out = poseA;
		auto rb = poseB.boneTransforms.find(hashed_string("root"));
		if (rb != poseB.boneTransforms.end()) out.boneTransforms[hashed_string("root")] = rb->second;
		return out;
	}

	UseWorldSpaceRotation *= progress;

	const float EPS_SCALE = 1e-6f;

	// --- Helpers ---
	auto DecomposeLocal = [&](const mat4& m, vec3& outT, vec3& outS, quat& outQ) {
		outT = vec3(m[3]);

		vec3 c0 = vec3(m[0]);
		vec3 c1 = vec3(m[1]);
		vec3 c2 = vec3(m[2]);

		outS.x = glm::length(c0);
		outS.y = glm::length(c1);
		outS.z = glm::length(c2);

		if (outS.x < EPS_SCALE) outS.x = 1.0f;
		if (outS.y < EPS_SCALE) outS.y = 1.0f;
		if (outS.z < EPS_SCALE) outS.z = 1.0f;

		vec3 r0 = c0 / outS.x;
		vec3 r1 = c1 / outS.y;
		vec3 r2 = c2 / outS.z;

		if (glm::length(r0) > EPS_SCALE) r0 = glm::normalize(r0);

		if (glm::length(r1) > EPS_SCALE) {
			r1 = r1 - r0 * glm::dot(r0, r1);
			if (glm::length(r1) > EPS_SCALE) r1 = glm::normalize(r1);
			else {
				r1 = glm::normalize(glm::cross(r0, vec3(0.0f, 0.0f, 1.0f)));
				if (glm::length(r1) < EPS_SCALE) r1 = glm::normalize(glm::cross(r0, vec3(0.0f, 1.0f, 0.0f)));
			}
		}
		else {
			r1 = glm::normalize(glm::cross(r2, r0));
			if (glm::length(r1) < EPS_SCALE) r1 = vec3(0.0f, 1.0f, 0.0f);
		}
		r2 = glm::cross(r0, r1);

		glm::mat3 R;
		R[0] = r0;
		R[1] = r1;
		R[2] = r2;

		outQ = glm::quat_cast(R);
		outQ = glm::normalize(outQ);
		};

	auto ComposeLocal = [&](const vec3& t, const vec3& s, const quat& q) -> mat4 {
		glm::mat3 r = glm::mat3_cast(q);
		glm::mat3 sm(s.x, 0.0f, 0.0f,
			0.0f, s.y, 0.0f,
			0.0f, 0.0f, s.z);
		glm::mat3 rs = r * sm;
		mat4 out = mat4(rs);
		out[3] = glm::vec4(t, 1.0f);
		return out;
		};

	// --- Cached skeleton data (unchanged) ---
	std::vector<hashed_string> traversal;
	std::unordered_map<hashed_string, int> indexMap;
	std::vector<int> parentIndex;
	std::vector<std::vector<int>> children;

	{
		std::shared_lock<std::shared_mutex> rlock(s_cacheMutex);
		auto it = s_indexMap.find(rootNode);
		if (it != s_indexMap.end()) {
			traversal = s_traversalMap[rootNode];
			indexMap = it->second;
			parentIndex = s_parentIndexMap[rootNode];
			children = s_childrenMap[rootNode];
		}
	}

	if (indexMap.empty()) {
		std::unique_lock<std::shared_mutex> wlock(s_cacheMutex);
		auto it2 = s_indexMap.find(rootNode);
		if (it2 == s_indexMap.end()) {
			std::vector<hashed_string> tmpTraversal;
			tmpTraversal.reserve(256);
			std::unordered_map<hashed_string, hashed_string> tempParent;
			std::function<void(roj::BoneNode*, const hashed_string*)> build = [&](roj::BoneNode* n, const hashed_string* parent) {
				if (!n) return;
				tmpTraversal.push_back(n->name);
				if (parent) tempParent[n->name] = *parent;
				for (auto& c : n->children) build(const_cast<roj::BoneNode*>(&c), &n->name);
				};
			build(rootNode, nullptr);
			int Ntmp = (int)tmpTraversal.size();
			std::unordered_map<hashed_string, int> tmpIndex; tmpIndex.reserve(Ntmp * 2);
			for (int i = 0; i < Ntmp; ++i) tmpIndex[tmpTraversal[i]] = i;
			std::vector<int> tmpParentIndex(Ntmp, -1);
			for (int i = 0; i < Ntmp; ++i) {
				auto pit = tempParent.find(tmpTraversal[i]);
				if (pit != tempParent.end()) {
					tmpParentIndex[i] = tmpIndex[pit->second];
				}
			}
			std::vector<std::vector<int>> tmpChildren(Ntmp);
			for (int i = 0; i < Ntmp; ++i) {
				int p = tmpParentIndex[i];
				if (p >= 0) tmpChildren[p].push_back(i);
			}
			s_traversalMap[rootNode] = std::move(tmpTraversal);
			s_indexMap[rootNode] = std::move(tmpIndex);
			s_parentIndexMap[rootNode] = std::move(tmpParentIndex);
			s_childrenMap[rootNode] = std::move(tmpChildren);

			traversal = s_traversalMap[rootNode];
			indexMap = s_indexMap[rootNode];
			parentIndex = s_parentIndexMap[rootNode];
			children = s_childrenMap[rootNode];
		}
		else {
			traversal = s_traversalMap[rootNode];
			indexMap = it2->second;
			parentIndex = s_parentIndexMap[rootNode];
			children = s_childrenMap[rootNode];
		}
	}

	int N = (int)traversal.size();
	if (N == 0) return poseA;

	// --- Decompose local matrices for A/B ---
	std::vector<mat4> localA(N, mat4(1.0f)), localB(N, mat4(1.0f));
	std::vector<vec3> tA(N), sA(N), tB(N), sB(N);
	std::vector<quat> qA(N), qB(N);

	for (int i = 0; i < N; ++i) {
		const hashed_string& name = traversal[i];
		auto ita = poseA.boneTransforms.find(name);
		auto itb = poseB.boneTransforms.find(name);
		if (ita != poseA.boneTransforms.end()) localA[i] = ita->second;
		if (itb != poseB.boneTransforms.end()) localB[i] = itb->second;
		DecomposeLocal(localA[i], tA[i], sA[i], qA[i]);
		DecomposeLocal(localB[i], tB[i], sB[i], qB[i]);
	}

	// --- Compute global matrices for A/B (only needed to get global quats for start bone) ---
	std::vector<mat4> globalA(N), globalB(N);
	for (int i = 0; i < N; ++i) {
		int p = parentIndex[i];
		if (p == -1) {
			globalA[i] = localA[i];
			globalB[i] = localB[i];
		}
		else {
			globalA[i] = globalA[p] * localA[i];
			globalB[i] = globalB[p] * localB[i];
		}
	}

	// --- Compute global quaternions by composing local quaternions (stable) ---
	std::vector<quat> globalQA(N), globalQB(N);
	for (int i = 0; i < N; ++i) {
		int p = parentIndex[i];
		if (p == -1) {
			globalQA[i] = glm::normalize(qA[i]);
			globalQB[i] = glm::normalize(qB[i]);
		}
		else {
			globalQA[i] = glm::normalize(globalQA[p] * qA[i]);
			globalQB[i] = glm::normalize(globalQB[p] * qB[i]);
		}
	}

	// --- start bone index ---
	auto itStart = indexMap.find(startBoneName);
	if (itStart == indexMap.end()) {
		AnimationPose out = poseA;
		auto rb = poseB.boneTransforms.find(hashed_string("root"));
		if (rb != poseB.boneTransforms.end()) out.boneTransforms[hashed_string("root")] = rb->second;
		return out;
	}
	int startIdx = itStart->second;

	// --- Collect subtree (parent-before-child order) ---
	std::vector<char> inSubtree(N, 0);
	std::vector<int> subtreeList; subtreeList.reserve(64);
	std::vector<int> stack; stack.push_back(startIdx);
	while (!stack.empty()) {
		int cur = stack.back(); stack.pop_back();
		if (inSubtree[cur]) continue;
		inSubtree[cur] = 1;
		subtreeList.push_back(cur);
		for (int ci : children[cur]) stack.push_back(ci);
	}
	std::sort(subtreeList.begin(), subtreeList.end(), [](int a, int b) { return a < b; });

	// --- Prepare result arrays ---
	std::vector<mat4> resultLocal(N);
	std::vector<mat4> resultGlobal(N);
	for (int i = 0; i < N; ++i) resultLocal[i] = localA[i];

	std::vector<quat> resultLocalQuat(N, quat(1.0f, 0.0f, 0.0f, 0.0f));
	std::vector<quat> resultGlobalQuat(N, quat(1.0f, 0.0f, 0.0f, 0.0f));

	// initialize nodes not in subtree from A
	for (int i = 0; i < N; ++i) {
		if (!inSubtree[i]) {
			int p = parentIndex[i];
			resultLocal[i] = localA[i];
			resultLocalQuat[i] = glm::normalize(qA[i]);
			if (p == -1) {
				resultGlobal[i] = resultLocal[i];
				resultGlobalQuat[i] = resultLocalQuat[i];
			}
			else {
				resultGlobal[i] = resultGlobal[p] * resultLocal[i];
				resultGlobalQuat[i] = glm::normalize(resultGlobalQuat[p] * resultLocalQuat[i]);
			}
		}
		else {
			resultGlobal[i] = mat4(0.0f); // mark unset
		}
	}

	// --- Top-down process subtree ---
	// Special case: the start bone uses world/global blending logic (as previously),
	// but ALL child bones (descendants) use simple local-space blending (local slerp).
	for (int idx : subtreeList) {
		vec3 blendedT;
		vec3 blendedS;
		quat finalLocalQ;

		// linear blend T/S always
		blendedT = glm::mix(tA[idx], tB[idx], progress);
		blendedS = glm::mix(sA[idx], sB[idx], progress);

		if (idx == startIdx) {
			// Start bone: perform world/global blending logic (allow mixing between local-mode and world-mode
			// controlled by UseWorldSpaceRotation).
			// local-mode quaternion:
			quat qa = glm::normalize(qA[idx]);
			quat qb = glm::normalize(qB[idx]);
			if (glm::dot(qa, qb) < 0.0f) qb = -qb;
			quat localModeQ = glm::normalize(glm::slerp(qa, qb, progress));


			if (UseWorldSpaceRotation <= 0.005f) {
				finalLocalQ = localModeQ;
			}
			else {
				// compute blended global quaternion (shortest-path)
				quat gqa = globalQA[idx];
				quat gqb = globalQB[idx];
				if (glm::dot(gqa, gqb) < 0.0f) gqb = -gqb;
				quat blendedGlobalQ = glm::normalize(glm::slerp(gqa, gqb, progress));

				int p = parentIndex[idx];
				quat parentResultQ = (p == -1) ? quat(1.0f, 0.0f, 0.0f, 0.0f) : resultGlobalQuat[p];

				quat invParent = glm::inverse(parentResultQ);
				quat worldModeQ = glm::normalize(invParent * blendedGlobalQ);

				if (UseWorldSpaceRotation >= 0.995f) {
					finalLocalQ = worldModeQ;
				}
				else {
					quat lm = localModeQ;
					quat wm = worldModeQ;
					if (glm::dot(lm, wm) < 0.0f) wm = -wm;
					finalLocalQ = glm::normalize(glm::slerp(lm, wm, UseWorldSpaceRotation));
				}
			}
		}
		else {
			// Child bones: SIMPLE local blend only (local slerp), do NOT compute/extract global rotations.
			quat qa = glm::normalize(qA[idx]);
			quat qb = glm::normalize(qB[idx]);
			if (glm::dot(qa, qb) < 0.0f) qb = -qb;
			finalLocalQ = glm::normalize(glm::slerp(qa, qb, progress));
		}

		// Build local matrix and update result arrays
		mat4 newLocal = ComposeLocal(blendedT, blendedS, finalLocalQ);
		resultLocal[idx] = newLocal;
		resultLocalQuat[idx] = glm::normalize(finalLocalQ);

		int p = parentIndex[idx];
		if (p == -1) {
			resultGlobal[idx] = newLocal;
			resultGlobalQuat[idx] = resultLocalQuat[idx];
		}
		else {
			resultGlobal[idx] = resultGlobal[p] * newLocal;
			resultGlobalQuat[idx] = glm::normalize(resultGlobalQuat[p] * resultLocalQuat[idx]);
		}
	}

	// --- Build output pose ---
	AnimationPose out;
	out.boneTransforms.reserve(N);
	for (int i = 0; i < N; ++i) out.boneTransforms[traversal[i]] = resultLocal[i];

	// keep previous behavior: copy root from B if present
	auto rootItB = poseB.boneTransforms.find(hashed_string("root"));
	if (rootItB != poseB.boneTransforms.end()) out.boneTransforms[hashed_string("root")] = rootItB->second;

	return out;
}





AnimationPose AnimationPose::ApplyFABRIK(
	const hashed_string& chainRootName,
	const hashed_string& endEffectorName,
	roj::BoneNode* rootNode,
	const AnimationPose& inputPose,
	const glm::vec3& targetPosition, // model space position
	const glm::quat& targetRotation, // model space rotation
	bool snapRotation,
	int maxIterations,
	float tolerance)
{
	using glm::mat4;
	using glm::vec3;
	using glm::quat;

	// Fast guard
	if (!rootNode) return inputPose;

	// Helpers (same as LayeredLerp)
	auto DecomposeLocal = [](const mat4& m, vec3& outT, vec3& outS, quat& outQ) {
		outT = vec3(m[3]);
		for (int i = 0; i < 3; ++i) outS[i] = glm::length(vec3(m[i]));
		glm::mat3 r;
		for (int i = 0; i < 3; ++i) {
			if (outS[i] != 0.0f) r[i] = vec3(m[i]) / outS[i];
			else r[i] = vec3(m[i]);
		}
		outQ = glm::quat_cast(r);
		};

	auto ComposeLocal = [](const vec3& t, const vec3& s, const quat& q) -> mat4 {
		glm::mat3 r = glm::mat3_cast(q);
		glm::mat3 sm(s.x, 0.0f, 0.0f,
			0.0f, s.y, 0.0f,
			0.0f, 0.0f, s.z);
		glm::mat3 rs = r * sm;
		mat4 out = mat4(rs);
		out[3] = glm::vec4(t, 1.0f);
		return out;
		};

	auto ExtractQuatAndScale = [](const mat4& m, quat& outQ, vec3& outS) {
		for (int i = 0; i < 3; ++i) outS[i] = glm::length(vec3(m[i]));
		glm::mat3 r;
		for (int i = 0; i < 3; ++i) {
			if (outS[i] != 0.0f) r[i] = vec3(m[i]) / outS[i];
			else r[i] = vec3(m[i]);
		}
		outQ = glm::quat_cast(r);
		};

	auto rotationBetweenVectors = [](const vec3& a, const vec3& b) -> quat {
		vec3 v1 = glm::normalize(a);
		vec3 v2 = glm::normalize(b);
		float d = glm::dot(v1, v2);
		if (d >= 1.0f - 1e-6f) {
			return quat(1.0f, 0.0f, 0.0f, 0.0f); // identity
		}
		if (d <= -1.0f + 1e-6f) {
			// 180 degree rotation: pick an orthogonal axis
			vec3 orth = glm::cross(vec3(1, 0, 0), v1);
			if (glm::length2(orth) < 1e-6f) orth = glm::cross(vec3(0, 1, 0), v1);
			orth = glm::normalize(orth);
			return glm::angleAxis(glm::pi<float>(), orth);
		}
		vec3 c = glm::cross(v1, v2);
		quat q;
		q.w = 1.0f + d;
		q.x = c.x;
		q.y = c.y;
		q.z = c.z;
		q = glm::normalize(q);
		return q;
		};

	// --- Get or build cached skeleton arrays as in LayeredLerp ---
	std::vector<hashed_string> traversal;
	std::unordered_map<hashed_string, int> indexMap;
	std::vector<int> parentIndex;
	std::vector<std::vector<int>> children;

	{
		std::shared_lock<std::shared_mutex> rlock(s_cacheMutex);
		auto it = s_indexMap.find(rootNode);
		if (it != s_indexMap.end()) {
			traversal = s_traversalMap[rootNode];
			indexMap = it->second;
			parentIndex = s_parentIndexMap[rootNode];
			children = s_childrenMap[rootNode];
		}
	}

	if (indexMap.empty()) {
		std::unique_lock<std::shared_mutex> wlock(s_cacheMutex);
		auto it2 = s_indexMap.find(rootNode);
		if (it2 == s_indexMap.end()) {
			std::vector<hashed_string> tmpTraversal; tmpTraversal.reserve(256);
			std::unordered_map<hashed_string, hashed_string> tempParent;
			std::function<void(roj::BoneNode*, const hashed_string*)> build = [&](roj::BoneNode* n, const hashed_string* parent) {
				if (!n) return;
				tmpTraversal.push_back(n->name);
				if (parent) tempParent[n->name] = *parent;
				for (auto& c : n->children) build(const_cast<roj::BoneNode*>(&c), &n->name);
				};
			build(rootNode, nullptr);
			int N = (int)tmpTraversal.size();
			std::unordered_map<hashed_string, int> tmpIndex; tmpIndex.reserve(N * 2);
			for (int i = 0; i < N; ++i) tmpIndex[tmpTraversal[i]] = i;
			std::vector<int> tmpParentIndex(N, -1);
			for (int i = 0; i < N; ++i) {
				auto pit = tempParent.find(tmpTraversal[i]);
				if (pit != tempParent.end()) tmpParentIndex[i] = tmpIndex[pit->second];
			}
			std::vector<std::vector<int>> tmpChildren(N);
			for (int i = 0; i < N; ++i) {
				int p = tmpParentIndex[i];
				if (p >= 0) tmpChildren[p].push_back(i);
			}
			s_traversalMap[rootNode] = std::move(tmpTraversal);
			s_indexMap[rootNode] = std::move(tmpIndex);
			s_parentIndexMap[rootNode] = std::move(tmpParentIndex);
			s_childrenMap[rootNode] = std::move(tmpChildren);
			traversal = s_traversalMap[rootNode];
			indexMap = s_indexMap[rootNode];
			parentIndex = s_parentIndexMap[rootNode];
			children = s_childrenMap[rootNode];
		}
		else {
			traversal = s_traversalMap[rootNode];
			indexMap = it2->second;

			parentIndex = s_parentIndexMap[rootNode];
			children = s_childrenMap[rootNode];
		}
	}

	int N = (int)traversal.size();
	if (N == 0) return inputPose;

	// --- Build local matrices from inputPose and global transforms ---
	std::vector<mat4> localInput(N, mat4(1.0f));
	for (int i = 0; i < N; ++i) {
		const hashed_string& name = traversal[i];
		auto it = inputPose.boneTransforms.find(name);
		if (it != inputPose.boneTransforms.end()) localInput[i] = it->second;
	}

	// compute globalInput (parent-before-child)
	std::vector<mat4> globalInput(N, mat4(1.0f));
	for (int i = 0; i < N; ++i) {
		int p = parentIndex[i];
		if (p == -1) globalInput[i] = localInput[i];
		else globalInput[i] = globalInput[p] * localInput[i];
	}

	// find chain indices
	auto itRoot = indexMap.find(chainRootName);
	auto itEnd = indexMap.find(endEffectorName);
	if (itRoot == indexMap.end() || itEnd == indexMap.end()) {
		// can't build chain, return inputPose
		return inputPose;
	}

	int rootIdx = itRoot->second;
	int endIdx = itEnd->second;

	// walk from end up to rootIdx, collecting indices
	std::vector<int> chainRev; // from end to root
	int cur = endIdx;
	bool found = false;
	while (cur != -1) {
		chainRev.push_back(cur);
		if (cur == rootIdx) { found = true; break; }
		cur = parentIndex[cur];
	}
	if (!found) {
		// end is not descendant of chainRoot
		return inputPose;
	}

	// reverse to have root -> end
	std::vector<int> chain;
	chain.reserve(chainRev.size());
	for (auto it = chainRev.rbegin(); it != chainRev.rend(); ++it) chain.push_back(*it);

	int M = (int)chain.size();
	if (M < 2) {
		// nothing to move
		return inputPose;
	}

	// prepare positions (global space), original global quats and scales
	std::vector<vec3> positions(M);
	std::vector<quat> originalGlobalQ(M);
	std::vector<vec3> originalGlobalS(M);
	for (int i = 0; i < M; ++i) {
		int idx = chain[i];
		positions[i] = vec3(globalInput[idx][3]);
		ExtractQuatAndScale(globalInput[idx], originalGlobalQ[i], originalGlobalS[i]);
	}

	// lengths between joints
	std::vector<float> lengths(M - 1);
	float totalLength = 0.0f;
	for (int i = 0; i < M - 1; ++i) {
		float d = glm::length(positions[i + 1] - positions[i]);
		lengths[i] = d;
		totalLength += d;
	}

	// distance root->target
	float distRootToTarget = glm::length(targetPosition - positions[0]);

	// FABRIK
	if (distRootToTarget > totalLength) {
		// unreachable: stretch towards target from root
		for (int i = 0; i < M - 1; ++i) {
			vec3 dir = glm::normalize(targetPosition - positions[i]);
			if (!glm::isnan(dir.x) && !glm::isnan(dir.y) && !glm::isnan(dir.z))
				positions[i + 1] = positions[i] + dir * lengths[i];
			else
				positions[i + 1] = positions[i]; // fallback
		}
	}
	else {
		// reachable: iterate
		vec3 rootPos = positions[0];
		for (int iter = 0; iter < maxIterations; ++iter) {
			// forward: set end to target
			positions[M - 1] = targetPosition;
			for (int i = M - 2; i >= 0; --i) {
				float r = glm::length(positions[i + 1] - positions[i]);
				if (r <= 1e-6f) continue;
				float lambda = lengths[i] / r;
				positions[i] = (1.0f - lambda) * positions[i + 1] + lambda * positions[i];
			}
			// backward: set root back
			positions[0] = rootPos;
			for (int i = 0; i < M - 1; ++i) {
				float r = glm::length(positions[i + 1] - positions[i]);
				if (r <= 1e-6f) continue;
				float lambda = lengths[i] / r;
				positions[i + 1] = (1.0f - lambda) * positions[i] + lambda * positions[i + 1];
			}
			// check tolerance on end effector
			float err = glm::length(positions[M - 1] - targetPosition);
			if (err <= tolerance) break;
		}
	}

	// Prepare result arrays: resultLocal (local transforms), resultGlobal (global transforms)
	std::vector<mat4> resultLocal(N);
	std::vector<mat4> resultGlobal(N);

	// initialize resultLocal to input local; resultGlobal to input global for non-chain bones
	for (int i = 0; i < N; ++i) resultLocal[i] = localInput[i];
	for (int i = 0; i < N; ++i) resultGlobal[i] = globalInput[i];

	// We'll update chain joints' global transforms and then compute corresponding local transforms.
	// For computing rotations, use originalGlobalQ and align child direction -> new direction, end effector uses targetRotation.
	// Build a mapping from chain index i -> traversal index idx
	for (int i = 0; i < M; ++i) {
		int traversalIdx = chain[i];
		// compute desired global quaternion
		quat desiredGlobalQ;
		vec3 desiredScale = originalGlobalS[i]; // preserve original global scale
		if (i < M - 1) {
			// has child in chain
			vec3 origChildDir = positions[i + 1] - positions[i];
			vec3 origChildDirFromInput = vec3(globalInput[chain[i + 1]][3]) - vec3(globalInput[chain[i]][3]);
			if (glm::length2(origChildDirFromInput) < 1e-8f) {
				// if original child dir is degenerate, fall back to original global quaternion
				desiredGlobalQ = originalGlobalQ[i];
			}
			else {
				vec3 a = origChildDirFromInput;
				vec3 b = origChildDir;
				if (glm::length2(b) < 1e-8f) {
					// no meaningful new direction; keep original orientation
					desiredGlobalQ = originalGlobalQ[i];
				}
				else {
					quat delta = rotationBetweenVectors(a, b);
					desiredGlobalQ = glm::normalize(delta * originalGlobalQ[i]);
				}
			}
		}
		else {

			if (snapRotation)
			{
				// end effector: set to targetRotation (explicit)
				desiredGlobalQ = targetRotation;
			}

		}

		// compose new global matrix for this joint
		mat4 newGlobal = ComposeLocal(positions[i], desiredScale, desiredGlobalQ);
		resultGlobal[traversalIdx] = newGlobal;
	}

	// Now convert globals back to locals for chain nodes (local = inverse(parentGlobal) * newGlobal)
	for (int i = 0; i < M; ++i) {
		int idx = chain[i];
		int p = parentIndex[idx];
		mat4 parentGlobal = (p == -1) ? mat4(1.0f) : resultGlobal[p];
		// compute local = inverse(parentGlobal) * resultGlobal[idx]
		mat4 invParent = glm::inverse(parentGlobal);
		mat4 newLocal = invParent * resultGlobal[idx];
		resultLocal[idx] = newLocal;
	}

	// Build output pose
	AnimationPose out;
	out.boneTransforms.reserve(N);

	for (int i = 0; i < N; ++i) {
		out.boneTransforms[traversal[i]] = resultLocal[i];
	}

	// Copy any bones in inputPose that are not part of traversal (unlikely) to preserve extras
	for (const auto& kv : inputPose.boneTransforms) {
		// if traversal doesn't have it, keep as-is
		// but traversal covers all bones from rootNode build, so this is defensive
		if (indexMap.find(kv.first) == indexMap.end()) {
			out.boneTransforms[kv.first] = kv.second;
		}
	}

	return out;
}

glm::mat4 AnimationPose::GetModelSpaceTransform(
	const hashed_string& boneName,
	roj::BoneNode* rootNode,
	const AnimationPose& pose)
{
	using glm::mat4;

	if (!rootNode) return mat4(1.0f);

	// --- read or build cached skeleton arrays (same pattern as LayeredLerp) ---
	std::vector<hashed_string> traversal;
	std::unordered_map<hashed_string, int> indexMap;
	std::vector<int> parentIndex;
	std::vector<std::vector<int>> children;

	{
		std::shared_lock<std::shared_mutex> rlock(s_cacheMutex);
		auto it = s_indexMap.find(rootNode);
		if (it != s_indexMap.end()) {
			traversal = s_traversalMap[rootNode];
			indexMap = it->second;
			parentIndex = s_parentIndexMap[rootNode];
			children = s_childrenMap[rootNode];
		}
	}

	if (indexMap.empty()) {
		std::unique_lock<std::shared_mutex> wlock(s_cacheMutex);
		auto it2 = s_indexMap.find(rootNode);
		if (it2 == s_indexMap.end()) {
			// build traversal and parent map
			std::vector<hashed_string> tmpTraversal;
			tmpTraversal.reserve(256);
			std::unordered_map<hashed_string, hashed_string> tempParent;
			std::function<void(roj::BoneNode*, const hashed_string*)> build = [&](roj::BoneNode* n, const hashed_string* parent) {
				if (!n) return;
				tmpTraversal.push_back(n->name);
				if (parent) tempParent[n->name] = *parent;
				for (auto& c : n->children) build(const_cast<roj::BoneNode*>(&c), &n->name);
				};
			build(rootNode, nullptr);

			int N = (int)tmpTraversal.size();
			std::unordered_map<hashed_string, int> tmpIndex; tmpIndex.reserve(N * 2);
			for (int i = 0; i < N; ++i) tmpIndex[tmpTraversal[i]] = i;

			std::vector<int> tmpParentIndex(N, -1);
			for (int i = 0; i < N; ++i) {
				auto pit = tempParent.find(tmpTraversal[i]);
				if (pit != tempParent.end()) tmpParentIndex[i] = tmpIndex[pit->second];
			}

			std::vector<std::vector<int>> tmpChildren(N);
			for (int i = 0; i < N; ++i) {
				int p = tmpParentIndex[i];
				if (p >= 0) tmpChildren[p].push_back(i);
			}

			s_traversalMap[rootNode] = std::move(tmpTraversal);
			s_indexMap[rootNode] = std::move(tmpIndex);
			s_parentIndexMap[rootNode] = std::move(tmpParentIndex);
			s_childrenMap[rootNode] = std::move(tmpChildren);

			traversal = s_traversalMap[rootNode];
			indexMap = s_indexMap[rootNode];
			parentIndex = s_parentIndexMap[rootNode];
			children = s_childrenMap[rootNode];
		}
		else {
			traversal = s_traversalMap[rootNode];
			indexMap = it2->second;
			parentIndex = s_parentIndexMap[rootNode];
			children = s_childrenMap[rootNode];
		}
	}

	// --- find bone index ---
	auto itBone = indexMap.find(boneName);
	if (itBone == indexMap.end()) {
		return mat4(1.0f); // bone not found
	}
	int targetIdx = itBone->second;

	// --- collect chain root->target indices (by following parents) ---
	std::vector<int> chain;
	int cur = targetIdx;
	while (cur != -1) {
		chain.push_back(cur);
		cur = parentIndex[cur];
	}
	// chain currently target->...->root, reverse to root->...->target
	std::reverse(chain.begin(), chain.end());

	// --- multiply local matrices along chain to produce global transform ---
	mat4 global = mat4(1.0f);
	for (int idx : chain) {
		const hashed_string& name = traversal[idx];
		auto itLocal = pose.boneTransforms.find(name);
		mat4 local = (itLocal != pose.boneTransforms.end()) ? itLocal->second : mat4(1.0f);
		global = global * local; // parentGlobal * local -> childGlobal
	}

	return global;
}

AnimationPose SkeletalMesh::GetAnimationPose()
{
	if (model == nullptr) return AnimationPose();

	if (dirtyPose == false)
		return lastPose;

	AnimationPose pose;
	pose.boneTreeRoot = &model->defaultRoot;
	pose.boneTransforms = animator.GetBonePoseArray();

	lastPose = pose;
	dirtyPose = false;
	return pose;
}

void SkeletalMesh::ApplyWorldSpaceBoneTransforms(std::unordered_map<hashed_string, mat4>& pose)
{

	mat4 world = GetWorldMatrix();

	for (auto& bonePose : pose)
	{
		pose[bonePose.first] = inverse(world) * pose[bonePose.first];
	}

	animator.ApplyLocalSpacePoseArray(GetAnimationPose().boneTransforms, pose);
	boneTransforms = animator.getBoneMatrices();

	dirtyPose = true;

}

void SkeletalMesh::PlayAnimation(std::string name, bool Loop, float interpIn)
{

	if (model == nullptr) return;

	SetLooped(Loop);
	animator.set(name);
	PlayAnimation(interpIn);
	oldRootMotionPos = vec3();
	oldRootMotionRot = vec3();
	animator.totalRootMotionPosition = vec3();
	animator.totalRootMotionRotation = vec3();
	animator.oldRootBoneTransform = MathHelper::Transform();
	rootMotionBasisQuat = quat();
	currentAnimationData = GetAnimationDataFromName(name);	
	oldAnimationEventTime = -1;
	Update(0.0001f);
	boneTransforms = animator.getBoneMatrices();
}

void SkeletalMesh::StopAnimation()
{
	SetLooped(false);
	animator.set("");
	animator.m_playing = false;
	currentAnimationData = nullptr;
}

std::string SkeletalMesh::GetAnimationName()
{

	if (currentAnimationData == nullptr) return "";

	return currentAnimationData->animationName;

}

void SkeletalMesh::SetAnimationPaused(bool value)
{

	animator.m_playing = !value;

}

bool SkeletalMesh::GetAnimationPaused()
{
	return !animator.m_playing;
}

bool SkeletalMesh::IsCameraVisible()
{
	if (InRagdoll == false)
	{
		return StaticMesh::IsCameraVisible();
	}

	mat4 world = GetWorldMatrix();

	vector<vec3> bonePositions;
	bonePositions.reserve(hitboxBodies.size());

	for (auto hitboxBody : hitboxBodies)
	{
		bonePositions.push_back(FromPhysics(hitboxBody->GetPosition()));
	}

	auto box = BoundingBox::FromPoints(bonePositions);

	box.Max += vec3(1);
	box.Max -= vec3(1);

	if (Level::Current->BspData.m_numOfVerts)
	{

		int cameraC = Level::Current->BspData.FindClusterAtPosition(Camera::finalizedPosition);
		int targetC = Level::Current->BspData.FindClusterAtPosition(box.Center() + vec3(0,1,0));

		

		if (Level::Current->BspData.IsClusterVisible(cameraC, targetC) == false)
		{

			//DebugDraw::Line(box.Center(), Camera::position - vec3(0, 1, 0));

			return false;
		}
	}
	return IsInFrustrum(Camera::frustum) && isVisible();
}

BoundingBox SkeletalMesh::GetBoundingBox()
{
	if (InRagdoll == false)
	{
		return StaticMesh::GetBoundingBox();
	}

	mat4 world = GetWorldMatrix();

	vector<vec3> bonePositions;
	bonePositions.reserve(hitboxBodies.size());

	for (auto hitboxBody : hitboxBodies)
	{

		bonePositions.push_back(FromPhysics(hitboxBody->GetPosition()));

	}

	auto box = BoundingBox::FromPoints(bonePositions);

	box.Min -= vec3(1);
	box.Max += vec3(1);

	//DebugDraw::Bounds(box.Min, box.Max, 0.01f);


	return box;
}

bool SkeletalMesh::IsInFrustrum(Frustum frustrum)
{

	if (InRagdoll == false)
	{
		return StaticMesh::IsInFrustrum(frustrum);
	}

	mat4 world = GetWorldMatrix();

	vector<vec3> bonePositions;
	bonePositions.reserve(hitboxBodies.size());
	
	for (auto hitboxBody : hitboxBodies)
	{

		bonePositions.push_back(FromPhysics(hitboxBody->GetPosition()));

	}

	auto box = BoundingBox::FromPoints(bonePositions);

	box.Min -= vec3(1);
	box.Max += vec3(1);

	//DebugDraw::Bounds(box.Min, box.Max, 0.01f);


	return frustrum.IsBoxVisible(box.Min, box.Max);

}

LightVolPointData SkeletalMesh::GetLightVolData()
{
	if (InRagdoll == false)
		return StaticMesh::GetLightVolData();

	mat4 world = GetWorldMatrix();

	vector<vec3> bonePositions;
	bonePositions.reserve(model->boneInfoMap.size());

	for (auto hitboxBody : hitboxBodies)
	{

		bonePositions.push_back(FromPhysics(hitboxBody->GetPosition()));

	}

	auto box = BoundingBox::FromPoints(bonePositions);

	if (model == nullptr ||
		Level::Current->BspData.lightVols.size() == 0) return LightVolPointData{ vec3(0),vec3(1),vec3(0) };

	vec3 samplePos = box.Center() + vec3(0, 0.5, 0);


	auto light = Level::Current->BspData.GetLightvolColorPoint(samplePos * MAP_SCALE, true);
	return light;
}

// In your SkeletalMesh:
MathHelper::Transform SkeletalMesh::PullRootMotion()
{

	// 1) grab the raw deltas from the animator
	glm::vec3 deltaPos = animator.totalRootMotionPosition - oldRootMotionPos;
	glm::vec3 deltaRot = animator.totalRootMotionRotation - oldRootMotionRot;

	// 2) stash totals for next frame:
	oldRootMotionPos = animator.totalRootMotionPosition;
	oldRootMotionRot = animator.totalRootMotionRotation;

	// 3) apply the offsets back to the skeleton (unchanged)
	positionOffset = -animator.totalRootMotionPosition;
	rotationOffset = -animator.totalRootMotionRotation ;

	// 4) build the output transform:
	MathHelper::Transform t;

	rootMotionBasisQuat = MathHelper::GetRotationQuaternion(Rotation + rotationOffset);

	// — USE the fixed “basis” quat for _all_ translation:
	//   this freezes the walk-direction in world-space
	glm::quat basis = true
		? rootMotionBasisQuat
		: MathHelper::GetRotationQuaternion(Rotation);

	t.Position = MathHelper::TransformVector(deltaPos, basis);

	// — but still spin each frame by whatever the animator gave you:
	t.Rotation = deltaRot;

	return t;
}

bool SkeletalMesh::IsAnimationPlaying()
{

	if (model == nullptr) return false;

	return animator.m_playing;
}

void SkeletalMesh::Update(float timeScale)
{

	if (model == nullptr) return;

	animator.UpdatePose = UpdatePose;

	animator.UsePrecomputedFrames = UsePrecomputedFrames;

	if (UpdatePoseOnlyWhenRendered)
	{
		animator.UpdatePose = WasRended;
	}

	animator.update(Time::DeltaTimeF * timeScale);
	UpdateAnimationEvents();

	if (animator.m_currTime != oldAnimTime)
	{
		if (animator.UpdatePose)
		{
			dirtyPose = true;
		}
	}

	oldAnimTime = animator.m_currTime;

	if (UpdatePose == false) return;

	float blendProgress = GetBlendInProgress();

	if (blendProgress < 0.99)
	{
		AnimationPose currentPose = GetAnimationPose();


		AnimationPose newPose = AnimationPose::Lerp(blendStartPose, currentPose, blendProgress);

		PasteAnimationPose(newPose);

	}

	boneTransforms = animator.getBoneMatrices();

}

float SkeletalMesh::GetAnimationDuration()
{
	if (model == nullptr) return 0;

	if (animator.m_currAnim == nullptr) return 0;


	return animator.m_currAnim->duration / animator.m_currAnim->ticksPerSec;

}

float SkeletalMesh::GetAnimationTime()
{
	if (model == nullptr) return 0;

	if (animator.m_currAnim == nullptr) return 0;

	return animator.m_currTime / animator.m_currAnim->ticksPerSec;
}

void SkeletalMesh::SetAnimationTime(float time)
{
	if (model == nullptr) return;

	if (animator.m_currAnim == nullptr) return;

	animator.m_currTime = time * animator.m_currAnim->ticksPerSec;

}

roj::BoneNode* SkeletalMesh::GetNodeFromName(const hashed_string& name)
{

	if (model == nullptr) return nullptr;

	auto nodeResult = model->boneNodesMap.find(name);

	if (nodeResult != model->boneNodesMap.end())
	{
		return &nodeResult->second;
	}

	return nullptr;
}

roj::BoneNode* SkeletalMesh::GetRootNode()
{
	if (model == nullptr) return nullptr;

	return & model->defaultRoot;

}

std::unordered_map<hashed_string, hashed_string>* SkeletalMesh::GetNodeParentMap()
{
	if (model == nullptr) return nullptr;

	return &model->parentMap;
}

void SkeletalMesh::StartedRendering()
{

	if (model == nullptr) return;

	if (InRagdoll) return;

	if (IsAnimationPlaying() == false)return;

	Update(0);

	boneTransforms = animator.getBoneMatrices();

}

mat4 SkeletalMesh::GetWorldMatrixNoOffsets()
{
	return translate(Position) * MathHelper::GetRotationMatrix(Rotation) * scale(Scale);
}

mat4 SkeletalMesh::GetBoneMatrix(string boneName)
{
	if (model == nullptr) return mat4();

	auto res = model->boneInfoMap.find(boneName);

	if(res == model->boneInfoMap.end()) return mat4();

	int id = res->second.id;

	mat4 invOffset = inverse(res->second.offset);

	return boneTransforms[id] * invOffset;

}

mat4 SkeletalMesh::GetBoneMatrixWorld(string boneName)
{
	return GetWorldMatrix() * GetBoneMatrix(boneName);
}

float SkeletalMesh::GetHitboxDamageMultiplier(string boneName)
{
	for (auto& hitbox : metaData.hitboxes)
	{
		if (hitbox.boneName == boneName)
			return hitbox.damageMultiplier;
	}

	return 1.0f;
}

void SkeletalMesh::StartRagdoll()
{

	InRagdoll = true;

	for (auto& hitbox : hitboxBodies)
	{

		Physics::SetMotionType(hitbox, JPH::EMotionType::Dynamic);
		hitbox->SetMotionType(JPH::EMotionType::Dynamic);

		Physics::SetCollisionMask(hitbox, BodyType::GroupCollisionTest & ~BodyType::CharacterCapsule | BodyType::HitBox);

		const string boneName = Physics::GetBodyData(hitbox)->hitboxName;

		vec3 linearVel = vec3();
		vec3 angularVel = vec3();

		auto linearRes = boneLinearVel.find(boneName);
		auto angularRes = boneAngularVel.find(boneName);

		if (linearRes != boneLinearVel.end())
		{
			linearVel = linearRes->second;
		}

		if (angularRes != boneAngularVel.end())
		{
			angularVel = angularRes->second;
		}

		auto constraint = GetConstraintByHitboxName(boneName);

		if(constraint != nullptr)
		{
			constraint->SetEnabled(true);
		}

		if (EngineMain::MainInstance->SimulatingGameTicks == false)
		{
			Physics::SetLinearVelocity(hitbox, linearVel / 1.5f);
			Physics::SetAngularVelocity(hitbox, angularVel / 1.5f);
		}


	}

}

void SkeletalMesh::StopRagdoll()
{
	InRagdoll = false;

	for (auto& hitbox : hitboxBodies)
	{

		Physics::SetMotionType(hitbox, JPH::EMotionType::Kinematic);

		Physics::SetCollisionMask(hitbox, BodyType::None);


	}

	for (auto constraint : hitboxConstraints)
	{
		constraint.second->SetEnabled(false);
	}

}

void SkeletalMesh::ClearHitboxes()
{
	std::lock_guard<std::recursive_mutex> lock(hitboxMutex);

	for (Body* body : hitboxBodies)
	{
		Physics::DestroyBody(body);
	}

	for (auto constraint : hitboxConstraints)
	{
		Physics::DestroyConstraint(constraint.second);
	}

	defaultBoneScale.clear();

	hitboxBodies.clear();
	hitboxConstraints.clear();
}

void SkeletalMesh::CreateHitbox(Entity* owner,HitboxData data)
{
	std::lock_guard<std::recursive_mutex> lock(hitboxMutex);

	Body* body = Physics::CreateHitBoxBody(owner, this, data.boneName, data.position, MathHelper::GetRotationQuaternion(data.rotation), data.size);

	defaultBoneScale[data.boneName] = MathHelper::DecomposeMatrix(GetBoneMatrixWorld(data.boneName)).Scale;

	hitboxBodies.push_back(body);

}

void SkeletalMesh::CreateHitboxes(Entity* owner)
{

	auto oldPose = GetAnimationPose();

	animator.ApplyBonePoseArray(std::unordered_map<hashed_string, mat4>{}); //applying rest pose without visual update

	ClearHitboxes();


	for (auto hitbox : metaData.hitboxes)
	{
		CreateHitbox(owner, hitbox);

	}

	UpdateHitboxes();

	for (auto hitbox : metaData.hitboxes)
	{

		if (hitbox.parentBone == "") continue;
		
		Body* parentBody = FindHitboxByName(hitbox.parentBone);
		Body* currentBody = FindHitboxByName(hitbox.boneName);

		if (parentBody == nullptr || currentBody == nullptr) continue;

		auto constraint = Physics::CreateRagdollConstraint(parentBody, currentBody, hitbox.twistParameters.x, hitbox.twistParameters.y, hitbox.twistParameters.z, ToPhysics(MathHelper::GetRotationQuaternion(hitbox.constraintRotation)));
		
		Physics::ConfigureSwingTwistMotor(constraint);

		constraint->SetEnabled(false);

		hitboxConstraints[hitbox.boneName] = constraint;

	}

	animator.ApplyBonePoseArray(oldPose.boneTransforms); //restoring old pose just in case

}

void SkeletalMesh::ApplyImpulseToAllHitboxes(vec3 impulse, bool scaleWithMass)
{

	if (scaleWithMass)
	{
		for (Body* body : hitboxBodies)
		{

			float invMass = body->GetMotionProperties()->GetInverseMass();
			float mass = (invMass > 0.0f) ? 1.0f / invMass : FLT_MAX;

			Physics::AddImpulse(body, impulse * mass);
		}
	}
	else
	{
		for (Body* body : hitboxBodies)
		{
			Physics::AddImpulse(body, impulse);
		}
	}

}

Constraint* SkeletalMesh::GetConstraintByHitboxName(string name)
{

	auto res = hitboxConstraints.find(name);

	if (res != hitboxConstraints.end())
	{
		return res->second;
	}

	return nullptr;
}

Body* SkeletalMesh::FindHitboxByName(string name)
{
	for (auto hitbox : hitboxBodies)
	{

		if (Physics::GetBodyData(hitbox)->hitboxName == name)
		{
			return hitbox;
		}

	}

	return nullptr;
}

void SkeletalMesh::UpdateHitboxes()
{

	if (InRagdoll)
	{

		

		if (hitboxConstraints.size() > 0)
		{



			std::unordered_map<hashed_string, mat4> animationPose;

			std::unordered_map<hashed_string, quat> hitboxRelativePose;

			bool hasInvalidTransform = false;

			for (const HitboxData& data : metaData.hitboxes)
			{

				if (data.parentBone == "") continue;

				const auto& boneName = data.boneName;

				if (hitboxConstraints.contains(boneName))
					hitboxConstraints[boneName]->SetEnabled(true);

				mat4 relativeTransform = mat4();

				if (RagdollPoseFollowStrength > 0)
				{
					mat4 parentBone = GetBoneMatrix(data.parentBone);
					mat4 childBone = GetBoneMatrix(data.boneName);

					relativeTransform = inverse(parentBone) * childBone;
				}

				quat resultQuat = normalize(quat_cast(relativeTransform));
				
				// 1) Ensure no NaNs / infinities
				if (!std::isfinite(resultQuat.x) || !std::isfinite(resultQuat.y) ||
					!std::isfinite(resultQuat.z) || !std::isfinite(resultQuat.w))
				{
					hasInvalidTransform = true;
				}

				hitboxRelativePose[boneName] = resultQuat;

			}

			if (hasInvalidTransform == false)
			{
				for (auto relHitboxPos : hitboxRelativePose)
				{

					Physics::UpdateSwingTwistMotor(hitboxConstraints[relHitboxPos.first], relHitboxPos.second, RagdollPoseFollowStrength);
				}
			}


		}
		

		std::unordered_map<hashed_string, mat4> pose;

		

		for (Body* body : hitboxBodies)
		{

			vec3 pos = FromPhysics(body->GetPosition());
			quat rot = FromPhysics(body->GetRotation());

			const auto& boneName = Physics::GetBodyData(body)->hitboxName;

			if (RagdollPoseFollowStrength > 0)
			{
				//Physics::Activate(body);
			}

			if (EngineMain::MainInstance->SimulatingGameTicks)
			{
				Physics::Deactivate(body);

				if (EngineMain::MainInstance->SimulatingPreciseGameTicks)
				{
					Physics::Activate(body);

				}

			}

			

			auto res = animator.currentPose.find(boneName);

			vec3 scale = defaultBoneScale[boneName];



			MathHelper::Transform boneTrans;

			boneTrans.Position = pos;
			boneTrans.RotationQuaternion = rot;
			boneTrans.Scale = scale;

			pose[boneName] = boneTrans.ToMatrix();


		}

		ApplyWorldSpaceBoneTransforms(pose);

		return;
	}

	std::lock_guard<std::recursive_mutex> lock(hitboxMutex);

	mat4 world = GetWorldMatrix();

	for (Body* body : hitboxBodies)
	{
		string boneName = Physics::GetBodyData(body)->hitboxName;

		if(hitboxConstraints.contains(boneName))
			hitboxConstraints[boneName]->SetEnabled(false);

		MathHelper::Transform boneTrans = MathHelper::DecomposeMatrix(world * GetBoneMatrix(boneName));

		vec3 oldPos = FromPhysics(body->GetPosition());
		vec3 newPos = boneTrans.Position;

		quat oldRot = FromPhysics(body->GetRotation());
		quat newRot = boneTrans.RotationQuaternion;

		vec3 linearVelocity = (newPos - oldPos) / Time::DeltaTimeF;

		quat deltaRot = newRot * glm::inverse(oldRot);
		deltaRot = glm::normalize(deltaRot);

		vec3 angularVelocity = (2.0f / Time::DeltaTimeF) * vec3(deltaRot.x, deltaRot.y, deltaRot.z);

		boneLinearVel[boneName] = linearVelocity;
		boneAngularVel[boneName] = angularVelocity;

		Physics::SetBodyPositionAndRotation(body, boneTrans.Position, boneTrans.RotationQuaternion);
	}

}

void SkeletalMesh::UpdateAnimationEvents()
{
	if (currentAnimationData == nullptr) return;

	float currentAnimationTime = GetAnimationTime();

	if (currentAnimationTime == oldAnimationEventTime) return;

	if (oldAnimationEventTime > currentAnimationTime)
	{
		oldAnimationEventTime = 0;
	}

	vector<AnimationEvent> pendingEvents;

	for (const auto& event : currentAnimationData->animationEvents)
	{
		if (event.time > oldAnimationEventTime && event.time <= currentAnimationTime)
		{
			pendingEvents.push_back(event);
		}
	}

	// Sort events from earliest to latest by time
	std::sort(pendingEvents.begin(), pendingEvents.end(), [](const AnimationEvent& a, const AnimationEvent& b) {
		return a.time < b.time;
		});

	// Push sorted events to the queue
	for (const auto& event : pendingEvents)
	{
		pendingAnimationEvents.push_back(event);
	}

	oldAnimationEventTime = currentAnimationTime;
}

vector<AnimationEvent> SkeletalMesh::PullAnimationEvents()
{
	vector<AnimationEvent> result = pendingAnimationEvents;
	pendingAnimationEvents.clear();
	return result;

}



void SkeletalMesh::ClearMetaDataCache()
{
	loaded_metas.clear();
}

void SkeletalMesh::SaveMetaToFile()
{
	if (model == nullptr) return;

	string metaFilePath = filePath + ".skmm";

	json jsonData = json(metaData);
	string content = jsonData.dump(4);

	try {
		auto parent = std::filesystem::path(metaFilePath).parent_path();
		if (!parent.empty() && !std::filesystem::exists(parent)) {
			std::filesystem::create_directories(parent);
		}
	}
	catch (const std::exception& e) {
		// log or throw
	}
	std::ofstream ofs(metaFilePath, std::ios::out 
		| std::ios::binary 
		| std::ios::trunc);
	if (!ofs) return;
	ofs.write(content.data(), content.size());

}

void SkeletalMesh::LoadMetaFromFile()
{
	if (model == nullptr) return;

	string metaFilePath = filePath + ".skmm";

	
	LoadMetaFromFile(metaFilePath);

}

void SkeletalMesh::LoadMetaFromFile(const std::string& path)
{

	if (invalid_meta_files.find(path) != invalid_meta_files.end())
	{
		return;
	}

	auto foundData = loaded_metas.find(path);

	SkeletalMeshMetaData data;

	if (foundData != loaded_metas.end())
	{
		data = foundData->second;
	}
	else
	{
		string file = AssetRegistry::ReadFileToString(path);

		if (file.size() < 3)
		{

			invalid_meta_files.insert(path);
			return;
		}


		json jsonData = json::parse(file);

		data = jsonData.get<SkeletalMeshMetaData>();

		loaded_metas[path] = data;

	}

	metaData = data;
}

AnimationData* SkeletalMesh::GetAnimationDataFromName(std::string name)
{
	for (AnimationData& data : metaData.animations)
	{
		if (data.animationName == name)
		{
			return &data;
		}
	}

	metaData.animations.push_back(AnimationData{ name});
	return GetAnimationDataFromName(name);
}

void SkeletalMesh::SetAnimationState(const AnimationState& animationState)
{

	if (animationState.animationName != "")
	{
		PlayAnimation(animationState.animationName, animationState.looping, 0);
	}
	else
	{
		PasteAnimationPose(AnimationPose()); //setting to rest pose
	}

	animator.m_currTime = animationState.animationTime;
	animator.UpdateAnimationPose();
	animator.m_playing = animationState.playing;
	oldAnimationEventTime = animationState.oldAnimationEventTime;
	Update(0);
	PullAnimationEvents();

	InRagdoll = animationState.inRagdoll;

	if (InRagdoll)
	{

		StartRagdoll();

		for (auto hitboxBody : hitboxBodies)
		{


			Physics::SetLinearVelocity(hitboxBody, vec3());
			Physics::SetAngularVelocity(hitboxBody, vec3());

			string hitboxName = Physics::GetBodyData(hitboxBody)->hitboxName;

			auto posRes = animationState.ragdollHitboxPositions.find(hitboxName);
			auto rotRes = animationState.ragdollHitboxRotations.find(hitboxName);

			auto posVelRes = animationState.ragdollHitboxLinearVelocty.find(hitboxName);
			auto rotVelRes = animationState.ragdollHitboxAngularVelocty.find(hitboxName);

			if (posRes != animationState.ragdollHitboxPositions.end())
			{
				Physics::SetBodyPosition(hitboxBody, posRes->second);
			}

			if (rotRes != animationState.ragdollHitboxRotations.end())
			{
				Physics::SetBodyRotation(hitboxBody, rotRes->second);
			}

			if (posVelRes != animationState.ragdollHitboxLinearVelocty.end())
			{
				Physics::SetLinearVelocity(hitboxBody, posVelRes->second);
			}

			if (rotVelRes != animationState.ragdollHitboxAngularVelocty.end())
			{
				Physics::SetAngularVelocity(hitboxBody, rotVelRes->second);
			}


		}
	}

}

AnimationState SkeletalMesh::GetAnimationState()
{
	AnimationState animationState;

	animationState.looping = animator.Loop;
	animationState.animationName = animator.currentAnimationName;
	animationState.animationTime = animator.m_currTime;
	animationState.playing = animator.m_playing;
	animationState.oldAnimationEventTime = oldAnimationEventTime;

	unordered_map<string, vec3> hitboxPositions;
	unordered_map<string, quat> hitboxRotations;

	unordered_map<string, vec3> hitboxPositionsVel;
	unordered_map<string, vec3> hitboxRotationsVel;

	if (InRagdoll)
	{
		for (auto hitboxBody : hitboxBodies)
		{

			string hitboxName = Physics::GetBodyData(hitboxBody)->hitboxName;

			hitboxPositions[hitboxName] = FromPhysics(hitboxBody->GetPosition());
			hitboxRotations[hitboxName] = FromPhysics(hitboxBody->GetRotation());

			hitboxPositionsVel[hitboxName] = FromPhysics(hitboxBody->GetLinearVelocity());
			hitboxRotationsVel[hitboxName] = FromPhysics(hitboxBody->GetAngularVelocity());

		}
	}

	animationState.inRagdoll = InRagdoll;
	animationState.ragdollHitboxPositions = hitboxPositions;
	animationState.ragdollHitboxRotations = hitboxRotations;

	animationState.ragdollHitboxLinearVelocty = hitboxPositionsVel;
	animationState.ragdollHitboxAngularVelocty = hitboxRotationsVel;

	return animationState;
}
