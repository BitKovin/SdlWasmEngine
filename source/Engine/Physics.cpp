#include "Physics.h"

#include "Entity.h"

#include <unordered_set>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>
#include <Jolt/Physics/Collision/CollidePointResult.h>
#include <Logger.hpp>


TempAllocatorImpl* Physics::tempMemAllocator = nullptr;

JobSystemThreadPool* Physics::threadPool = nullptr;

BPLayerInterfaceImpl* Physics::broad_phase_layer_interface = nullptr;

ObjectVsBroadPhaseLayerFilterImpl* Physics::object_vs_broadphase_layer_filter = nullptr;

ObjectLayerPairFilterImpl* Physics::object_vs_object_layer_filter = nullptr;

PhysicsSystem* Physics::physics_system = nullptr;

MyContactListener* Physics::contact_listener = nullptr;

BodyInterface* Physics::bodyInterface = nullptr;

unordered_map<BodyID, Body*> Physics::bodyIdMap;

vector<Body*> Physics::existingBodies = vector<Body*>();

std::unordered_map<std::string, uint32_t> Physics::SurfaceIds = std::unordered_map<std::string, uint32_t>();
std::unordered_map<uint32_t, std::string> Physics::SurfaceNames = std::unordered_map<uint32_t, std::string>();
uint32_t Physics::nextSurfaceId = 1;

recursive_mutex Physics::physicsMainLock = recursive_mutex();

bool Physics::DebugDraw = false;
#ifdef JPH_DEBUG_RENDERER
MyDebugRenderer* Physics::debugRenderer = nullptr;
#endif
BodyType Physics::DebugDrawMask = BodyType::GroupAll & ~BodyType::CharacterCapsule;

std::vector<Physics::PendingBodyEnterPair> Physics::gRemovals;
std::vector<Physics::PendingBodyEnterPair> Physics::gAdds;

void MyContactListener::addIgnorePair(const BodyID& body1, const BodyID& body2)
{
	uint32_t n1 = body1.GetIndexAndSequenceNumber();
	uint32_t n2 = body2.GetIndexAndSequenceNumber();
	if (n1 > n2) std::swap(n1, n2);
	ignoredPairs.insert({ n1, n2 });
}

void MyContactListener::removeIgnorePair(const BodyID& body1, const BodyID& body2)
{
	uint32_t n1 = body1.GetIndexAndSequenceNumber();
	uint32_t n2 = body2.GetIndexAndSequenceNumber();
	if (n1 > n2) std::swap(n1, n2);
	ignoredPairs.erase({ n1, n2 });
}

void MyContactListener::CleanIgnorePairs()
{
	ignoredPairs.clear();
}

ValidateResult MyContactListener::OnContactValidate(const Body& inBody1, const Body& inBody2, RVec3Arg inBaseOffset, const CollideShapeResult& inCollisionResult)
{
	uint32_t n1 = inBody1.GetID().GetIndexAndSequenceNumber();
	uint32_t n2 = inBody2.GetID().GetIndexAndSequenceNumber();
	if (n1 > n2) std::swap(n1, n2);
	if (ignoredPairs.contains({ n1, n2 }))
		return ValidateResult::RejectContact;

	auto* props1 = reinterpret_cast<BodyData*>(inBody1.GetUserData());
	auto* props2 = reinterpret_cast<BodyData*>(inBody2.GetUserData());
	if (props1 && props2)
	{
		if (props1->mask == BodyType::None)
			return ValidateResult::RejectContact;
		if (props2->mask == BodyType::None)
			return ValidateResult::RejectContact;

		bool collide1 = (static_cast<uint32_t>(props1->group) & static_cast<uint32_t>(props2->mask)) != 0;
		bool collide2 = (static_cast<uint32_t>(props2->group) & static_cast<uint32_t>(props1->mask)) != 0;
		if (!collide1 && !collide2)
		{
			if (props1->dynamicCollisionGroupOrMask || props2->dynamicCollisionGroupOrMask)
				return ValidateResult::RejectContact;
			else
				return ValidateResult::RejectAllContactsForThisBodyPair;
		}

		// Check SensorCollisionMode for each body against whether the other body is a sensor.
		// body1's mode is validated against body2 being a sensor, and vice versa.
		bool rejectBySensorMode = false;

		const bool body1IsSensor = inBody1.IsSensor();
		const bool body2IsSensor = inBody2.IsSensor();

		// body1's rules: what kind of bodies does body1 want to collide with?
		switch (props1->sensorCollisionMode)
		{
		case SensorCollisionMode::NotCollideWithSensors:
			if (body2IsSensor) rejectBySensorMode = true;
			break;
		case SensorCollisionMode::CollideOnlyWithSensors:
			if (!body2IsSensor) rejectBySensorMode = true;
			break;
		case SensorCollisionMode::CollideWithEverything:
		default:
			break;
		}

		// body2's rules: what kind of bodies does body2 want to collide with?
		if (!rejectBySensorMode)
		{
			switch (props2->sensorCollisionMode)
			{
			case SensorCollisionMode::NotCollideWithSensors:
				if (body1IsSensor) rejectBySensorMode = true;
				break;
			case SensorCollisionMode::CollideOnlyWithSensors:
				if (!body1IsSensor) rejectBySensorMode = true;
				break;
			case SensorCollisionMode::CollideWithEverything:
			default:
				break;
			}
		}

		if (rejectBySensorMode)
		{
			if (props1->dynamicCollisionGroupOrMask || props2->dynamicCollisionGroupOrMask)
				return ValidateResult::RejectContact;
			else
				return ValidateResult::RejectAllContactsForThisBodyPair;
		}

		if (props1->dynamicCollisionGroupOrMask || props2->dynamicCollisionGroupOrMask)
			return ValidateResult::AcceptContact;
	}
	return ValidateResult::AcceptAllContactsForThisBodyPair;
}

void MyContactListener::beforeSimulation()
{
	previousContacts = std::move(currentContacts); // Move for efficiency
	currentContacts.clear(); // Reset for the new step
}

void MyContactListener::afterSimulation()
{
	std::vector<Physics::PendingBodyEnterPair> adds;
	std::vector<Physics::PendingBodyEnterPair> removals;



	// Find added contacts: in current but not in previous
	for (const auto& pair : currentContacts)
	{
		if (previousContacts.find(pair) == previousContacts.end())
		{
			Physics::PendingBodyEnterPair p;
			p.entity = pair.first;  // Sensor entity
			p.target = pair.second; // Target entity
			adds.push_back(p);
		}
	}

	// Find removed contacts: in previous but not in current
	for (const auto& pair : previousContacts)
	{
		if (currentContacts.find(pair) == currentContacts.end())
		{
			Physics::PendingBodyEnterPair p;
			p.entity = pair.first;  // Sensor entity
			p.target = pair.second; // Target entity
			removals.push_back(p);
		}
	}

	auto newAdds = Physics::gAdds;
	auto newRemovs = Physics::gRemovals;

	for (const auto& p : adds)
	{
		newAdds.push_back(p);
	}
	for (const auto& p : removals)
	{
		newRemovs.push_back(p);
	}

	Physics::gAdds = newAdds;
	Physics::gRemovals = newRemovs;
	

}

void MyContactListener::OnContactAdded(const Body& inBody1, const Body& inBody2, const ContactManifold& inManifold, ContactSettings& ioSettings)
{
	auto data1 = Physics::GetBodyData(&inBody1);
	auto data2 = Physics::GetBodyData(&inBody2);
	Entity* entity1 = data1 ? data1->OwnerEntity : nullptr;
	Entity* entity2 = data2 ? data2->OwnerEntity : nullptr;

	if (entity1 == nullptr || entity2 == nullptr) return;

	Physics::physicsMainLock.lock();

	// Add pair if body1 is a sensor
	if (inBody1.IsSensor())
	{
		currentContacts.insert({ entity1, entity2 });
	}
	// Add pair if body2 is a sensor
	if (inBody2.IsSensor())
	{
		currentContacts.insert({ entity2, entity1 });
	}

	Physics::physicsMainLock.unlock();
}

