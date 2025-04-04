#pragma once

#if __EMSCRIPTEN__ // i'm too tired right now to figure proper way

#define NDEBUG 1
#define JPH_OBJECT_STREAM 1

#endif // __EMSCRIPTEN__

#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Character/CharacterVirtual.h>

#include "PhysicsConverter.h"

#include "glm.h"

#include <iostream>

#include "Time.hpp"

#include "Logger.hpp"

#include "MathHelper.hpp"

using namespace JPH;

using namespace std;

// Layer that objects can be in, determines which other objects it can collide with
// Typically you at least want to have 1 layer for moving bodies and 1 layer for static bodies, but you can have more
// layers if you want. E.g. you could have a layer for high detail collision (which is not used by the physics simulation
// but only if you do collision testing).
namespace Layers
{
	static constexpr ObjectLayer NON_MOVING = 0;
	static constexpr ObjectLayer MOVING = 1;
	static constexpr ObjectLayer NUM_LAYERS = 2;
};

/// Class that determines if two object layers can collide
class ObjectLayerPairFilterImpl : public ObjectLayerPairFilter
{
public:
	virtual bool					ShouldCollide(ObjectLayer inObject1, ObjectLayer inObject2) const override
	{
		switch (inObject1)
		{
		case Layers::NON_MOVING:
			return inObject2 == Layers::MOVING; // Non moving only collides with moving
		case Layers::MOVING:
			return true; // Moving collides with everything
		default:
			JPH_ASSERT(false);
			return false;
		}
	}
};

// Each broadphase layer results in a separate bounding volume tree in the broad phase. You at least want to have
// a layer for non-moving and moving objects to avoid having to update a tree full of static objects every frame.
// You can have a 1-on-1 mapping between object layers and broadphase layers (like in this case) but if you have
// many object layers you'll be creating many broad phase trees, which is not efficient. If you want to fine tune
// your broadphase layers define JPH_TRACK_BROADPHASE_STATS and look at the stats reported on the TTY.
namespace BroadPhaseLayers
{
	static constexpr BroadPhaseLayer NON_MOVING(0);
	static constexpr BroadPhaseLayer MOVING(1);
	static constexpr uint NUM_LAYERS(2);
};

// BroadPhaseLayerInterface implementation
// This defines a mapping between object and broadphase layers.
class BPLayerInterfaceImpl final : public BroadPhaseLayerInterface
{
public:
	BPLayerInterfaceImpl()
	{
		// Create a mapping table from object to broad phase layer
		mObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
		mObjectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
	}

	virtual uint					GetNumBroadPhaseLayers() const override
	{
		return BroadPhaseLayers::NUM_LAYERS;
	}

	virtual BroadPhaseLayer			GetBroadPhaseLayer(ObjectLayer inLayer) const override
	{
		JPH_ASSERT(inLayer < Layers::NUM_LAYERS);
		return mObjectToBroadPhase[inLayer];
	}

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
	virtual const char* GetBroadPhaseLayerName(BroadPhaseLayer inLayer) const override
	{
		switch ((BroadPhaseLayer::Type)inLayer)
		{
		case (BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING:	return "NON_MOVING";
		case (BroadPhaseLayer::Type)BroadPhaseLayers::MOVING:		return "MOVING";
		default:													JPH_ASSERT(false); return "INVALID";
		}
	}
#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

private:
	BroadPhaseLayer					mObjectToBroadPhase[Layers::NUM_LAYERS];
};

/// Class that determines if an object layer can collide with a broadphase layer
class ObjectVsBroadPhaseLayerFilterImpl : public ObjectVsBroadPhaseLayerFilter
{
public:
	virtual bool				ShouldCollide(ObjectLayer inLayer1, BroadPhaseLayer inLayer2) const override
	{
		switch (inLayer1)
		{
		case Layers::NON_MOVING:
			return inLayer2 == BroadPhaseLayers::MOVING;
		case Layers::MOVING:
			return true;
		default:
			JPH_ASSERT(false);
			return false;
		}
	}
};

// An example contact listener
class MyContactListener : public ContactListener
{
public:
	// See: ContactListener
	virtual ValidateResult	OnContactValidate(const Body& inBody1, const Body& inBody2, RVec3Arg inBaseOffset, const CollideShapeResult& inCollisionResult) override
	{
		//cout << "Contact validate callback" << endl;

		// Allows you to ignore a contact before it is created (using layers to not make objects collide is cheaper!)
		return ValidateResult::AcceptAllContactsForThisBodyPair;
	}

	virtual void			OnContactAdded(const Body& inBody1, const Body& inBody2, const ContactManifold& inManifold, ContactSettings& ioSettings) override
	{
		//cout << "A contact was added" << endl;
	}

	virtual void			OnContactPersisted(const Body& inBody1, const Body& inBody2, const ContactManifold& inManifold, ContactSettings& ioSettings) override
	{
		//cout << "A contact was persisted" << endl;
	}

	virtual void			OnContactRemoved(const SubShapeIDPair& inSubShapePair) override
	{
		//cout << "A contact was removed" << endl;
	}
};

class Physics
{

private:

	static TempAllocatorImpl* tempMemAllocator;

	static JobSystemThreadPool* threadPool;

	static BPLayerInterfaceImpl* broad_phase_layer_interface;

	static ObjectVsBroadPhaseLayerFilterImpl* object_vs_broadphase_layer_filter;

	static ObjectLayerPairFilterImpl* object_vs_object_layer_filter;

	static PhysicsSystem* physics_system;

	static MyContactListener* contact_listener;

	static BodyInterface* bodyInterface;

	static vector<Body*> existingBodies;

	static mutex physicsMainLock;

public:
	
