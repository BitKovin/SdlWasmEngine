#pragma once

#ifndef _MSC_VER

#else

#endif



#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <Jolt/Physics/Collision/Shape/OffsetCenterOfMassShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/CylinderShape.h>
#include <Jolt/Physics/Character/CharacterVirtual.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/ShapeCast.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Collision/Shape/CompoundShape.h>
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Core/Reference.h>
#include <Jolt/Physics/Body/BodyLockMulti.h>
#include <Jolt/Physics/Constraints/SwingTwistConstraint.h>
#include <Jolt/Physics/Collision/PhysicsMaterial.h>

#include <unordered_set>

#include "DebugDraw.hpp"

#ifdef JPH_DEBUG_RENDERER
#include <Jolt/Renderer/DebugRendererSimple.h>
#endif

#include "malloc_override.h"
#include <Jolt/Core/TempAllocator.h>

#include "PhysicsConverter.h"
#include "glm.h"
#include <iostream>
#include "Time.hpp"
#include "Logger.hpp"
#include "MathHelper.hpp"
#include "Camera.h"
#include <unordered_map>
#include <set>

using namespace JPH;

using namespace std;

enum BodyType : uint32_t {
	None = 1u << 0,
	MainBody = 1u << 1,
	HitBox = 1u << 2,
	WorldOpaque = 1u << 3,
	CharacterCapsule = 1u << 4,
	NoRayTest = 1u << 5,
	Liquid = 1u << 6,
	WorldTransparent = 1u << 7,
	WorldSkybox = 1u << 8 ,

	Area1 = 1u << 15, //per game logic for areas
	Area2 = 1u << 16, //per game logic for areas
	Area3 = 1u << 17, //per game logic for areas
	Area4 = 1u << 18, //per game logic for areas
	Area5 = 1u << 19, //per game logic for areas

	CustomType1 = 1u << 21, //per game custom type
	CustomType2 = 1u << 22, //per game custom type
	CustomType3 = 1u << 23, //per game custom type
	CustomType4 = 1u << 24, //per game custom type
	CustomType5 = 1u << 25, //per game custom type

	// Combined groups:
	World = WorldOpaque | WorldTransparent,
	GroupAll = MainBody | HitBox | World | CharacterCapsule | NoRayTest | Liquid,
	GroupHitTest = GroupAll & ~CharacterCapsule & ~Liquid,
	GroupCollisionTest = GroupAll & ~HitBox & ~Liquid,
	GroupAllPhysical = GroupHitTest & Liquid,
};