void MyContactListener::OnContactPersisted(const Body& inBody1, const Body& inBody2, const ContactManifold& inManifold, ContactSettings& ioSettings)
{
	auto data1 = Physics::GetBodyData(&inBody1);
	auto data2 = Physics::GetBodyData(&inBody2);
	Entity* entity1 = data1 ? data1->OwnerEntity : nullptr;
	Entity* entity2 = data2 ? data2->OwnerEntity : nullptr;

	if (entity1 == nullptr || entity2 == nullptr) return;

	Physics::physicsMainLock.lock();

	// Add pair if body1 is a sensor
	if (inBody1.IsSensor())
	{
		currentContacts.insert({ entity1, entity2 });
	}
	// Add pair if body2 is a sensor
	if (inBody2.IsSensor())
	{
		currentContacts.insert({ entity2, entity1 });
	}

	Physics::physicsMainLock.unlock();
}

void MyContactListener::OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair)
{
	// Leave empty; removals are handled in afterSimulation()
}

void Physics::SetGravity(vec3 gravity)
{
	physics_system->SetGravity(ToPhysics(gravity));
}

void Physics::DestroyBody(Body* body)
{
	if (body == nullptr)
		return;

	physicsMainLock.lock();

	bodyIdMap.erase(body->GetID());


	// Retrieve and delete collision properties if present.
	auto* props = reinterpret_cast<BodyData*>(body->GetUserData());

	if (props)
	{
		delete props;
	}

	bodyInterface->RemoveBody(body->GetID());

	bodyInterface->DestroyBody(body->GetID());


	// Remove body from existingBodies
	auto it = std::find(existingBodies.begin(), existingBodies.end(), body);
	if (it != existingBodies.end())
		existingBodies.erase(it);

	physicsMainLock.unlock();
}

void Physics::DestroyConstraint(Constraint* constraint)
{

	if (constraint == nullptr) return;

	physicsMainLock.lock();

	physics_system->RemoveConstraint(constraint);

	physicsMainLock.unlock();


}

void Physics::SetMotionType(Body* body, JPH::EMotionType type)
{
	bodyInterface->SetMotionType(body->GetID(), type, JPH::EActivation::Activate);
}

void Physics::AddIgnorePair(const BodyID& bodyA, const BodyID& bodyB)
{
	contact_listener->addIgnorePair(bodyA, bodyB);
}

void Physics::RemoveIgnorePair(const BodyID& bodyA, const BodyID& bodyB)
{
	contact_listener->removeIgnorePair(bodyA, bodyB);
}

bool initilizedPhysics = false;

void Physics::Init()
{
	gAdds.clear();
	gRemovals.clear();

	if(initilizedPhysics == false)
	{
		initilizedPhysics = true;
	}
	else
	{
		return;
	}

	RegisterDefaultAllocator();
	Factory::sInstance = new Factory();

	RegisterTypes();

	tempMemAllocator = new TempAllocatorImpl(30 * 1024 * 1024);

	int numThreads = ThreadPool::GetNumThreadsForPhysics();


	threadPool = new JobSystemThreadPool(cMaxPhysicsJobs, cMaxPhysicsBarriers, numThreads);

	const uint cMaxBodies = 65536;

	const uint cNumBodyMutexes = 0;

	const uint cMaxBodyPairs = 65536;

	const uint cMaxContactConstraints = 20240;

	object_vs_broadphase_layer_filter = new ObjectVsBroadPhaseLayerFilterImpl();

	object_vs_object_layer_filter = new ObjectLayerPairFilterImpl();

	broad_phase_layer_interface = new BPLayerInterfaceImpl();

	physics_system = new PhysicsSystem();

	JPH::PhysicsSettings settings = physics_system->GetPhysicsSettings(); 

	settings.mNumVelocitySteps = 5u;
	settings.mNumPositionSteps = 2u;

	if (ThreadPool::Supported() == false)
	{
		settings.mNumVelocitySteps = 3u;
		settings.mNumPositionSteps = 1u;
		settings.mPenetrationSlop = 0.05f;
		settings.mDeterministicSimulation = false;

	}
	physics_system->SetPhysicsSettings(settings);

	physics_system->Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints, *broad_phase_layer_interface, *object_vs_broadphase_layer_filter, *object_vs_object_layer_filter);


	contact_listener = new MyContactListener();
	physics_system->SetContactListener(contact_listener);

	bodyInterface = &physics_system->GetBodyInterface();

#ifdef JPH_DEBUG_RENDERER
	debugRenderer = new MyDebugRenderer();
#endif

}

void Physics::UpdatePendingBodyExitsEnters()
{

	std::unordered_set<PendingBodyEnterPair> processedAdds;
	std::unordered_set<PendingBodyEnterPair> processedRemovals;

	for (auto& pair : gRemovals)
	{

		auto result = processedRemovals.find(pair);
		if (result != processedRemovals.end()) 
		{
			Logger::Log("found duplicate exit pair");
			continue;
		}

		if (pair.entity == nullptr || pair.target == nullptr)
			continue;

		//in case if one of them was deleted in previous frame
		if (Level::Current->DeletedLevelObjectAdresses.find(pair.entity) != Level::Current->DeletedLevelObjectAdresses.end())
		{
			continue;
		}
		if (Level::Current->DeletedLevelObjectAdresses.find(pair.target) != Level::Current->DeletedLevelObjectAdresses.end())
		{
			continue;
		}

		//pair.target->OnBodyExited(pair.entity->LeadBody, pair.entity);	
		pair.entity->OnBodyExited(pair.target->LeadBody, pair.target);

		processedRemovals.insert(pair);
	}
	gRemovals.clear();

	for (auto& pair : gAdds)
	{

		auto result = processedAdds.find(pair);
		{
			if (result != processedAdds.end()) 
			{
				Logger::Log("found duplicate enter pair");
				continue;
			}
		}

		if (pair.entity == nullptr || pair.target == nullptr)
			continue;

		//in case if one of them was deleted in previous frame
		if (Level::Current->DeletedLevelObjectAdresses.find(pair.entity) != Level::Current->DeletedLevelObjectAdresses.end())
		{
			continue;
		}
		if (Level::Current->DeletedLevelObjectAdresses.find(pair.target) != Level::Current->DeletedLevelObjectAdresses.end())
		{
			continue;
		}


		pair.entity->OnBodyEntered(pair.target->LeadBody, pair.target);
		//pair.target->OnBodyEntered(pair.entity->LeadBody, pair.entity);


		processedAdds.insert(pair);
	}
	gAdds.clear();

}

void Physics::DrawConstraint(Constraint* constraint)
{

	if (constraint == nullptr) return;
#ifdef JPH_DEBUG_RENDERER
	constraint->DrawConstraint(debugRenderer);
	constraint->DrawConstraintLimits(debugRenderer);
	constraint->DrawConstraintReferenceFrame(debugRenderer);
#endif

}




Body* Physics::CreateHitBoxBody(Entity* owner, SkeletalMesh* mesh, string hitboxName, vec3 PositionOffset, quat RotationOffset, vec3 Size, float Mass, BodyType group, BodyType mask)
{
	
	// 1) Base box, centered at its own origin
	auto box_settings = JPH::BoxShapeSettings();
	box_settings.SetEmbedded();
	box_settings.mHalfExtent = ToPhysics(Size) * 0.5f;

	// 2) Rotate and translate the box in shape-local space
	auto geo_settings = JPH::RotatedTranslatedShapeSettings(
		ToPhysics(PositionOffset),   // translate
		ToPhysics(RotationOffset),   // rotate
		&box_settings                // child shape
	);
	geo_settings.SetEmbedded();

	// 3) Create the final shape directly from geo_settings
	JPH::Shape::ShapeResult sr = geo_settings.Create();
	if (sr.HasError())
		Logger::Log(sr.GetError().c_str());
	JPH::Ref<JPH::Shape> final_shape = sr.Get();

	// 4) Use the entity’s (bone’s) world-space position as the body’s position
	JPH::RVec3 world_com = Vec3(0, 0, 0);

	// 5) Build the body
	JPH::BodyCreationSettings bcs(
		final_shape,
		world_com,
		JPH::Quat::sIdentity(),      // shape rotation is baked in
		JPH::EMotionType::Kinematic,
		Layers::MOVING
	);

	bcs.mMotionQuality = EMotionQuality::LinearCast;

	bcs.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
	bcs.mMassPropertiesOverride.mMass = Mass;
	bcs.mFriction = 0.9f;

	bcs.mAngularDamping = 10;

	BodyData* props = new BodyData{ group, mask, true, owner, hitboxName, mesh };
	bcs.mUserData = reinterpret_cast<uintptr_t>(props);

	// 6) Create and register the body
	JPH::Body* body = bodyInterface->CreateBody(bcs);
	AddBody(body);

	return body;

}

