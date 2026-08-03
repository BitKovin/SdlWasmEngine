#include <SkeletalMesh.hpp>

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
		auto& out = result.boneTransforms;

		// Insert bones that only exist in B (no interpolation, same as slow path)
		for (const auto& [bone, bMat] : b.boneTransforms)
		{
			if (out.find(bone) == out.end())
				out[bone] = bMat;
		}

		// Root always comes from B
		auto bRootIt = b.boneTransforms.find("root");
		if (bRootIt != b.boneTransforms.end())
			out["root"] = bRootIt->second;

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


struct SkeletonTopology
{
	std::vector<hashed_string> traversal;                     // preorder bone order
	std::unordered_map<hashed_string, int> indexMap;           // name -> index in traversal
	std::vector<int> parentIndex;                               // -1 for root
	std::vector<int> subtreeEnd;                                // inclusive end of subtree range
};

static std::unordered_map<roj::BoneNode*, std::shared_ptr<const SkeletonTopology>> s_topologyCache;
static std::shared_mutex s_topologyMutex;

static std::shared_ptr<const SkeletonTopology> GetOrBuildTopology(roj::BoneNode* rootNode)
{
	{
		std::shared_lock<std::shared_mutex> rlock(s_topologyMutex);
		auto it = s_topologyCache.find(rootNode);
		if (it != s_topologyCache.end()) return it->second;
	}

	std::unique_lock<std::shared_mutex> wlock(s_topologyMutex);
	auto existing = s_topologyCache.find(rootNode);
	if (existing != s_topologyCache.end()) return existing->second; // built by another thread while we waited

	auto topo = std::make_shared<SkeletonTopology>();
	topo->traversal.reserve(256);

	std::unordered_map<hashed_string, hashed_string> tempParent;
	std::function<void(roj::BoneNode*, const hashed_string*)> build =
		[&](roj::BoneNode* n, const hashed_string* parent) {
		if (!n) return;
		topo->traversal.push_back(n->name);
		if (parent) tempParent[n->name] = *parent;
		for (auto& c : n->children) build(const_cast<roj::BoneNode*>(&c), &n->name);
		};
	build(rootNode, nullptr);

	const int N = (int)topo->traversal.size();
	topo->indexMap.reserve(N * 2);
	for (int i = 0; i < N; ++i) topo->indexMap[topo->traversal[i]] = i;

	topo->parentIndex.assign(N, -1);
	for (int i = 0; i < N; ++i) {
		auto pit = tempParent.find(topo->traversal[i]);
		if (pit != tempParent.end()) topo->parentIndex[i] = topo->indexMap[pit->second];
	}

	// subtreeEnd[i] starts as i, then each node pushes its own subtreeEnd up
	// to its parent. Walking indices high-to-low guarantees a node's own
	// descendants are already folded in before it propagates further up.
	topo->subtreeEnd.resize(N);
	for (int i = 0; i < N; ++i) topo->subtreeEnd[i] = i;
	for (int i = N - 1; i >= 1; --i) {
		int p = topo->parentIndex[i];
		if (topo->subtreeEnd[i] > topo->subtreeEnd[p]) topo->subtreeEnd[p] = topo->subtreeEnd[i];
	}

	s_topologyCache[rootNode] = topo;
	return topo;
}