// Overload bitwise operators for BodyType
inline BodyType operator|(BodyType a, BodyType b) {
	return static_cast<BodyType>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

inline BodyType operator&(BodyType a, BodyType b) {
	return static_cast<BodyType>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

inline BodyType operator~(BodyType a) {
	return static_cast<BodyType>(~static_cast<uint32_t>(a));
}

inline BodyType& operator|=(BodyType& a, BodyType b) {
	a = a | b;
	return a;
}

inline BodyType& operator&=(BodyType& a, BodyType b) {
	a = a & b;
	return a;
}

class Entity;
class SkeletalMesh;

enum class SensorCollisionMode
{
	CollideWithEverything,
	NotCollideWithSensors,
	CollideOnlyWithSensors
};

struct BodyData 
{

	~BodyData()
	{
		
	}

	BodyType group = BodyType::None;
	BodyType mask = BodyType::None;

	bool dynamicCollisionGroupOrMask = false;

	Entity* OwnerEntity = nullptr;

	string hitboxName = "";

	SkeletalMesh* OwnerSkeletalMesh = nullptr; //for hitbox bodies

	SensorCollisionMode sensorCollisionMode = SensorCollisionMode::CollideWithEverything;

};

struct ShapeData
{

	string surfaceType = "default";

};

// Layer that objects can be in, determines which other objects it can collide with
// Typically you at least want to have 1 layer for moving bodies and 1 layer for static bodies, but you can have more
// layers if you want. E.g. you could have a layer for high detail collision (which is not used by the physics simulation
// but only if you do collision testing).
namespace Layers
{
	static constexpr ObjectLayer NON_MOVING = 0;
	static constexpr ObjectLayer MOVING = 1;
	static constexpr ObjectLayer TRACE_ONLY = 2;
	static constexpr ObjectLayer NUM_LAYERS = 3;
};


class MySurfaceMaterial : public JPH::PhysicsMaterial
{
public:

	MySurfaceMaterial(uint32_t surfaceId);

	std::string GetName() const;

	// Required by Jolt
	virtual const char* GetDebugName() const override { return GetName().c_str(); }
	virtual JPH::Color GetDebugColor() const override { return JPH::Color::sGreen; } // Pick a color



	int mSurfaceId = 0;
};

/// Class that determines if two object layers can collide
class ObjectLayerPairFilterImpl : public ObjectLayerPairFilter
{
public:
	virtual bool					ShouldCollide(ObjectLayer inObject1, ObjectLayer inObject2) const override
	{

		if (inObject1 == Layers::TRACE_ONLY || inObject2 == Layers::TRACE_ONLY)
			return false;

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

	struct PairHash {
		size_t operator()(const std::pair<uint32_t, uint32_t>& p) const {
			return std::hash<uint64_t>{}((uint64_t)p.first << 32 | p.second);
		}
	};

	// See: ContactListener
	virtual ValidateResult OnContactValidate(const Body& inBody1, const Body& inBody2,
		RVec3Arg inBaseOffset,
		const CollideShapeResult& inCollisionResult) override;
	static void beforeSimulation();
	static void afterSimulation();
	void OnContactAdded(const Body& inBody1, const Body& inBody2, const ContactManifold& inManifold, ContactSettings& ioSettings) override;
	void OnContactPersisted(const Body& inBody1, const Body& inBody2, const ContactManifold& inManifold, ContactSettings& ioSettings) override;
	void OnContactRemoved(const SubShapeIDPair& inSubShapePair) override;

	static void addIgnorePair(const BodyID& body1, const BodyID& body2);
	static void removeIgnorePair(const BodyID& body1, const BodyID& body2);

	static void CleanIgnorePairs();

private:
	static inline std::set<std::pair<Entity*, Entity*>> previousContacts;
	static inline std::set<std::pair<Entity*, Entity*>> currentContacts;
	static inline std::unordered_set<std::pair<uint32_t, uint32_t>, PairHash> ignoredPairs;
};

class TraceBodyFilter : public BodyFilter
{
public:
	// List of bodies to ignore during the ray cast.
	std::vector<Body*> ignoreList = {};
	std::vector<Entity*> entityIgnoreList = {};
	// Collision mask to filter out bodies that don't belong to the desired groups.
	BodyType mask = BodyType::GroupHitTest;

	TraceBodyFilter() = default;
	virtual ~TraceBodyFilter() = default;

	/// Filter function when the Body is locked.
	virtual bool ShouldCollideLocked(const Body& inBody) const override;
};

class TraceBodyFilterIncludeOnly : public BodyFilter
{
public:
	// List of bodies to ignore during the ray cast.
	std::vector<Body*> ignoreList = {};

	std::vector<Entity*> includeEntityList = {};

	// Collision mask to filter out bodies that don't belong to the desired groups.
	BodyType mask = BodyType::GroupHitTest;

	TraceBodyFilterIncludeOnly() = default;
	virtual ~TraceBodyFilterIncludeOnly() = default;

	/// Filter function when the Body is locked.
	virtual bool ShouldCollideLocked(const Body& inBody) const override;
};

#ifdef JPH_DEBUG_RENDERER

class DrawFilter : public BodyDrawFilter
{
public:

	// List of bodies to ignore during the ray cast.
	std::vector<Body*> ignoreList = {};
	// Collision mask to filter out bodies that don't belong to the desired groups.
	BodyType mask = BodyType::GroupAll & ~ BodyType::CharacterCapsule;

	/// Filter function. Returns true if inBody should be rendered
	bool ShouldDraw([[maybe_unused]] const Body& inBody) const override;
};


class MyDebugRenderer : public JPH::DebugRendererSimple
{
public:
	MyDebugRenderer()
	{
		// Must call Initialize() to set sInstance
		Initialize();
	}
	~MyDebugRenderer() override
	{
		if (sInstance == this)
			sInstance = nullptr;
	}

	// 1) Wireframe line
	void DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor) override
	{

		vec3 start = FromPhysics(inFrom);
		vec3 end = FromPhysics(inTo);

		if (distance(Camera::position, start) > 100 && distance(Camera::position, end) > 100) return;

		DebugDraw::Line(start, end, Time::DeltaTimeF*1.5f, 0.01);
	}

	/*
	// 2) Filled triangle (optional—Simple already falls back to DrawLine)
	void DrawTriangle(JPH::RVec3Arg inV1, JPH::RVec3Arg inV2, JPH::RVec3Arg inV3, JPH::ColorArg inColor, ECastShadow inCastShadow) override
	{
		DrawDebugTriangle(Convert(inV1), Convert(inV2), Convert(inV3), Convert(inColor));
	}
	*/
	// 3) 3D text (optional)
	void DrawText3D(JPH::RVec3Arg inPos, const std::string_view& inString, JPH::ColorArg inColor, float inHeight) override
	{
		// e.g. DrawDebugText3D(...)
	}
};

#endif // JPH_DEBUG_RENDERER




class Physics
{

private:

	static TempAllocatorImpl* tempMemAllocator;

	static JobSystemThreadPool* threadPool;

	static BPLayerInterfaceImpl* broad_phase_layer_interface;

	static ObjectVsBroadPhaseLayerFilterImpl* object_vs_broadphase_layer_filter;

	static ObjectLayerPairFilterImpl* object_vs_object_layer_filter;

	static unordered_map<BodyID, Body*> bodyIdMap;

	static MyContactListener* contact_listener;

	static BodyInterface* bodyInterface;

	static vector<Body*> existingBodies;

	static std::unordered_map<std::string, uint32_t> SurfaceIds;
	static std::unordered_map<uint32_t, std::string> SurfaceNames;
	static uint32_t nextSurfaceId;

	static inline std::unordered_map<TwoBodyConstraint*, float> mConstraintBaseTorque;
	static inline std::unordered_map<TwoBodyConstraint*, float> mConstraintLastStrength;
	static inline std::unordered_map<TwoBodyConstraint*, JPH::Quat> mConstraintPrevTarget;


#ifdef JPH_DEBUG_RENDERER
	static MyDebugRenderer* debugRenderer;
#endif

	static BodyType DebugDrawMask;

public:

	static inline std::unordered_set<Body*> ExcludedDrawBodies;


	struct PendingBodyEnterPair
	{
		Entity* target = nullptr;

		Entity* entity = nullptr;

		bool operator==(const PendingBodyEnterPair& other) const
		{
			return target == other.target && entity == other.entity;
		}

	};

	static std::recursive_mutex physicsMainLock;

	static PhysicsSystem* physics_system;
	
	static std::vector<Physics::PendingBodyEnterPair> gRemovals;
	static std::vector<Physics::PendingBodyEnterPair> gAdds;

	static bool DebugDraw;

	static void AddBody(Body* body)
	{
		physicsMainLock.lock();


		existingBodies.push_back(body);

		bodyInterface->AddBody(body->GetID(), JPH::EActivation::Activate);

		bodyIdMap[body->GetID()] = body;


		physicsMainLock.unlock();

	}

	static void SetGravity(vec3 gravity);

	static Body* GetBodyFromId(BodyID id)
	{
		auto res = bodyIdMap.find(id);

		if (res != bodyIdMap.end())
		{
			return res->second;
		}

		return nullptr;

	}

	static void MoveBody(Body* body, vec3 offset)
	{
		if (body == nullptr) return;

		auto data = GetBodyData(body);

		if (data == nullptr) return;


		auto currentPos = body->GetPosition();
		auto targetPos = FromPhysics(currentPos) + offset;

		auto hit = SphereTrace(FromPhysics(currentPos), targetPos, 0.3, data->mask, {body});
		
		if (hit.hasHit)
		{
			targetPos = hit.shapePosition;
		}

		SetBodyPosition(body, targetPos);

	}

	static void SweepBody(Body* body, vec3 position, std::vector<Entity*> ignoreEntities = {})
	{
		if (body == nullptr) return;

		auto data = GetBodyData(body);

		if (data == nullptr) return;


		auto currentPos = body->GetPosition();
		auto targetPos = position;

		auto hit = ShapeTrace(body->GetShape(), FromPhysics(currentPos), targetPos,vec3(1.00), data->mask, {body}, ignoreEntities);

		if (hit.hasHit)
		{
			targetPos = hit.shapePosition;
		}

		SetBodyPosition(body, targetPos);

	}

	static void DestroyBody(Body* body);

	static void DestroyConstraint(Constraint* constraint);

	static void SetMotionType(Body* body, JPH::EMotionType type);

	static BodyData* GetBodyData(const Body* body)
	{
		auto* props = reinterpret_cast<BodyData*>(body->GetUserData());
		return props;
	}

	static void SetBodyType(Body* body, BodyType bodyType)
	{
		if (body == nullptr) return;

		auto* props = reinterpret_cast<BodyData*>(body->GetUserData());
		if (props)
		{
			props->group = bodyType;
		}

	}

	static BodyType GetBodyType(const Body* body)
	{
		if (body == nullptr) return BodyType::None;

		auto* props = reinterpret_cast<BodyData*>(body->GetUserData());
		if (props)
		{
			return props->group;
		}

		return BodyType::None;

	}

	static void SetCollisionMask(const Body* body, BodyType mask)
	{
		if (body == nullptr) return;

		auto* props = reinterpret_cast<BodyData*>(body->GetUserData());
		if (props)
		{
			props->mask = mask;
		}

	}

	static void AddIgnorePair(const BodyID& bodyA, const BodyID& bodyB);

	static void RemoveIgnorePair(const BodyID& bodyA, const BodyID& bodyB);

	static BodyType GetCollisionMask(Body* body)
	{
		if (body == nullptr) return BodyType::None;

		auto* props = reinterpret_cast<BodyData*>(body->GetUserData());
		if (props)
		{
			return props->mask;
		}

		return BodyType::None;

	}

	static void DestroyAllBodies()
	{
		physicsMainLock.lock();

		auto bodiesToDelete = existingBodies;

		for (Body* body : bodiesToDelete)
		{
			DestroyBody(body);
		}
		existingBodies.clear();

		physicsMainLock.unlock();

		ExcludedDrawBodies.clear();
	}

	static void Init();


	static void SetBodyPosition(Body* body,vec3 pos)
	{
		bodyInterface->SetPosition(body->GetID(), ToPhysics(pos), JPH::EActivation::Activate);
	}

	static void SetBodyRotation(Body* body, quat rot)
	{
		bodyInterface->SetRotation(body->GetID(), ToPhysics(rot), JPH::EActivation::Activate);
	}

	static void SetBodyPositionAndRotation(Body* body, vec3 pos, quat rot)
	{
		bodyInterface->SetPositionAndRotation(body->GetID(), ToPhysics(pos),ToPhysics(rot), JPH::EActivation::Activate);
	}

	static void SetBodyPositionAndRotationNoActivation(Body* body, vec3 pos, quat rot)
	{
		bodyInterface->SetPositionAndRotation(body->GetID(), ToPhysics(pos), ToPhysics(rot), JPH::EActivation::DontActivate);
	}

	static void SetBodyPositionAndRotation(Body* body, vec3 pos, vec3 rot)
	{
		bodyInterface->SetPositionAndRotation(body->GetID(), ToPhysics(pos), ToPhysics(MathHelper::GetRotationQuaternion(rot)), JPH::EActivation::Activate);
	}

	static void MoveKinematic(Body* body, vec3 pos, vec3 rot)
	{
		if (!body) return;

		// target transforms
		JPH::RVec3 targetPos = ToPhysics(pos);
		JPH::Quat  targetRot = ToPhysics(MathHelper::GetRotationQuaternion(rot));

		// MoveKinematic every physics update (wakes body if sleeping; updates internal velocities).
		bodyInterface->MoveKinematic(body->GetID(), targetPos, targetRot, Time::DeltaTimeF);

		// Make sure it's active (optional, MoveKinematic should already wake it, but this is safe):
		bodyInterface->ActivateBody(body->GetID());
	}

	static void UpdatePendingBodyExitsEnters();

	static void SimulateWorldNoContacts()
	{
		physics_system->Update(Time::DeltaTime, 1, tempMemAllocator, threadPool);
	}

	static void Simulate()
	{

		MyContactListener::beforeSimulation();
		physics_system->Update(Time::DeltaTime, 1, tempMemAllocator, threadPool);
		MyContactListener::afterSimulation();

		UpdatePendingBodyExitsEnters();
		
	}

	static void OptimizeWorld()
	{
		physics_system->OptimizeBroadPhase();
	}

	static void ResetSimulation()
	{

		MyContactListener::beforeSimulation();
		MyContactListener::beforeSimulation();
		gAdds.clear();
		gRemovals.clear();

	}

	static void UpdateDebugDraw()
	{
#ifdef JPH_DEBUG_RENDERER
		if (DebugDraw)
		{

			BodyManager::DrawSettings draw_settings;
			draw_settings.mDrawShape = true;
			draw_settings.mDrawShapeWireframe = true;
			
		

			DrawFilter filter;
			filter.mask = DebugDrawMask;
			filter.ignoreList = std::vector<Body*>(ExcludedDrawBodies.begin(), ExcludedDrawBodies.end());

			physics_system->DrawBodies(draw_settings, debugRenderer, &filter);      // draws all bodies
			//physics_system->DrawConstraints(debugRenderer);
			//physics_system->DrawConstraintLimits(debugRenderer);
			//physics_system->DrawConstraintReferenceFrame(debugRenderer);

		}
#endif
	}

	static void DrawConstraint(Constraint* constraint);

	static Body* CreateBoxBody(Entity* owner, vec3 Position, vec3 Size, float Mass = 10, bool Static = false,
		BodyType group = BodyType::MainBody,
		BodyType mask = BodyType::GroupCollisionTest)
	{
		// Create a box shape (existing code)
		BoxShapeSettings box_shape_settings = BoxShapeSettings();
		box_shape_settings.SetEmbedded();
		box_shape_settings.mHalfExtent = ToPhysics(Size) / 2.0f;
		JPH::Shape::ShapeResult shape_result = box_shape_settings.Create();
		JPH::Shape* box_shape = shape_result.Get();

		if (shape_result.HasError())
			Logger::Log(shape_result.GetError().c_str());

		// Choose a collision layer based on dynamic or static:
		JPH::BodyCreationSettings body_settings(
			box_shape,
			ToPhysics(Position),
			JPH::Quat::sIdentity(),
			Static ? JPH::EMotionType::Static : JPH::EMotionType::Dynamic,
			Static ? Layers::NON_MOVING : Layers::MOVING);

		body_settings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
		body_settings.mMassPropertiesOverride.mMass = Mass;
		body_settings.mFriction = 0.5f;

		// Allocate and attach collision properties to the body via the user data field:
		BodyData* properties = new BodyData{ group, mask, false, owner };
		body_settings.mUserData = reinterpret_cast<uintptr_t>(properties);

		JPH::Body* dynamic_body = bodyInterface->CreateBody(body_settings);
		AddBody(dynamic_body);

		return dynamic_body;
	}

	static RefConst<Shape> CreateBoxShape(vec3 Size)
	{
		// Convert & halve once, up-front:
		Vec3 halfExtent = ToPhysics(Size) * 0.5f;

		// Sanity check:
		if (halfExtent.GetX() <= 0 || halfExtent.GetY() <= 0 || halfExtent.GetZ() <= 0)
		{
			printf("CreateBoxShape: invalid half-extent (%f, %f, %f)\n",
				halfExtent.GetX(), halfExtent.GetY(), halfExtent.GetZ());
			return nullptr;
		}

		// Use the constructor that takes half extents; embed for efficiency
		JPH::BoxShapeSettings boxSettings(halfExtent);
		boxSettings.SetEmbedded();

		// Try to create and catch any errors
		auto result = boxSettings.Create();
		if (result.HasError())
		{
			printf("Error creating box shape: %s\n", result.GetError().c_str());
			return nullptr;
		}

		return result.Get();
	}

	static Body* CreateHitBoxBody(Entity* owner, SkeletalMesh* mesh, string hitboxName,
		vec3 PositionOffset,
		quat RotationOffset,   // now a quaternion
		vec3 Size,
		float Mass = 10.0f,
		BodyType group = BodyType::HitBox,
		BodyType mask = BodyType::None);

	static TwoBodyConstraint* CreateRagdollConstraint(Body* parent,
		Body* child,
		float    twistMinAngle,
		float    twistMaxAngle,
		float    swingHalfConeAngle,
		JPH::QuatArg childSpaceConstraintRotation);

	// Enable & configure swing/twist position motors (call once when creating the ragdoll joint)
	static void ConfigureSwingTwistMotor(TwoBodyConstraint* constraint,
		float frequencyHz = 10.0f,
		float damping = 2.0f,
		float max_torque = 100);

	// Update the motor target every frame (childTransformRelParent = child transform in parent space)
	static void UpdateSwingTwistMotor(TwoBodyConstraint* constraint, const quat& childTransformRelParent, const float& strength);

	static uint32_t FindSurfaceId(string surfaceName);
	static string FindSurfacyById(uint32_t id);


	static void Deactivate(const Body* body)
	{

		bodyInterface->DeactivateBody(body->GetID());
	}

	static void Activate(const Body* body)
	{

		bodyInterface->ActivateBody(body->GetID());
	}

	static void SetGravityFactor(const Body* body, float factor)
	{
		bodyInterface->SetGravityFactor(body->GetID(), factor);
	}

	static void SetLinearVelocity(Body* body, vec3 velocity)
	{
		bodyInterface->SetLinearVelocity(body->GetID(), Vec3(velocity.x, velocity.y, velocity.z));
		body->SetLinearVelocity(ToPhysics(velocity));
	}

	static void SetAngularVelocity(Body* body, vec3 velocity)
	{
		bodyInterface->SetAngularVelocity(body->GetID(), ToPhysics(velocity));
		body->SetAngularVelocity(ToPhysics(velocity));
	}

	static void SetBodyCCDEnabled(const Body* body, bool enabled)
	{
		bodyInterface->SetMotionQuality(body->GetID(), enabled ? EMotionQuality::LinearCast : EMotionQuality::Discrete);
	}

	static void AddImpulse(const Body* body, vec3 impulse);
	static void AddImpulseAtLocation(const Body* body, vec3 impulse, vec3 point);

	/**
 * Creates a MeshShape for a static mesh in Jolt Physics.
 * @param vertices A vector of 3D vertex positions (vec3).
 * @param indices A vector of indices defining triangles (3 indices per triangle).
 * @return A RefConst<Shape> containing the created shape, or an empty RefConst if creation fails.
 */
	static RefConst<Shape> CreateMeshShape(const std::vector<vec3>& vertices, const std::vector<uint32_t>& indices, const std::vector<std::string>& materials);



	static RefConst<Shape> CreateConvexHullFromPoints(const std::vector<glm::vec3>& points, std::string surfaceName);

	static RefConst<Shape> CreateStaticCompoundShapeFromConvexShapes(const std::vector<RefConst<Shape>>& convexShapes)
	{
		// Create settings for a static compound shape
		StaticCompoundShapeSettings compoundSettings;

		// Add each convex shape to the compound shape at the origin
		for (const auto& shape : convexShapes)
		{
			compoundSettings.AddShape(Vec3::sZero(), Quat::sIdentity(), shape.GetPtr());
		}

		// Create the compound shape from the settings
		Shape::ShapeResult result = compoundSettings.Create();

		// Check for errors during shape creation
		if (result.HasError())
		{
			printf("Error creating compound shape: %s\n", result.GetError().c_str());
			return RefConst<Shape>(); // Return an empty shape reference on error
		}

		// Return the successfully created compound shape
		return result.Get();
	}


	// Create a body from the provided shape
	static JPH::Body* CreateBodyFromShape(Entity* owner, vec3 Position, RefConst<Shape> shape, float Mass = 10, JPH::EMotionType motionType = JPH::EMotionType::Dynamic,
		BodyType group = BodyType::MainBody,
		BodyType mask = BodyType::GroupCollisionTest)
	{
		// Choose a collision layer based on dynamic or static:
		JPH::BodyCreationSettings body_settings(
			shape.GetPtr(),                        // Pass the shape's raw pointer
			ToPhysics(Position),                   // Convert position to physics coordinates
			JPH::Quat::sIdentity(),                // Default orientation (no rotation)
			motionType,  
			motionType == JPH::EMotionType::Static ? Layers::NON_MOVING : Layers::MOVING);                  // Collision layer

		// Set mass properties
		body_settings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
		body_settings.mMassPropertiesOverride.mMass = Mass;

		if (motionType == JPH::EMotionType::Kinematic)
		{
			body_settings.mCollideKinematicVsNonDynamic = true;   // <— Important
			body_settings.mMotionQuality = JPH::EMotionQuality::LinearCast; // optional CCD
		}

		// Set friction
		body_settings.mFriction = 0.5f;

		// Allocate and attach collision properties to the body via the user data field:
		BodyData* properties = new BodyData{ group, mask,false, owner };
		body_settings.mUserData = reinterpret_cast<uintptr_t>(properties);

		// Create the body and add it to the physics system
		JPH::Body* body = bodyInterface->CreateBody(body_settings);
		AddBody(body);

		return body;
	}


	static Body* CreateCharacterBody(Entity* owner, vec3 Position, float Radius, float Height, float Mass,
		BodyType group = BodyType::CharacterCapsule,
		BodyType mask = BodyType::GroupCollisionTest | BodyType::WorldSkybox);

	static Body* CreateCharacterCylinderBody(Entity* owner, vec3 Position, float Radius, float Height, float Mass,
		BodyType group = BodyType::CharacterCapsule,
		BodyType mask = BodyType::GroupCollisionTest | BodyType::WorldSkybox);


	struct HitResult
	{
		bool hasHit = false;
		vec3 position = {};   // World space hit position.
		vec3 shapePosition = {};   // World space hit position.
		vec3 normal = {};     // World space hit normal.
		const JPH::Body* hitbody = nullptr; // ID of the hit body.
		float fraction = 1;  // Fraction along the ray where the hit occurred.
		Entity* entity = nullptr;
		string hitboxName = "";
		string surfaceName = "";
	};

	static vector<Physics::HitResult> PointTrace(
		const vec3 point,
		const BodyType mask,
		const vector<Body*> ignoreList = {},
		const vector<Entity*> entityIgnoreList = {});

	static HitResult LineTrace(const vec3 start, const vec3 end, const BodyType mask = BodyType::GroupHitTest, const vector<Body*> ignoreList = {}, const vector<Entity*> entityIgnoreList = {});

	class ClosestHitShapeCastCollector : public JPH::CollisionCollector<JPH::ShapeCastResult, JPH::CollisionCollectorTraitsCastShape>
	{
	public:
		ClosestHitShapeCastCollector() : mHadHit(false), mClosestFraction(1.0f) {}

		virtual void AddHit(const JPH::ShapeCastResult& inHit) override
		{
			if (inHit.mFraction < mClosestFraction)
			{
				mClosestFraction = inHit.mFraction;
				mHit = inHit;
				mHadHit = true;
			}
		}

		bool HadHit() const { return mHadHit; }
		const JPH::ShapeCastResult& GetHit() const { return mHit; }

	private:
		bool mHadHit;
		float mClosestFraction;
		JPH::ShapeCastResult mHit;
	};
	
	static HitResult SphereTrace(const vec3 start, const vec3 end, float radius, const BodyType mask = BodyType::GroupHitTest, const vector<Body*> ignoreList = {}, const vector<Entity*> entityIgnoreList = {});
	static std::vector<HitResult> MultiLineTrace(const vec3 start, const vec3 end, const BodyType mask, const vector<Body*> ignoreList, const vector<Entity*> entityIgnoreList);
	static std::vector<HitResult> MultiSphereTrace(const vec3 start, const vec3 end, float radius, const BodyType mask, const vector<Body*> ignoreList, const vector<Entity*> entityIgnoreList);
	static std::vector<Physics::HitResult> MultiSphereOverlap(const vec3 center, float radius, const BodyType mask, const vector<Body*> ignoreList, const vector<Entity*> entityIgnoreList);
	static HitResult SphereTraceForEntity(vector<Entity*> entityties, const vec3 start, const vec3 end, float radius, const BodyType mask = BodyType::GroupHitTest, const vector<Body*> ignoreList = {});

	static HitResult CylinderTrace(const vec3 start, const vec3 end, float radius, float halfHeight, const BodyType mask = BodyType::GroupHitTest, const vector<Body*> ignoreList = {}, const vector<Entity*> entityIgnoreList = {});
	static HitResult ShapeTrace(const Shape* shape,vec3 start, vec3 end, vec3 scale, const BodyType mask = BodyType::GroupHitTest, const vector<Body*> ignoreList = {}, const vector<Entity*> entityIgnoreList = {});

};

namespace std {
	template<>
	struct hash<Physics::PendingBodyEnterPair>
	{
		size_t operator()(const Physics::PendingBodyEnterPair& pair) const noexcept
		{
			return hash<Entity*>()(pair.target) ^ (hash<Entity*>()(pair.entity) << 1);
		}
	};
}