TwoBodyConstraint* Physics::CreateRagdollConstraint(Body* parent,
	Body* child,
	float    twistMinAngle,
	float    twistMaxAngle,
	float    swingHalfConeAngle,
	JPH::QuatArg childSpaceConstraintRotation)
{
	
	twistMinAngle /= 180.0/pi<double>();
	twistMaxAngle /= 180.0 / pi<double>();
	swingHalfConeAngle /= 180.0 / pi<double>();

	SwingTwistConstraintSettings settings;
	settings.mSpace = EConstraintSpace::LocalToBodyCOM;

	// --- 1) grab the child's entity/world position (not its COM)
	//     (assuming GetWorldTransform() returns a JPH::RMat44)
	RMat44  childEntityX = child->GetWorldTransform();
	Vec3    worldPivot = childEntityX.GetTranslation();

	// --- 2) compute child‐space pivot (in child‐COM local coords)
	RMat44  childCOMX = child->GetCenterOfMassTransform();
	RMat44  invChildCOM = childCOMX.Inversed();
	Vec4    local2_v4 = invChildCOM * Vec4(worldPivot, 1.0f);
	settings.mPosition2 = Vec3(
		local2_v4.GetX(), local2_v4.GetY(), local2_v4.GetZ()
	);

	// --- 3) compute parent‐space pivot (in parent‐COM local coords)
	RMat44  parentCOMX = parent->GetCenterOfMassTransform();
	RMat44  invParentCOM = parentCOMX.Inversed();
	Vec4    local1_v4 = invParentCOM * Vec4(worldPivot, 1.0f);
	settings.mPosition1 = Vec3(
		local1_v4.GetX(), local1_v4.GetY(), local1_v4.GetZ()
	);

	// --- 4) build twist & plane axes from childSpaceConstraintRotation
	Vec3 csTwistAxis = childSpaceConstraintRotation * Vec3(0, 1, 0);
	Vec3 csPlaneAxis = childSpaceConstraintRotation * Vec3(1, 0, 0);

	// world axes:
	Vec3 worldTwist = (child->GetRotation() * csTwistAxis).Normalized();
	Vec3 worldPlane = (child->GetRotation() * csPlaneAxis).Normalized();

	// back into parent‐local:
	Vec3 parentTwist = parent->GetRotation().Conjugated() * worldTwist;
	Vec3 parentPlane = parent->GetRotation().Conjugated() * worldPlane;

	settings.mTwistAxis1 = parentTwist;
	settings.mTwistAxis2 = csTwistAxis;
	settings.mPlaneAxis1 = parentPlane;
	settings.mPlaneAxis2 = csPlaneAxis;

	// --- 5) set rotational limits (translation is auto‐locked)
	settings.mTwistMinAngle = twistMinAngle;
	settings.mTwistMaxAngle = twistMaxAngle;
	settings.mNormalHalfConeAngle = swingHalfConeAngle;
	settings.mPlaneHalfConeAngle = swingHalfConeAngle;

	// --- 6) create & register
	Ref<TwoBodyConstraint> c = settings.Create(*parent, *child);
	physics_system->AddConstraint(c);

	AddIgnorePair(parent->GetID(), child->GetID());

	return c;

}

void Physics::ConfigureSwingTwistMotor(TwoBodyConstraint* constraint, float frequencyHz, float damping, float max_torque)
{

	if (constraint == nullptr) return;
	if (constraint->GetSubType() != EConstraintSubType::SwingTwist)
		return; // not the right constraint

	SwingTwistConstraint* st = static_cast<SwingTwistConstraint*>(constraint);

	// Swing motor
	MotorSettings& swing = st->GetSwingMotorSettings();
	swing.mSpringSettings.mMode = ESpringMode::FrequencyAndDamping;
	swing.mSpringSettings.mFrequency = frequencyHz;
	swing.mSpringSettings.mDamping = damping;
	swing.SetTorqueLimits(-max_torque, max_torque);

	// Twist motor
	MotorSettings& twist = st->GetTwistMotorSettings();
	twist.mSpringSettings.mMode = ESpringMode::FrequencyAndDamping;
	twist.mSpringSettings.mFrequency = frequencyHz;
	twist.mSpringSettings.mDamping = damping;
	twist.SetTorqueLimits(-max_torque, max_torque);


	st->SetSwingMotorState(EMotorState::Off);
	st->SetTwistMotorState(EMotorState::Off);

	// If you just changed motor settings after the constraint already ran in the sim,
	// reset warm start so previous impulses aren't applied to the new config.
	st->ResetWarmStart();

}

void Physics::UpdateSwingTwistMotor(TwoBodyConstraint* constraint,
	const quat& childTransformRelParent,
	const float& strength)
{
	if (constraint == nullptr) return;
	if (constraint->GetSubType() != EConstraintSubType::SwingTwist) return;

	SwingTwistConstraint* st = static_cast<SwingTwistConstraint*>(constraint);

	// Convert and normalize
	Quat targetRel = ToPhysics(childTransformRelParent);

	// 1) Ensure no NaNs / infinities
	if (!std::isfinite(targetRel.GetX()) || !std::isfinite(targetRel.GetY()) ||
		!std::isfinite(targetRel.GetZ()) || !std::isfinite(targetRel.GetW()))
	{
		return;
	}

	// 2) Normalize (avoid drift)
	if (targetRel.LengthSq() > 1e-6f)
		targetRel = targetRel.Normalized();
	else
		return;

	st->SetTargetOrientationBS(targetRel);

	// Scale torque limits by strength
	float maxTorque = 60 * strength; // or a tuned value instead of FLT_MAX
	st->GetSwingMotorSettings().SetTorqueLimits(-maxTorque, maxTorque);
	st->GetTwistMotorSettings().SetTorqueLimits(-maxTorque, maxTorque);

	st->GetTwistMotorSettings().mSpringSettings.mStiffness = 1000 * strength;

	// Optional: completely disable motors if strength == 0
	if (strength <= 0.0f) {
		st->SetSwingMotorState(EMotorState::Off);
		st->SetTwistMotorState(EMotorState::Off);
	}
	else {
		st->SetSwingMotorState(EMotorState::Position);
		st->SetTwistMotorState(EMotorState::Position);
	}

}

uint32_t Physics::FindSurfaceId(string surfaceName)
{
	Physics::physicsMainLock.lock();

	auto result = SurfaceIds.find(surfaceName);

	if (result != SurfaceIds.end())
	{
		Physics::physicsMainLock.unlock();
		return result->second;
	}

	int newId = nextSurfaceId;
	nextSurfaceId++;
	SurfaceIds[surfaceName] = newId;
	SurfaceNames[newId] = surfaceName;
	Physics::physicsMainLock.unlock();

	return newId;
}

string Physics::FindSurfacyById(uint32_t id)
{
	std::lock_guard<std::recursive_mutex> guard(Physics::physicsMainLock);

	auto it = SurfaceNames.find(id);
	if (it != SurfaceNames.end())
		return it->second;

	// Not found — choose what makes sense: empty, error, or throw
	return std::string();  // empty = “unknown”
}

void Physics::AddImpulse(const Body* body, vec3 impulse)
{

	if (body == nullptr) return;

	bodyInterface->AddImpulse(body->GetID(), ToPhysics(impulse));

}

void Physics::AddImpulseAtLocation(const Body* body, vec3 impulse, vec3 point)
{

	if (body == nullptr) return;

	bodyInterface->AddImpulse(body->GetID(), ToPhysics(impulse), ToPhysics(point));

}

