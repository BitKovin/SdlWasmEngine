#include "SkeletalMesh.hpp"
#include <shared_mutex>
#include <algorithm>
#include "Level.hpp"
#include "EngineMain.h"



// Zero-copy read access. Rebuilds lastPose only when dirty (one unavoidable
// deep copy of the animator's bone map into lastPose, since lastPose has to
// be an independent, frozen snapshot the animator can't invalidate next
// frame). Callers that only need to read the pose - not own an independent
// copy - should call this instead of GetAnimationPose() to skip the extra
// copy of returning an AnimationPose by value.
const AnimationPose& SkeletalMesh::GetAnimationPoseRef()
{
	static const AnimationPose s_emptyPose;
	if (model == nullptr) return s_emptyPose;

	if (dirtyPose)
	{
		std::lock_guard<std::recursive_mutex> lock(animationsMutex);
		lastPose.boneTreeRoot = &model->defaultRoot;
		lastPose.boneTransforms = animator.GetBonePoseArray();
		dirtyPose = false;
	}

	return lastPose;
}

AnimationPose SkeletalMesh::GetAnimationPose()
{
	if (model == nullptr) return AnimationPose();

	// One copy here, paid only by callers that actually need to own a
	// snapshot (e.g. to survive across later mutating calls).
	return GetAnimationPoseRef();
}

void SkeletalMesh::ApplyWorldSpaceBoneTransforms(std::unordered_map<hashed_string, mat4>& pose)
{
	std::lock_guard<std::recursive_mutex> lock(animationsMutex);
	mat4 invWorld = inverse(GetWorldMatrix()); // hoisted - was being inverted once per bone

	for (auto& bonePose : pose)
	{
		bonePose.second = invWorld * bonePose.second; // was pose[bonePose.first], a redundant second hash lookup per bone
	}

	animator.ApplyLocalSpacePoseArray(GetAnimationPoseRef().boneTransforms, pose);
	boneTransforms = animator.getBoneMatrices();

	dirtyPose = true;

}

void SkeletalMesh::PlayAnimation(std::string name, bool Loop, float interpIn)
{
	if (model == nullptr) return;

	std::lock_guard<std::recursive_mutex> lock(animationsMutex);

	SetLooped(Loop);
	animator.set(name);
	PlayAnimation(interpIn);
	animator.totalRootMotionPosition = vec3();
	animator.totalRootMotionRotation = vec3();
	animator.oldRootBoneTransform = MathHelper::Transform();
	rootMotionBasisQuat = quat();
	currentAnimationData = GetAnimationDataFromName(name);
	oldAnimationEventTime = -1;
	Update(0.0001f);

	positionOffset = vec3();
	rotationOffset = vec3();

	// Sync AFTER Update — absorbs the first-frame root bone starting offset
	// into oldRootMotionPos so PullRootMotion() doesn't emit it as phantom delta
	oldRootMotionPos = animator.totalRootMotionPosition;
	oldRootMotionRot = animator.totalRootMotionRotation;

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
	box.Min -= vec3(1);

	if (Level::Current->BspData.m_numOfVerts)
	{
		int cameraC = Level::Current->BspData.FindClusterAtPosition(Camera::finalizedPosition);

		vec3 min = box.Min;
		vec3 max = box.Max;

		vec3 points[8] =
		{
			{min.x, min.y, min.z},
			{max.x, min.y, min.z},
			{min.x, max.y, min.z},
			{max.x, max.y, min.z},

			{min.x, min.y, max.z},
			{max.x, min.y, max.z},
			{min.x, max.y, max.z},
			{max.x, max.y, max.z}
		};

		bool visible = false;

		for (int i = 0; i < 8; i++)
		{
			int targetC = Level::Current->BspData.FindClusterAtPosition(points[i]);

			if (Level::Current->BspData.IsClusterVisible(cameraC, targetC))
			{
				visible = true;
				break;
			}
		}

		if (!visible)
			return false;
	}

	return IsInFrustrum(Camera::frustum) && isVisible();
}

BoundingBox SkeletalMesh::GetBoundingBox()
{

	if (model == nullptr) return BoundingBox();

	mat4 world = GetWorldMatrix();


	if (InRagdoll == false)
	{
		return model->ComputeAnimatedBounds(world, boneTransforms);
	}


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

	if (model == nullptr) return LightVolPointData{ vec3(0),vec3(1),vec3(0) };

	vec3 samplePos = box.Center() + vec3(0, 0.5, 0);


	auto light = Level::Current->BspData.GetLightvolColorPoint(samplePos * MAP_SCALE, true);
	return light;
}

// In your SkeletalMesh:
MathHelper::Transform SkeletalMesh::PullRootMotion()
{

	std::lock_guard<std::recursive_mutex> lock(animationsMutex);

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

	std::lock_guard<std::recursive_mutex> lock(animationsMutex);

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
		// NOTE: assumes AnimationPose::Lerp takes its pose arguments by
		// const&, matching LayeredLerp's signature elsewhere in this file.
		// If Lerp takes AnimationPose by value, this reference gets copied
		// there instead - still fine, just double check the signature.
		const AnimationPose& currentPose = GetAnimationPoseRef();

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

			if (JPH::isfinite(ToPhysics(linearVel).LengthSq()) && JPH::isfinite(ToPhysics(angularVel).LengthSq()))
			{
				Physics::SetLinearVelocity(hitbox, linearVel / 1.5f);
				Physics::SetAngularVelocity(hitbox, angularVel / 1.5f);
			}

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

		std::lock_guard<std::recursive_mutex> lock(animationsMutex);

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