	static void AddBody(Body* body)
	{
		physicsMainLock.lock();

		existingBodies.push_back(body);

		bodyInterface->AddBody(body->GetID(), JPH::EActivation::Activate);

		physicsMainLock.unlock();

	}

	static void DestroyBody(Body* body)
	{

		physicsMainLock.lock();

		bodyInterface->RemoveBody(body->GetID());

		bodyInterface->DestroyBody(body->GetID());

		physicsMainLock.unlock();
	}

	static void Init()
	{
		RegisterDefaultAllocator();
		Factory::sInstance = new Factory();

		RegisterTypes();

		tempMemAllocator = new TempAllocatorImpl(30 * 1024 * 1024);

		threadPool = new JobSystemThreadPool(cMaxPhysicsJobs, cMaxPhysicsBarriers, thread::hardware_concurrency() - 2);

		const uint cMaxBodies = 65536;

		const uint cNumBodyMutexes = 0;

		const uint cMaxBodyPairs = 65536;

		const uint cMaxContactConstraints = 20240;

		object_vs_broadphase_layer_filter = new ObjectVsBroadPhaseLayerFilterImpl();

		object_vs_object_layer_filter = new ObjectLayerPairFilterImpl();

		broad_phase_layer_interface = new BPLayerInterfaceImpl();

		physics_system = new PhysicsSystem();

		physics_system->Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints, *broad_phase_layer_interface, *object_vs_broadphase_layer_filter, *object_vs_object_layer_filter);


		contact_listener = new MyContactListener();
		physics_system->SetContactListener(contact_listener);

		bodyInterface = &physics_system->GetBodyInterface();
		

	}

	static void SetBodyPosition(Body* body,vec3 pos)
	{
		bodyInterface->SetPosition(body->GetID(), ToPhysics(pos), JPH::EActivation::Activate);
	}

	static void SetBodyPositionAndRotation(Body* body, vec3 pos, quat rot)
	{
		bodyInterface->SetPositionAndRotation(body->GetID(), ToPhysics(pos),ToPhysics(rot), JPH::EActivation::Activate);
	}

	static void SetBodyPositionAndRotation(Body* body, vec3 pos, vec3 rot)
	{
		bodyInterface->SetPositionAndRotation(body->GetID(), ToPhysics(pos), ToPhysics(MathHelper::GetRotationQuaternion(rot)), JPH::EActivation::Activate);
	}

	static void Simulate()
	{
		physicsMainLock.lock();
		physics_system->Update(Time::DeltaTime, 1, tempMemAllocator, threadPool);
		physicsMainLock.unlock();
	}

	static void Update()
	{

	}

	static Body* CreateBoxBody(vec3 Position, vec3 Size, float Mass = 10, bool Static = false)
	{

		auto box_shape_settings = new JPH::BoxShapeSettings();
		box_shape_settings->SetEmbedded();
		box_shape_settings->mHalfExtent = ToPhysics(Size)/2.0f;
		JPH::Shape::ShapeResult shape_result = box_shape_settings->Create();
		JPH::Shape* box_shape = shape_result.Get();

		if (shape_result.HasError())
			Logger::Log(shape_result.GetError().c_str());

		JPH::BodyCreationSettings body_settings(
			box_shape,                   // Collision shape (e.g., a box shape)
			ToPhysics(Position),          // Initial position
			JPH::Quat::sIdentity(),       // No initial rotation
			Static ? JPH::EMotionType::Static : JPH::EMotionType::Dynamic,    // Dynamic so that it is affected by forces
			Static ? Layers::NON_MOVING : Layers::MOVING);          // Collision layer for moving objects

		body_settings.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
		body_settings.mMassPropertiesOverride.mMass = Mass;
		body_settings.mFriction = 0.5f;

		JPH::Body* dynamic_body = bodyInterface->CreateBody(body_settings);

		AddBody(dynamic_body);

		return dynamic_body;
	}

	static Body* CreateCharacterBody(vec3 Position, float Radius, float Height, float Mass)
	{
		// Calculate cylinder portion height (total capsule height = cylinder_height + 2 * radius)
		float cylinder_half_height = (Height - 2.0f * Radius) / 2.0f;
		if (cylinder_half_height < 0.0f) {
			Logger::Log("Capsule height is too small for given radius, using minimum height");
			cylinder_half_height = 0.0f;
		}

		// Create capsule shape
		auto capsule_shape_settings = new JPH::CapsuleShapeSettings();
		capsule_shape_settings->SetEmbedded();
		capsule_shape_settings->mRadius = Radius;
		capsule_shape_settings->mHalfHeightOfCylinder = cylinder_half_height;

		JPH::Shape::ShapeResult shape_result = capsule_shape_settings->Create();
		JPH::Shape* capsule_shape = shape_result.Get();

		if (shape_result.HasError())
			Logger::Log(shape_result.GetError().c_str());

		// Configure body settings
		JPH::BodyCreationSettings body_settings(
			capsule_shape,
			ToPhysics(Position),
			JPH::Quat::sIdentity(),
			JPH::EMotionType::Dynamic,  // Dynamic body type
			Layers::MOVING              // Use moving layer
		);

		// Lock rotation in all axes
		body_settings.mAllowedDOFs = JPH::EAllowedDOFs::TranslationX | JPH::EAllowedDOFs::TranslationY | JPH::EAllowedDOFs::TranslationZ;  // Allow only translation

		// Set mass properties
		body_settings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
		body_settings.mMassPropertiesOverride.mMass = Mass;
		body_settings.mFriction = 0.5f;  // Match box friction

		// Create and add body to world
		JPH::Body* character_body = bodyInterface->CreateBody(body_settings);
		AddBody(character_body);

		return character_body;
	}


};