RefConst<Shape> Physics::CreateMeshShape(const std::vector<vec3>& vertices, const std::vector<uint32_t>& indices, const std::vector<std::string>& materials)
{
	if (indices.size() % 3 != 0)
	{
		Logger::Error("Number of indices (%zu) must be a multiple of 3 to form complete triangles.", indices.size());
		return RefConst<Shape>();
	}

	size_t numTriangles = indices.size() / 3;
	if (!materials.empty() && materials.size() != numTriangles)
	{
		Logger::Error("Material count (%zu) does not match triangle count (%zu).", materials.size(), numTriangles);
		return RefConst<Shape>();
	}

	// 1. Convert vertices
	Array<Float3> joltVertices;
	joltVertices.reserve(vertices.size());
	for (const auto& v : vertices)
	{
		joltVertices.push_back(Float3(v.x, v.y, v.z));
	}

	// 2. Prepare material tracking
	// Jolt requires an array of RefConst<PhysicsMaterial>
	JPH::PhysicsMaterialList joltMaterials;
	// Map to prevent creating duplicate materials for the same surface ID
	std::unordered_map<int, uint32_t> surfaceIdToJoltMatIndex;

	// 3. Convert indices and assign materials
	Array<IndexedTriangle> joltTriangles;
	joltTriangles.reserve(numTriangles);

	for (size_t i = 0; i < indices.size(); i += 3)
	{
		IndexedTriangle tri;
		tri.mIdx[0] = indices[i + 0];
		tri.mIdx[1] = indices[i + 1];
		tri.mIdx[2] = indices[i + 2];

		int surfaceId = materials.empty() ? 0 : FindSurfaceId(materials[i / 3]);

		// Find or create the material in our unique list
		uint32_t matIndex = 0;
		auto it = surfaceIdToJoltMatIndex.find(surfaceId);
		if (it == surfaceIdToJoltMatIndex.end())
		{
			// Material doesn't exist yet, create it
			matIndex = static_cast<uint32_t>(joltMaterials.size());
			surfaceIdToJoltMatIndex[surfaceId] = matIndex;

			// Allocate our custom material and add it to Jolt's list
			joltMaterials.push_back(new MySurfaceMaterial(surfaceId));
		}
		else
		{
			// Material already exists, use its index
			matIndex = it->second;
		}

		// Assign the index pointing to the joltMaterials array
		tri.mMaterialIndex = matIndex;
		joltTriangles.push_back(tri);
	}

	// 4. Construct the settings object with ALL arrays populated
	MeshShapeSettings shapeSettings;
	shapeSettings.mTriangleVertices = std::move(joltVertices);
	shapeSettings.mIndexedTriangles = std::move(joltTriangles);
	shapeSettings.mMaxTrianglesPerLeaf = 4;
	shapeSettings.mBuildQuality = JPH::MeshShapeSettings::EBuildQuality::FavorRuntimePerformance;

	// CRITICAL: We must pass the material list to Jolt, or it will delete our indices!
	shapeSettings.mMaterials = std::move(joltMaterials);

	Shape::ShapeResult result = shapeSettings.Create();
	if (result.HasError())
	{
		Logger::Error("Error creating mesh shape: %s", result.GetError().c_str());
		return RefConst<Shape>();
	}

	return result.Get();
}

RefConst<Shape> Physics::CreateConvexHullFromPoints(const std::vector<glm::vec3>& points, std::string surfaceName)
{
	// Convert std::vector<Vec3> to Array<Vec3> (Jolt's format)
	Array<Vec3> hullPoints;
	for (auto pt : points)
	{
		hullPoints.push_back(Vec3(pt.x, pt.y, pt.z));
	}

	// Settings for the convex hull shape
	ConvexHullShapeSettings shapeSettings(hullPoints);
	shapeSettings.mMaterial = new MySurfaceMaterial(FindSurfaceId(surfaceName)); // Assign the specified material

	// Optional: check for errors
	Shape::ShapeResult result = shapeSettings.Create();
	if (result.HasError())
	{
		printf("Error creating convex hull shape: %s\n", result.GetError().c_str());
		return nullptr;
	}

	// Successfully created shape
	RefConst<Shape> shape = result.Get();

	return shape;
}

vector<Physics::HitResult> Physics::PointTrace(
	const vec3 point,
	const BodyType mask,
	const vector<Body*> ignoreList,
	const vector<Entity*> entityIgnoreList)
{
	vector<HitResult> hits;

	// Convert point to physics space
	JPH::RVec3 physPoint = ToPhysics(point);

	// Collector for ALL hits
	using Collector = JPH::AllHitCollisionCollector<JPH::CollidePointCollector>;

	Collector collector;

	// Prepare filters
	TraceBodyFilter filter;
	filter.mask = mask;
	filter.ignoreList = ignoreList;
	filter.entityIgnoreList = entityIgnoreList;

	// Run the query
	physics_system->GetNarrowPhaseQuery().CollidePoint(
		physPoint,
		collector,
		{},        // BroadPhaseLayerFilter
		{},        // ObjectLayerFilter
		filter     // BodyFilter
	);

	// If nothing hit
	if (!collector.HadHit())
		return hits;

	// For each hit
	for (const JPH::CollidePointResult& r : collector.mHits)
	{
		HitResult hr;
		hr.hasHit = false; // will set below
		hr.fraction = 0.0f;
		hr.position = point;          // per your instruction
		hr.shapePosition = point;     // contact = input point
		hr.normal = vec3(0, 0, 0);    // no normal for point test
		hr.hitbody = nullptr;
		hr.entity = nullptr;
		hr.hitboxName = "";
		hr.surfaceName = "";

		// Lock body
		JPH::BodyLockRead lock(physics_system->GetBodyLockInterface(), r.mBodyID);
		if (!lock.Succeeded())
			continue;

		const JPH::Body& body = lock.GetBody();

		// Retrieve subshape user data (same as in LineTrace)
		const JPH::Shape* shape = body.GetShape();
		JPH::SubShapeID remainder;
		const JPH::Shape* leaf = shape->GetLeafShape(r.mSubShapeID2, remainder);

		if (leaf)
		{
			auto material = leaf->GetMaterial(r.mSubShapeID2);
			if (material != JPH::PhysicsMaterial::sDefault)
			{
				hr.surfaceName = ((MySurfaceMaterial*)material)->GetName();
			}
		}

		// Set body + entity info
		hr.hitbody = &body;

		auto* props = reinterpret_cast<BodyData*>(body.GetUserData());
		if (props)
		{
			hr.entity = props->OwnerEntity;
			hr.hitboxName = props->hitboxName;
		}

		// Final validity
		hr.hasHit = (hr.entity != nullptr && !hr.entity->Destroyed);

		if (hr.hasHit)
			hits.push_back(hr);
	}

	return hits;
}

Physics::HitResult Physics::LineTrace(const vec3 start, const vec3 end, const BodyType mask, const vector<Body*> ignoreList, const vector<Entity*> entityIgnoreList)
{
	HitResult hit;
	hit.fraction = 1.0f;
	hit.position = end;
	hit.normal = vec3(0, 0, 0);
	hit.hitbody = nullptr;
	hit.hasHit = false;
	hit.shapePosition = end;
	hit.entity = nullptr;
	hit.hitboxName = "";
	hit.surfaceName = "";

	// Convert start and end from your own vector type to Jolt's coordinate system.
	JPH::Vec3 startLoc = ToPhysics(start);
	JPH::Vec3 endLoc = ToPhysics(end);

	// Set up the ray from startLoc to endLoc.
	JPH::RRayCast ray;
	ray.mOrigin = startLoc;
	JPH::Vec3 ray_dir = endLoc - startLoc;
	ray.mDirection = ray_dir;

	// Prepare a result structure for the ray cast.
	JPH::RayCastResult result;

	TraceBodyFilter filter;
	filter.mask = mask;
	filter.ignoreList = ignoreList;
	filter.entityIgnoreList = entityIgnoreList;

	//physicsMainLock.lock();

	// Cast the ray using the narrow phase query.
	bool hasHit = physics_system->GetNarrowPhaseQuery().CastRay(ray, result, {}, {}, filter);

	if (hasHit)
	{


		// Lock the body using the BodyLockInterface to safely access it.
		JPH::BodyLockRead body_lock(physics_system->GetBodyLockInterface(), result.mBodyID);

		// Retrieve the hit body's surface normal.
		// The method GetWorldSpaceSurfaceNormal requires the sub-shape ID and the hit position.
		const JPH::Body* body = &body_lock.GetBody();

		if (body)
		{
			const JPH::Shape* root_shape = body->GetShape();
			auto material = root_shape->GetMaterial(result.mSubShapeID2);
			
			if (material != JPH::PhysicsMaterial::sDefault)
			{
				hit.surfaceName = ((MySurfaceMaterial*)material)->GetName();
			}

			// Calculate fraction of the hit along the ray.
			hit.fraction = result.mFraction;

			hit.shapePosition = mix(start, end, hit.fraction);

			// Compute the hit position in physics space and convert it back to your coordinate system.
			hit.position = FromPhysics(ray.GetPointOnRay(result.mFraction));


			hit.normal = FromPhysics(body->GetWorldSpaceSurfaceNormal(result.mSubShapeID2, ray.GetPointOnRay(result.mFraction)));


			// Record the hit body and shape.
			hit.hitbody = body;

			auto* props = reinterpret_cast<BodyData*>(body->GetUserData());

			if (props == nullptr)
			{
				hasHit = false;
			}
			else
			{
				hit.entity = props->OwnerEntity;
				hit.hitboxName = props->hitboxName;
			}


		}
	}




	hit.hasHit = hasHit && hit.entity != nullptr;

	if (hit.hasHit)
	{
		if (hit.entity->Destroyed)
			hit.hasHit = false;

	}

	return hit;
}