// Call once per skeleton right after the model finishes loading (e.g. from
// wherever the BoneNode hierarchy gets constructed/parsed). Purely an
// optimization - without this call, the first LayeredLerp for that skeleton
// just builds and caches the topology itself on demand.
void PrewarmSkeletonTopology(roj::BoneNode* rootNode)
{
	if (rootNode) GetOrBuildTopology(rootNode);
}

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

	// --- Helpers (unchanged from the original) ---
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

	// Rotation-only decompose - for the ancestor chain, where T/S are never
	// needed and would otherwise just be thrown away immediately.
	auto DecomposeRotation = [&](const mat4& m) -> quat {
		vec3 dummyT, dummyS;
		quat q;
		DecomposeLocal(m, dummyT, dummyS, q);
		return q;
		};

	// --- Topology: one cheap shared_ptr fetch, no per-call container copying ---
	std::shared_ptr<const SkeletonTopology> topo = GetOrBuildTopology(rootNode);
	const int N = (int)topo->traversal.size();
	if (N == 0) return poseA;

	auto itStart = topo->indexMap.find(startBoneName);
	if (itStart == topo->indexMap.end()) {
		AnimationPose out = poseA;
		auto rb = poseB.boneTransforms.find(hashed_string("root"));
		if (rb != poseB.boneTransforms.end()) out.boneTransforms[hashed_string("root")] = rb->second;
		return out;
	}
	const int startIdx = itStart->second;
	const int subEnd = topo->subtreeEnd[startIdx]; // contiguous [startIdx, subEnd], thanks to preorder layout

	// Seed the output with pose A verbatim. Every bone outside the blend
	// subtree keeps its exact pose-A matrix - no decompose/recompose round
	// trip, and no risk of EPS_SCALE silently clamping an untouched bone's
	// near-zero scale axis to 1.0 (which the original did to every bone,
	// including ones nothing here is actually blending).
	AnimationPose out = poseA;

	// Ancestor chain (root .. parent(startIdx)), only walked/decomposed when
	// world-space blending is actually in play for this call.
	const bool needWorldChain = (UseWorldSpaceRotation > 0.005f);
	quat parentResultQ(1.0f, 0.0f, 0.0f, 0.0f); // product of pose-A rotations, root -> parent(startIdx)
	quat gqbChain(1.0f, 0.0f, 0.0f, 0.0f);      // product of pose-B rotations, root -> parent(startIdx)

	if (needWorldChain) {
		std::vector<int> ancestors;
		ancestors.reserve(32);
		for (int p = topo->parentIndex[startIdx]; p != -1; p = topo->parentIndex[p]) ancestors.push_back(p);
		for (auto it = ancestors.rbegin(); it != ancestors.rend(); ++it) { // walk root-first
			const hashed_string& name = topo->traversal[*it];
			quat qa(1.0f, 0.0f, 0.0f, 0.0f), qb(1.0f, 0.0f, 0.0f, 0.0f);
			auto ita = poseA.boneTransforms.find(name);
			auto itb = poseB.boneTransforms.find(name);
			if (ita != poseA.boneTransforms.end()) qa = DecomposeRotation(ita->second);
			if (itb != poseB.boneTransforms.end()) qb = DecomposeRotation(itb->second);
			parentResultQ = glm::normalize(parentResultQ * qa);
			gqbChain = glm::normalize(gqbChain * qb);
		}
	}

	// Blend the start bone + its full subtree - a single contiguous range.
	for (int idx = startIdx; idx <= subEnd; ++idx) {
		const hashed_string& name = topo->traversal[idx];

		mat4 mA(1.0f), mB(1.0f);
		auto ita = poseA.boneTransforms.find(name);
		auto itb = poseB.boneTransforms.find(name);
		if (ita != poseA.boneTransforms.end()) mA = ita->second;
		if (itb != poseB.boneTransforms.end()) mB = itb->second;

		vec3 t_A, s_A, t_B, s_B;
		quat q_A, q_B;
		DecomposeLocal(mA, t_A, s_A, q_A);
		DecomposeLocal(mB, t_B, s_B, q_B);

		vec3 blendedT = glm::mix(t_A, t_B, progress);
		vec3 blendedS = glm::mix(s_A, s_B, progress);
		quat finalLocalQ;

		if (idx == startIdx) {
			quat qa = glm::normalize(q_A);
			quat qb = glm::normalize(q_B);
			if (glm::dot(qa, qb) < 0.0f) qb = -qb;
			quat localModeQ = glm::normalize(glm::slerp(qa, qb, progress));

			if (!needWorldChain) {
				finalLocalQ = localModeQ;
			}
			else {
				// gqa is just parentResultQ * qa - the identity the original's
				// full-skeleton globalQA chain reduces to at this one index.
				quat gqa = glm::normalize(parentResultQ * qa);
				quat gqb = glm::normalize(gqbChain * qb);
				if (glm::dot(gqa, gqb) < 0.0f) gqb = -gqb;
				quat blendedGlobalQ = glm::normalize(glm::slerp(gqa, gqb, progress));

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
			// Child bones: simple local slerp only, exactly as before.
			quat qa = glm::normalize(q_A);
			quat qb = glm::normalize(q_B);
			if (glm::dot(qa, qb) < 0.0f) qb = -qb;
			finalLocalQ = glm::normalize(glm::slerp(qa, qb, progress));
		}

		out.boneTransforms[name] = ComposeLocal(blendedT, blendedS, finalLocalQ);
	}

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