Physics::HitResult Physics::SphereTrace(const vec3 start, const vec3 end, float radius, const BodyType mask, const vector<Body*> ignoreList, const vector<Entity*> entityIgnoreList)
{
	HitResult hit;


	hit.fraction = 1.0f;
	hit.position = end;
	hit.normal = vec3(0, 0, 0);
	hit.hitbody = nullptr;
	hit.hasHit = false;
	hit.shapePosition = end;
	hit.entity = nullptr;

	// Convert start and end from your own vector type to Jolt's coordinate system.
	JPH::Vec3 startLoc = ToPhysics(start);
	JPH::Vec3 endLoc = ToPhysics(end);

	// Create a sphere shape for the trace.
	auto sphere_shape_settings = JPH::SphereShapeSettings();
	sphere_shape_settings.SetEmbedded();
	sphere_shape_settings.mRadius = radius;
	JPH::Shape::ShapeResult shape_result = sphere_shape_settings.Create();
	JPH::ShapeRefC sphere_shape = shape_result.Get();

	if (shape_result.HasError())
	{
		Logger::Log(shape_result.GetError().c_str());
		hit.hasHit = false;
		hit.fraction = 1.0f;
		hit.position = end;
		hit.shapePosition = end;
		hit.normal = vec3(0, 0, 0);
		hit.hitbody = nullptr;
		hit.hitboxName = "";
		return hit;
	}

	// Set up the shape cast from startLoc to endLoc.
	JPH::Vec3 direction = endLoc - startLoc;
	JPH::RMat44 start_transform = JPH::RMat44::sTranslation(startLoc);
	JPH::RShapeCast shape_cast(sphere_shape, JPH::Vec3::sReplicate(1.0f), start_transform, direction);



	TraceBodyFilter filter;
	filter.mask = mask;
	filter.ignoreList = ignoreList;
	filter.entityIgnoreList = entityIgnoreList;

	//physicsMainLock.lock();

	// Cast the shape using the narrow phase query.
	ClosestHitShapeCastCollector collector;
	physics_system->GetNarrowPhaseQuery().CastShape(shape_cast, JPH::ShapeCastSettings(), JPH::Vec3::sZero(), collector, {}, {}, filter);
	if (collector.HadHit())
	{


		// Lock the body using the BodyLockInterface to safely access it.
		JPH::BodyLockRead body_lock(physics_system->GetBodyLockInterface(), collector.GetHit().mBodyID2);

		// Retrieve the hit body's surface normal.
		const JPH::Body* body = &body_lock.GetBody();
		if (body)
		{

			const JPH::Shape* root_shape = body->GetShape();
			JPH::SubShapeID remainder;
			const JPH::Shape* hit_shape = root_shape->GetLeafShape(collector.GetHit().mSubShapeID2, remainder);

			auto material = root_shape->GetMaterial(collector.GetHit().mSubShapeID2);

			if (material != JPH::PhysicsMaterial::sDefault)
			{
				hit.surfaceName = ((MySurfaceMaterial*)material)->GetName();
			}

			hit.normal = FromPhysics(body->GetWorldSpaceSurfaceNormal(collector.GetHit().mSubShapeID2, collector.GetHit().mContactPointOn2));

			// Calculate fraction of the hit along the path.
			hit.fraction = collector.GetHit().mFraction;

			hit.shapePosition = mix(start, end, hit.fraction);

			// Compute the hit position in physics space (point on the hit body) and convert it back to your coordinate system.
			hit.position = FromPhysics(collector.GetHit().mContactPointOn2);

			auto* props = reinterpret_cast<BodyData*>(body->GetUserData());

			if (props == nullptr)
			{
				hit.hasHit = false;
				hit.entity = nullptr;
			}
			else
			{
				hit.entity = props->OwnerEntity;

				// Record the hit body.
				hit.hitbody = body;
				hit.hasHit = true;
				hit.hitboxName = props->hitboxName;
			}

		}
	}


	hit.hasHit = hit.hasHit && hit.entity != nullptr;

	if (hit.hasHit)
	{
		if (hit.entity->Destroyed)
			hit.hasHit = false;

	}

	return hit;
}

// Multi Line Trace (collects ALL hits along the ray, sorted closest → farthest)
std::vector<Physics::HitResult> Physics::MultiLineTrace(const vec3 start, const vec3 end, const BodyType mask,
	const vector<Body*> ignoreList, const vector<Entity*> entityIgnoreList)
{
	std::vector<HitResult> outHits;

	JPH::Vec3 startLoc = ToPhysics(start);
	JPH::Vec3 endLoc = ToPhysics(end);

	JPH::RRayCast ray;
	ray.mOrigin = startLoc;
	ray.mDirection = endLoc - startLoc;

	TraceBodyFilter filter;
	filter.mask = mask;
	filter.ignoreList = ignoreList;
	filter.entityIgnoreList = entityIgnoreList;

	// Jolt multi-hit raycast requires RayCastSettings + the templated collector
	JPH::RayCastSettings rayCastSettings; // defaults are fine for most use-cases
	JPH::AllHitCollisionCollector<JPH::CastRayCollector> collector;

	physics_system->GetNarrowPhaseQuery().CastRay(ray, rayCastSettings, collector, {}, {}, filter);

	collector.Sort(); // built-in sort (closest → farthest) – fixes your lambda ambiguity error

	for (const JPH::RayCastResult& result : collector.mHits)
	{
		HitResult hit{};
		hit.fraction = result.mFraction;
		hit.shapePosition = mix(start, end, hit.fraction);
		hit.position = FromPhysics(ray.GetPointOnRay(result.mFraction));
		hit.normal = vec3(0, 0, 0);
		hit.hitbody = nullptr;
		hit.hasHit = false;
		hit.entity = nullptr;
		hit.hitboxName = "";
		hit.surfaceName = "";

		JPH::BodyLockRead body_lock(physics_system->GetBodyLockInterface(), result.mBodyID);
		if (!body_lock.Succeeded()) continue;

		const JPH::Body* body = &body_lock.GetBody();
		if (!body) continue;

		// Same surface normal + name logic as your original LineTrace
		hit.normal = FromPhysics(body->GetWorldSpaceSurfaceNormal(result.mSubShapeID2, ray.GetPointOnRay(result.mFraction)));

		const JPH::Shape* root_shape = body->GetShape();
		
		auto material = root_shape->GetMaterial(result.mSubShapeID2);

		if (material != JPH::PhysicsMaterial::sDefault)
		{
			hit.surfaceName = ((MySurfaceMaterial*)material)->GetName();
		}


		hit.hitbody = body;

		auto* props = reinterpret_cast<BodyData*>(body->GetUserData());
		if (props == nullptr || props->OwnerEntity == nullptr || props->OwnerEntity->Destroyed)
			continue;

		hit.entity = props->OwnerEntity;
		hit.hitboxName = props->hitboxName;
		hit.hasHit = true;

		outHits.push_back(hit);
	}

	return outHits;
}

// Multi Sphere Trace (collects ALL hits along the sphere sweep, sorted closest → farthest)
std::vector<Physics::HitResult> Physics::MultiSphereTrace(const vec3 start, const vec3 end, float radius,
	const BodyType mask, const vector<Body*> ignoreList, const vector<Entity*> entityIgnoreList)
{
	std::vector<HitResult> outHits;

	// Create sphere shape (identical to your single SphereTrace)
	auto sphere_shape_settings = JPH::SphereShapeSettings();
	sphere_shape_settings.SetEmbedded();
	sphere_shape_settings.mRadius = radius;
	JPH::Shape::ShapeResult shape_result = sphere_shape_settings.Create();
	if (shape_result.HasError())
	{
		Logger::Log(shape_result.GetError().c_str());
		return outHits;
	}
	JPH::ShapeRefC sphere_shape = shape_result.Get();

	JPH::Vec3 startLoc = ToPhysics(start);
	JPH::Vec3 endLoc = ToPhysics(end);
	JPH::Vec3 direction = endLoc - startLoc;
	JPH::RMat44 start_transform = JPH::RMat44::sTranslation(startLoc);
	JPH::RShapeCast shape_cast(sphere_shape, JPH::Vec3::sReplicate(1.0f), start_transform, direction);

	TraceBodyFilter filter;
	filter.mask = mask;
	filter.ignoreList = ignoreList;
	filter.entityIgnoreList = entityIgnoreList;

	// Correct Jolt multi-hit collector for shape casts
	JPH::AllHitCollisionCollector<JPH::CastShapeCollector> collector;
	physics_system->GetNarrowPhaseQuery().CastShape(shape_cast, JPH::ShapeCastSettings(), JPH::Vec3::sZero(), collector, {}, {}, filter);

	collector.Sort(); // built-in sort (closest → farthest)

	for (const JPH::ShapeCastResult& result : collector.mHits)
	{
		HitResult hit{};
		hit.fraction = result.mFraction;
		hit.shapePosition = mix(start, end, hit.fraction);
		hit.position = FromPhysics(result.mContactPointOn2);
		hit.normal = vec3(0, 0, 0);
		hit.hitbody = nullptr;
		hit.hasHit = false;
		hit.entity = nullptr;
		hit.hitboxName = "";
		hit.surfaceName = "";

		JPH::BodyLockRead body_lock(physics_system->GetBodyLockInterface(), result.mBodyID2);
		if (!body_lock.Succeeded()) continue;

		const JPH::Body* body = &body_lock.GetBody();
		if (!body) continue;

		// Same normal + surface name logic as your original SphereTrace
		hit.normal = FromPhysics(body->GetWorldSpaceSurfaceNormal(result.mSubShapeID2, result.mContactPointOn2));

		const JPH::Shape* root_shape = body->GetShape();
		
		auto material = root_shape->GetMaterial(result.mSubShapeID2);

		if (material != JPH::PhysicsMaterial::sDefault)
		{
			hit.surfaceName = ((MySurfaceMaterial*)material)->GetName();
		}

		hit.hitbody = body;

		auto* props = reinterpret_cast<BodyData*>(body->GetUserData());
		if (props == nullptr || props->OwnerEntity == nullptr || props->OwnerEntity->Destroyed)
			continue;

		hit.entity = props->OwnerEntity;
		hit.hitboxName = props->hitboxName;
		hit.hasHit = true;

		outHits.push_back(hit);
	}

	return outHits;
}

std::vector<Physics::HitResult> Physics::MultiSphereOverlap(const vec3 center, float radius,
	const BodyType mask, const vector<Body*> ignoreList, const vector<Entity*> entityIgnoreList)
{
	std::vector<HitResult> outHits;

	// Create the sphere shape
	auto sphere_shape_settings = JPH::SphereShapeSettings(radius);  // simpler constructor
	sphere_shape_settings.SetEmbedded();
	JPH::Shape::ShapeResult shape_result = sphere_shape_settings.Create();
	if (shape_result.HasError())
	{
		Logger::Log(shape_result.GetError().c_str());
		return outHits;
	}
	JPH::ShapeRefC sphere_shape = shape_result.Get();   // keep as RefC, do NOT call .Get() again

	JPH::Vec3 physCenter = ToPhysics(center);
	JPH::RMat44 transform = JPH::RMat44::sTranslation(physCenter);

	TraceBodyFilter filter;
	filter.mask = mask;
	filter.ignoreList = ignoreList;
	filter.entityIgnoreList = entityIgnoreList;

	// Correct collector for CollideShape
	JPH::AllHitCollisionCollector<JPH::CollideShapeCollector> collector;

	// Correct CollideShape call
	JPH::CollideShapeSettings collideSettings;   // default settings are usually fine

	physics_system->GetNarrowPhaseQuery().CollideShape(
		sphere_shape,                          // const Shape*
		JPH::Vec3::sReplicate(1.0f),           // shape scale
		transform,                             // center of mass transform
		collideSettings,                       // CollideShapeSettings (was missing)
		JPH::RVec3::sZero(),                   // base offset (usually zero)
		collector,                             // collector
		{},                                    // BroadPhaseLayerFilter (default)
		{},                                    // ObjectLayerFilter (default)
		filter,                                // BodyFilter (your TraceBodyFilter should derive from BodyFilter)
		{}                                     // ShapeFilter (default)
	);

	collector.Sort();   // closest to farthest based on penetration

	for (const JPH::CollideShapeResult& result : collector.mHits)
	{
		HitResult hit{};
		hit.fraction = 0.0f;
		hit.shapePosition = center;
		hit.position = FromPhysics(result.mContactPointOn2);
		hit.normal = FromPhysics(result.mPenetrationAxis.NormalizedOr(JPH::Vec3(0, 1, 0))); // safe fallback
		hit.hitbody = nullptr;
		hit.hasHit = false;
		hit.entity = nullptr;
		hit.hitboxName = "";
		hit.surfaceName = "";

		JPH::BodyLockRead body_lock(physics_system->GetBodyLockInterface(), result.mBodyID2);
		if (!body_lock.Succeeded()) continue;

		const JPH::Body* body = &body_lock.GetBody();
		if (!body) continue;

		// Surface name logic (same as your other traces)
		const JPH::Shape* root_shape = body->GetShape();
		
		auto material = root_shape->GetMaterial(result.mSubShapeID2);

		if (material != JPH::PhysicsMaterial::sDefault)
		{
			hit.surfaceName = ((MySurfaceMaterial*)material)->GetName();
		}

		hit.hitbody = body;

		auto* props = reinterpret_cast<BodyData*>(body->GetUserData());
		if (props == nullptr || props->OwnerEntity == nullptr || props->OwnerEntity->Destroyed)
			continue;

		hit.entity = props->OwnerEntity;
		hit.hitboxName = props->hitboxName;
		hit.hasHit = true;

		outHits.push_back(hit);
	}

	return outHits;
}

Physics::HitResult Physics::SphereTraceForEntity(vector<Entity*> entityties, const vec3 start, const vec3 end, float radius, const BodyType mask, const vector<Body*> ignoreList)
{
	HitResult hit;


	hit.fraction = 1.0f;
	hit.position = end;
	hit.normal = vec3(0, 0, 0);
	hit.hitbody = nullptr;
	hit.hasHit = false;
	hit.shapePosition = end;
	hit.entity = nullptr;

	// Convert start and end from your own vector type to Jolt's coordinate system.
	JPH::Vec3 startLoc = ToPhysics(start);
	JPH::Vec3 endLoc = ToPhysics(end);

	// Create a sphere shape for the trace.
	auto sphere_shape_settings = JPH::SphereShapeSettings();
	sphere_shape_settings.SetEmbedded();
	sphere_shape_settings.mRadius = radius;
	JPH::Shape::ShapeResult shape_result = sphere_shape_settings.Create();
	JPH::ShapeRefC sphere_shape = shape_result.Get();

	if (shape_result.HasError())
	{
		Logger::Log(shape_result.GetError().c_str());
		hit.hasHit = false;
		hit.fraction = 1.0f;
		hit.position = end;
		hit.shapePosition = end;
		hit.normal = vec3(0, 0, 0);
		hit.hitbody = nullptr;
		hit.hitboxName = "";
		return hit;
	}

	// Set up the shape cast from startLoc to endLoc.
	JPH::Vec3 direction = endLoc - startLoc;
	JPH::RMat44 start_transform = JPH::RMat44::sTranslation(startLoc);
	JPH::RShapeCast shape_cast(sphere_shape, JPH::Vec3::sReplicate(1.0f), start_transform, direction);



	TraceBodyFilterIncludeOnly filter;
	filter.includeEntityList = entityties;
	filter.mask = mask;
	filter.ignoreList = ignoreList;

	//physicsMainLock.lock();

	// Cast the shape using the narrow phase query.
	ClosestHitShapeCastCollector collector;
	physics_system->GetNarrowPhaseQuery().CastShape(shape_cast, JPH::ShapeCastSettings(), JPH::Vec3::sZero(), collector, {}, {}, filter);
	if (collector.HadHit())
	{


		// Lock the body using the BodyLockInterface to safely access it.
		JPH::BodyLockRead body_lock(physics_system->GetBodyLockInterface(), collector.GetHit().mBodyID2);

		// Retrieve the hit body's surface normal.
		const JPH::Body* body = &body_lock.GetBody();
		if (body)
		{

			const JPH::Shape* root_shape = body->GetShape();
			
			auto material = root_shape->GetMaterial(collector.GetHit().mSubShapeID2);

			if (material != JPH::PhysicsMaterial::sDefault)
			{
				hit.surfaceName = ((MySurfaceMaterial*)material)->GetName();
			}

			hit.normal = FromPhysics(body->GetWorldSpaceSurfaceNormal(collector.GetHit().mSubShapeID2, collector.GetHit().mContactPointOn2));

			// Calculate fraction of the hit along the path.
			hit.fraction = collector.GetHit().mFraction;

			hit.shapePosition = mix(start, end, hit.fraction);

			// Compute the hit position in physics space (point on the hit body) and convert it back to your coordinate system.
			hit.position = FromPhysics(collector.GetHit().mContactPointOn2);

			auto* props = reinterpret_cast<BodyData*>(body->GetUserData());

			if (props == nullptr)
			{

			}
			else
			{
				hit.entity = props->OwnerEntity;

				// Record the hit body.
				hit.hitbody = body;
				hit.hasHit = true;
				hit.hitboxName = props->hitboxName;
			}

		}
	}


	hit.hasHit = hit.hasHit && hit.entity != nullptr;

	if (hit.hasHit)
	{
		if (hit.entity->Destroyed)
			hit.hasHit = false;

	}

	return hit;
}

Physics::HitResult Physics::CylinderTrace(const vec3 start, const vec3 end, float radius, float halfHeight, const BodyType mask, const vector<Body*> ignoreList, const vector<Entity*> entityIgnoreList)
{
	HitResult hit;
	// Initialize default values
	hit.fraction = 1.0f;
	hit.position = end;
	hit.normal = vec3(0, 0, 0);
	hit.hitbody = nullptr;
	hit.hasHit = false;
	hit.shapePosition = end;
	hit.entity = nullptr;

	// Convert to Jolt coordinates
	JPH::Vec3 startLoc = ToPhysics(start);
	JPH::Vec3 endLoc = ToPhysics(end);
	JPH::Vec3 direction = endLoc - startLoc;

	// Handle zero-length trace
	if (direction.IsNearZero()) {
		return hit;
	}

	// Create cylinder shape
	auto cylinder_shape_settings = JPH::CylinderShapeSettings(halfHeight, radius);
	cylinder_shape_settings.SetEmbedded();
	JPH::Shape::ShapeResult shape_result = cylinder_shape_settings.Create();
	JPH::ShapeRefC cylinder_shape = shape_result.Get();

	if (shape_result.HasError()) {
		Logger::Log(shape_result.GetError().c_str());
		return hit;
	}

	// Align cylinder axis with trace direction
	JPH::Vec3 normalizedDirection = direction.Normalized();
	JPH::Quat rotation = JPH::Quat::sFromTo(JPH::Vec3::sAxisY(), normalizedDirection);
	JPH::RMat44 start_transform = JPH::RMat44::sRotationTranslation(rotation, startLoc);

	// Set up shape cast
	JPH::RShapeCast shape_cast(cylinder_shape, JPH::Vec3::sReplicate(1.0f), start_transform, direction);

	// Configure collision filter
	TraceBodyFilter filter;
	filter.mask = mask;
	filter.ignoreList = ignoreList;
	filter.entityIgnoreList = entityIgnoreList;

	// Perform shape cast
	// physicsMainLock.lock(); // Uncomment if thread safety required
	ClosestHitShapeCastCollector collector;
	physics_system->GetNarrowPhaseQuery().CastShape(shape_cast, JPH::ShapeCastSettings(), JPH::Vec3::sZero(), collector, {}, {}, filter);

	if (collector.HadHit()) {
		JPH::BodyLockRead body_lock(physics_system->GetBodyLockInterface(), collector.GetHit().mBodyID2);
		const JPH::Body* body = &body_lock.GetBody();
		if (body) {
			// Process hit shape
			const JPH::Shape* root_shape = body->GetShape();
			
			auto material = root_shape->GetMaterial(collector.GetHit().mSubShapeID2);

			if (material != JPH::PhysicsMaterial::sDefault)
			{
				hit.surfaceName = ((MySurfaceMaterial*)material)->GetName();
			}

			// Extract hit information
			hit.normal = FromPhysics(body->GetWorldSpaceSurfaceNormal(collector.GetHit().mSubShapeID2, collector.GetHit().mContactPointOn2));
			hit.fraction = collector.GetHit().mFraction;
			hit.shapePosition = mix(start, end, hit.fraction);
			hit.position = FromPhysics(collector.GetHit().mContactPointOn2);

			auto* props = reinterpret_cast<BodyData*>(body->GetUserData());

			if (props == nullptr)
			{
				hit.hasHit = false;
				hit.entity = nullptr;
			}
			else
			{
				hit.entity = props->OwnerEntity;
				hit.hitbody = body;
				hit.hasHit = true;
				hit.hitboxName = props->hitboxName;
			}


		}
	}
	// physicsMainLock.unlock(); // Uncomment if thread safety required


	// Validate hit entity
	hit.hasHit = hit.hasHit && hit.entity != nullptr;
	if (hit.hasHit && hit.entity->Destroyed) {
		hit.hasHit = false;
	}

	return hit;
}

Physics::HitResult Physics::ShapeTrace(const Shape* shape, vec3 start, vec3 end, vec3 scale, const BodyType mask, const vector<Body*> ignoreList, const vector<Entity*> entityIgnoreList)
{
	HitResult hit;
	hit.fraction = 1.0f;
	hit.position = end;
	hit.normal = vec3(0, 0, 0);
	hit.hitbody = nullptr;
	hit.hasHit = false;
	hit.shapePosition = end;
	hit.entity = nullptr;

	JPH::Vec3 startLoc = ToPhysics(start);
	JPH::Vec3 endLoc = ToPhysics(end);
	JPH::Vec3 direction = endLoc - startLoc;

	if (direction.IsNearZero()) {
		return hit;
	}

	JPH::ShapeRefC cylinder_shape = shape;

	JPH::Vec3 normalizedDirection = direction.Normalized();
	JPH::Quat rotation = JPH::Quat::sFromTo(JPH::Vec3::sAxisY(), normalizedDirection);
	JPH::RMat44 start_transform = JPH::RMat44::sRotationTranslation(rotation, startLoc);

	JPH::Vec3 joltScale(scale.x, scale.y, scale.z); // Apply custom scaling
	JPH::RShapeCast shape_cast(cylinder_shape, joltScale, start_transform, direction);

	TraceBodyFilter filter;
	filter.mask = mask;
	filter.ignoreList = ignoreList;
	filter.entityIgnoreList = entityIgnoreList;

	ClosestHitShapeCastCollector collector;
	physics_system->GetNarrowPhaseQuery().CastShape(shape_cast, JPH::ShapeCastSettings(), JPH::Vec3::sZero(), collector, {}, {}, filter);

	if (collector.HadHit()) {
		JPH::BodyLockRead body_lock(physics_system->GetBodyLockInterface(), collector.GetHit().mBodyID2);
		const JPH::Body* body = &body_lock.GetBody();
		if (body) {
			
			const JPH::Shape* root_shape = body->GetShape();
			auto material = root_shape->GetMaterial(collector.GetHit().mSubShapeID2);

			if (material != JPH::PhysicsMaterial::sDefault)
			{
				hit.surfaceName = ((MySurfaceMaterial*)material)->GetName();
			}

			hit.normal = FromPhysics(body->GetWorldSpaceSurfaceNormal(collector.GetHit().mSubShapeID2, collector.GetHit().mContactPointOn2));
			hit.fraction = collector.GetHit().mFraction;
			hit.shapePosition = mix(start, end, hit.fraction);
			hit.position = FromPhysics(collector.GetHit().mContactPointOn2);

			auto* props = reinterpret_cast<BodyData*>(body->GetUserData());

			if (props == nullptr)
			{
				hit.hasHit = false;
				hit.entity = nullptr;
			}
			else
			{
				hit.entity = props->OwnerEntity;
				hit.hitbody = body;
				hit.hasHit = hit.entity != nullptr && !hit.entity->Destroyed;
				hit.hitboxName = props->hitboxName;
			}


		}
	}

	return hit;
}


bool TraceBodyFilter::ShouldCollideLocked(const Body& inBody) const
{

	// Check if the body is in the ignore list.
	for (Body* ignored : ignoreList)
	{
		if (ignored == &inBody)
			return false;
	}

	// Retrieve collision properties from the body's user data.
	// It is assumed that user data points to a CollisionProperties struct.
	auto* properties = reinterpret_cast<BodyData*>(inBody.GetUserData());
	if (properties)
	{

		if (properties->sensorCollisionMode == SensorCollisionMode::CollideOnlyWithSensors)
			return false;

		for (auto ignored : entityIgnoreList)
		{
			if (ignored == properties->OwnerEntity)
				return false;
		}

		if (properties->OwnerEntity)
		{
			if (properties->OwnerEntity->Destroyed)
			{
				return false;
			}
		}

		const BodyType AllowedSensorMask =
			BodyType::Liquid |
			BodyType::Area1 |
			BodyType::Area2 |
			BodyType::Area3 |
			BodyType::Area4 |
			BodyType::Area5;

		if (inBody.IsSensor() && !(properties->group & AllowedSensorMask))
			return false;


		// Check if the body's group is included in our filter's mask.
		// If the bitwise AND of mask and the body's group is zero, they don't match.
		if ((static_cast<uint32_t>(mask) & static_cast<uint32_t>(properties->group)) == 0)
			return false;
	}

	// Accept the collision if no condition rejects it.
	return true;
}


Body* Physics::CreateCharacterBody(Entity* owner, vec3 Position, float Radius, float Height, float Mass,
	BodyType group,
	BodyType mask)
{
	// Calculate cylinder portion height (total capsule height = cylinder_height + 2 * radius)
	float cylinder_half_height = (Height - 2.0f * Radius) / 2.0f;
	if (cylinder_half_height < 0.0f) {
		Logger::Log("Capsule height is too small for given radius, using minimum height");
		cylinder_half_height = 0.0f;
	}

	// Create capsule shape
	auto capsule_shape_settings = JPH::CapsuleShapeSettings();
	capsule_shape_settings.SetEmbedded();
	capsule_shape_settings.mRadius = Radius;
	capsule_shape_settings.mHalfHeightOfCylinder = cylinder_half_height;

	JPH::Shape::ShapeResult shape_result = capsule_shape_settings.Create();
	JPH::Shape* capsule_shape = shape_result.Get();

	if (shape_result.HasError())
		Logger::Log(shape_result.GetError().c_str());

	// Configure body settings
	JPH::BodyCreationSettings body_settings(
		capsule_shape,
		ToPhysics(Position),
		JPH::Quat::sIdentity(),
		JPH::EMotionType::Dynamic,  // Dynamic body type
		Layers::MOVING             // Use moving layer
	);

	body_settings.mMotionQuality = EMotionQuality::LinearCast;

	// Lock rotation in all axes
	body_settings.mAllowedDOFs = JPH::EAllowedDOFs::TranslationX | JPH::EAllowedDOFs::TranslationY | JPH::EAllowedDOFs::TranslationZ;  // Allow only translation

	// Set mass properties
	body_settings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
	body_settings.mMassPropertiesOverride.mMass = Mass;
	body_settings.mFriction = 0.0f;  // Match box friction
	body_settings.mRestitution = 0.0f; // No bounciness

	// Allocate and attach collision properties to the body via the user data field:
	BodyData* properties = new BodyData{ group, mask,false, owner };
	properties->dynamicCollisionGroupOrMask = true;
	body_settings.mUserData = reinterpret_cast<uintptr_t>(properties);

	// Create and add body to world
	JPH::Body* character_body = bodyInterface->CreateBody(body_settings);
	
	AddBody(character_body);

	return character_body;
}

Body* Physics::CreateCharacterCylinderBody(Entity* owner, vec3 Position, float Radius, float Height, float Mass,
	BodyType group, BodyType mask)
{
	// Calculate cylinder half height (total height = 2 * half_height)
	float cylinder_half_height = Height * 0.5f;
	if (cylinder_half_height <= 0.0f) {
		Logger::Log("Cylinder height must be positive, using minimal value");
		cylinder_half_height = 0.01f;
	}

	// Create cylinder shape
	auto cylinder_shape_settings = JPH::CylinderShapeSettings(
		cylinder_half_height,
		Radius
	);
	cylinder_shape_settings.SetEmbedded();

	JPH::Shape::ShapeResult shape_result = cylinder_shape_settings.Create();
	JPH::Shape* cylinder_shape = shape_result.Get();

	if (shape_result.HasError())
		Logger::Log(shape_result.GetError().c_str());

	// Configure body settings (identical to capsule version)
	JPH::BodyCreationSettings body_settings(
		cylinder_shape,
		ToPhysics(Position),
		JPH::Quat::sIdentity(),
		JPH::EMotionType::Dynamic,  // Dynamic body type
		Layers::MOVING              // Use moving layer
		
	);

	body_settings.mMotionQuality = EMotionQuality::LinearCast;
	body_settings.mAllowedDOFs = JPH::EAllowedDOFs::TranslationX |
		JPH::EAllowedDOFs::TranslationY |
		JPH::EAllowedDOFs::TranslationZ;  // Lock rotation
	body_settings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
	body_settings.mMassPropertiesOverride.mMass = Mass;
	body_settings.mFriction = 0.0f;
	body_settings.mRestitution = 0.0f;

	// Attach collision properties
	BodyData* properties = new BodyData{ group, mask, false, owner };
	properties->dynamicCollisionGroupOrMask = true;
	body_settings.mUserData = reinterpret_cast<uintptr_t>(properties);

	// Create and add body to world
	JPH::Body* cylinder_body = bodyInterface->CreateBody(body_settings);
	AddBody(cylinder_body);

	return cylinder_body;
}

bool TraceBodyFilterIncludeOnly::ShouldCollideLocked(const Body& inBody) const
{
	// Check if the body is in the ignore list.
	for (Body* ignored : ignoreList)
	{
		if (ignored == &inBody)
			return false;
	}

	// Retrieve collision properties from the body's user data.
	// It is assumed that user data points to a CollisionProperties struct.
	auto* properties = reinterpret_cast<BodyData*>(inBody.GetUserData());
	if (properties)
	{


		if (properties->sensorCollisionMode == SensorCollisionMode::CollideOnlyWithSensors)
			return false;

		if (properties->OwnerEntity)
		{
			if (properties->OwnerEntity->Destroyed)
			{
				return false;
			}
		}

		for (auto entity : includeEntityList)
		{

			if (entity != properties->OwnerEntity)
			{
				return false;
			}

		}

		if( properties->OwnerEntity)

		if (inBody.IsSensor() && properties->group != BodyType::Liquid)
			return false;

		// Check if the body's group is included in our filter's mask.
		// If the bitwise AND of mask and the body's group is zero, they don't match.
		if ((static_cast<uint32_t>(mask) & static_cast<uint32_t>(properties->group)) == 0)
			return false;
	}

	// Accept the collision if no condition rejects it.
	return true;
}

#ifdef JPH_DEBUG_RENDERER


bool DrawFilter::ShouldDraw(const Body& inBody) const
{


	// Check if the body is in the ignore list.
	for (Body* ignored : ignoreList)
	{
		if (ignored == &inBody)
			return false;
	}

	// Retrieve collision properties from the body's user data.
	// It is assumed that user data points to a CollisionProperties struct.
	auto* properties = reinterpret_cast<BodyData*>(inBody.GetUserData());
	if (properties)
	{
		// Check if the body's group is included in our filter's mask.
		// If the bitwise AND of mask and the body's group is zero, they don't match.
		if ((static_cast<uint32_t>(mask) & static_cast<uint32_t>(properties->group)) == 0)
			return false;
	}
	else
	{
		return false;
	}

	// Accept the collision if no condition rejects it.
	return true;
}
#endif // DEBUG

MySurfaceMaterial::MySurfaceMaterial(uint32_t surfaceId)
{

	mSurfaceId = surfaceId;


}

std::string MySurfaceMaterial::GetName() const
{
	return Physics::FindSurfacyById(mSurfaceId);
}
