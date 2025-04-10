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
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/ShapeCast.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Collision/Shape/CompoundShape.h>
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Core/Reference.h>

#include "PhysicsConverter.h"

#include "glm.h"

#include <iostream>

#include "Time.hpp"

#include "Logger.hpp"

#include "MathHelper.hpp"

using namespace JPH;

using namespace std;

enum class BodyType : uint32_t {
	None = 1,       // 1
	MainBody = 2,       // 2
	HitBox = 4,       // 4
	WorldOpaque = 8,       // 8
	CharacterCapsule = 16,      // 16
	NoRayTest = 32,      // 32
	Liquid = 64,      // 64
	WorldTransparent = 128,     // 128

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

struct BodyData 
{
	BodyType group;
	BodyType mask;

	Entity* OwnerEntity;

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
	// See: ContactListener
	virtual ValidateResult OnContactValidate(const Body& inBody1, const Body& inBody2,
		RVec3Arg inBaseOffset,
		const CollideShapeResult& inCollisionResult) override
	{
		auto* props1 = reinterpret_cast<BodyData*>(inBody1.GetUserData());
		auto* props2 = reinterpret_cast<BodyData*>(inBody2.GetUserData());

		if (props1 && props2) {
			// Only allow collision if both:
			// 1. The first body's group is contained in the second body's mask.
			// 2. The second body's group is contained in the first body's mask.
			bool collide1 = (static_cast<uint32_t>(props1->group) & static_cast<uint32_t>(props2->mask)) != 0;
			bool collide2 = (static_cast<uint32_t>(props2->group) & static_cast<uint32_t>(props1->mask)) != 0;

			if (!collide1 || !collide2)
				return ValidateResult::RejectContact;
		}

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

class TraceBodyFilter : public BodyFilter
{
public:
	// List of bodies to ignore during the ray cast.
	std::vector<Body*> ignoreList = {};
	// Collision mask to filter out bodies that don't belong to the desired groups.
	BodyType mask = BodyType::GroupHitTest;

	TraceBodyFilter() = default;
	virtual ~TraceBodyFilter() = default;

	/// Filter function when the Body is locked.
	virtual bool ShouldCollideLocked(const Body& inBody) const override
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

		// Accept the collision if no condition rejects it.
		return true;
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

	//static mutex physicsMainLock;

public:
	
	static void AddBody(Body* body)
	{
		//physicsMainLock.lock();

		existingBodies.push_back(body);

		bodyInterface->AddBody(body->GetID(), JPH::EActivation::Activate);

		//physicsMainLock.unlock();

	}

	static void DestroyBody(Body* body)
	{


		//physicsMainLock.lock();

		bodyInterface->RemoveBody(body->GetID());

		// Retrieve and delete collision properties if present.
		auto* props = reinterpret_cast<BodyData*>(body->GetUserData());
		if (props)
			delete props;

		bodyInterface->DestroyBody(body->GetID());
		//physicsMainLock.unlock();
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
		//physicsMainLock.lock();
		physics_system->Update(Time::DeltaTime, 1, tempMemAllocator, threadPool);
		//physicsMainLock.unlock();
	}

	static void Update()
	{

	}

	static Body* CreateBoxBody(Entity* owner, vec3 Position, vec3 Size, float Mass = 10, bool Static = false,
		BodyType group = BodyType::MainBody,
		BodyType mask = BodyType::GroupCollisionTest)
	{
		// Create a box shape (existing code)
		auto box_shape_settings = new JPH::BoxShapeSettings();
		box_shape_settings->SetEmbedded();
		box_shape_settings->mHalfExtent = ToPhysics(Size) / 2.0f;
		JPH::Shape::ShapeResult shape_result = box_shape_settings->Create();
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
		BodyData* properties = new BodyData{ group, mask, owner };
		body_settings.mUserData = reinterpret_cast<uintptr_t>(properties);

		JPH::Body* dynamic_body = bodyInterface->CreateBody(body_settings);
		AddBody(dynamic_body);

		return dynamic_body;
	}


	static void Activate(Body* body)
	{

		bodyInterface->ActivateBody(body->GetID());
	}

	static void SetGravityFactor(Body* body, float factor)
	{
		bodyInterface->SetGravityFactor(body->GetID(), factor);
	}

	/**
 * Creates a MeshShape for a static mesh in Jolt Physics.
 * @param vertices A vector of 3D vertex positions (vec3).
 * @param indices A vector of indices defining triangles (3 indices per triangle).
 * @return A RefConst<Shape> containing the created shape, or an empty RefConst if creation fails.
 */
	static RefConst<Shape> CreateMeshShape(const std::vector<vec3>& vertices, const std::vector<uint32_t>& indices)
	{
		// Validate that the number of indices is a multiple of 3 (required for triangles)
		if (indices.size() % 3 != 0)
		{
			printf("Error: Number of indices (%zu) must be a multiple of 3 to form complete triangles.\n", indices.size());
			return RefConst<Shape>(); // Return empty shape on error
		}

		// Convert vertices to Jolt's Float3 format
		Array<Float3> joltVertices;
		joltVertices.reserve(vertices.size()); // Reserve space to avoid reallocations
		for (const auto& v : vertices)
		{
			joltVertices.push_back(Float3(v.x, v.y, v.z));
		}

		// Convert indices to Jolt's IndexedTriangle format
		Array<IndexedTriangle> joltTriangles;
		joltTriangles.reserve(indices.size() / 3); // Reserve space for the number of triangles
		for (size_t i = 0; i < indices.size(); i += 3)
		{
			IndexedTriangle tri;
			tri.mIdx[0] = indices[i];     // First vertex index of the triangle
			tri.mIdx[1] = indices[i + 1]; // Second vertex index
			tri.mIdx[2] = indices[i + 2]; // Third vertex index
			joltTriangles.push_back(tri);
		}

		// Create MeshShapeSettings with the converted data
		MeshShapeSettings shapeSettings(joltVertices, joltTriangles);

		// Attempt to create the shape
		Shape::ShapeResult result = shapeSettings.Create();
		if (result.HasError())
		{
			printf("Error creating mesh shape: %s\n", result.GetError().c_str());
			return RefConst<Shape>(); // Return empty shape on error
		}

		// Return the successfully created shape
		return result.Get();
	}

	RefConst<Shape> CreateConvexHullFromPoints(const std::vector<Vec3>& points)
	{
		// Convert std::vector<Vec3> to Array<Vec3> (Jolt's format)
		Array<Vec3> hullPoints;
		for (const Vec3& pt : points)
		{
			hullPoints.push_back(pt);
		}

		// Settings for the convex hull shape
		ConvexHullShapeSettings shapeSettings(hullPoints);

		// Optional: check for errors
		Shape::ShapeResult result = shapeSettings.Create();
		if (result.HasError())
		{
			printf("Error creating convex hull shape: %s\n", result.GetError().c_str());
			return;
		}

		// Successfully created shape
		RefConst<Shape> shape = result.Get();
	}

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
	static JPH::Body* CreateBodyFromShape(Entity* owner, vec3 Position, RefConst<Shape> shape, float Mass = 10, bool Static = false,
		BodyType group = BodyType::MainBody,
		BodyType mask = BodyType::GroupCollisionTest)
	{
		// Choose a collision layer based on dynamic or static:
		JPH::BodyCreationSettings body_settings(
			shape.GetPtr(),                        // Pass the shape's raw pointer
			ToPhysics(Position),                   // Convert position to physics coordinates
			JPH::Quat::sIdentity(),                // Default orientation (no rotation)
			Static ? JPH::EMotionType::Static : JPH::EMotionType::Dynamic,  // Motion type based on Static flag
			Static ? Layers::NON_MOVING : Layers::MOVING);                  // Collision layer

		// Set mass properties
		body_settings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
		body_settings.mMassPropertiesOverride.mMass = Mass;

		// Set friction
		body_settings.mFriction = 0.5f;

		// Allocate and attach collision properties to the body via the user data field:
		BodyData* properties = new BodyData{ group, mask, owner };
		body_settings.mUserData = reinterpret_cast<uintptr_t>(properties);

		// Create the body and add it to the physics system
		JPH::Body* body = bodyInterface->CreateBody(body_settings);
		AddBody(body);

		return body;
	}

	struct HitResult
	{
		bool hasHit;
		vec3 position;   // World space hit position.
		vec3 shapePosition;   // World space hit position.
		vec3 normal;     // World space hit normal.
		const JPH::Body* hitbody; // ID of the hit body.
		float fraction;  // Fraction along the ray where the hit occurred.
	};

	static HitResult LineTrace(const vec3 start, const vec3 end, const BodyType mask = BodyType::GroupHitTest, const vector<Body*> ignoreList = {})
	{
		HitResult hit;

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

		//physicsMainLock.lock();

		// Cast the ray using the narrow phase query.
		bool hasHit = physics_system->GetNarrowPhaseQuery().CastRay(ray, result, {}, {}, filter );

		if (hasHit)
		{
			// Calculate fraction of the hit along the ray.
			hit.fraction = result.mFraction;

			hit.shapePosition = mix(start, end, hit.fraction);

			// Compute the hit position in physics space and convert it back to your coordinate system.
			hit.position = FromPhysics(ray.GetPointOnRay(result.mFraction));

			// Lock the body using the BodyLockInterface to safely access it.
			JPH::BodyLockRead body_lock(physics_system->GetBodyLockInterface(), result.mBodyID);

			// Retrieve the hit body's surface normal.
			// The method GetWorldSpaceSurfaceNormal requires the sub-shape ID and the hit position.
			const JPH::Body* body = &body_lock.GetBody();
			if (body)
			{
				hit.normal = FromPhysics(body->GetWorldSpaceSurfaceNormal(result.mSubShapeID2, ray.GetPointOnRay(result.mFraction)));
			}
			else
			{
				hit.normal = vec3(0, 0, 0);
			}

			// Record the hit body and shape.
			hit.hitbody = body;
		}
		else
		{
			// No hit: return the end point, a fraction of 1.0 and zero normal.
			hit.fraction = 1.0f;
			hit.position = end;
			hit.normal = vec3(0, 0, 0);
			hit.hitbody = nullptr; // Invalid body ID.
		}

		//physicsMainLock.unlock();

		hit.hasHit = hasHit;

		return hit;
	}

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
	
	static HitResult SphereTrace(const vec3 start, const vec3 end, float radius, const BodyType mask = BodyType::GroupHitTest, const vector<Body*> ignoreList = {})
	{
		HitResult hit;

		// Convert start and end from your own vector type to Jolt's coordinate system.
		JPH::Vec3 startLoc = ToPhysics(start);
		JPH::Vec3 endLoc = ToPhysics(end);

		// Create a sphere shape for the trace.
		auto sphere_shape_settings = new JPH::SphereShapeSettings();
		sphere_shape_settings->SetEmbedded();
		sphere_shape_settings->mRadius = radius;
		JPH::Shape::ShapeResult shape_result = sphere_shape_settings->Create();
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
			delete sphere_shape_settings;
			return hit;
		}

		// Set up the shape cast from startLoc to endLoc.
		JPH::Vec3 direction = endLoc - startLoc;
		JPH::RMat44 start_transform = JPH::RMat44::sTranslation(startLoc);
		JPH::RShapeCast shape_cast(sphere_shape, JPH::Vec3::sReplicate(1.0f), start_transform, direction);



		TraceBodyFilter filter;
		filter.mask = mask;
		filter.ignoreList = ignoreList;

		//physicsMainLock.lock();

		// Cast the shape using the narrow phase query.
		ClosestHitShapeCastCollector collector;
		physics_system->GetNarrowPhaseQuery().CastShape(shape_cast, JPH::ShapeCastSettings(), JPH::Vec3::sZero(), collector, {}, {}, filter);
		if (collector.HadHit())
		{
			// Calculate fraction of the hit along the path.
			hit.fraction = collector.GetHit().mFraction;

			hit.shapePosition = mix(start, end, hit.fraction);

			// Compute the hit position in physics space (point on the hit body) and convert it back to your coordinate system.
			hit.position = FromPhysics(collector.GetHit().mContactPointOn2);

			// Lock the body using the BodyLockInterface to safely access it.
			JPH::BodyLockRead body_lock(physics_system->GetBodyLockInterface(), collector.GetHit().mBodyID2);

			// Retrieve the hit body's surface normal.
			const JPH::Body* body = &body_lock.GetBody();
			if (body)
			{
				hit.normal = FromPhysics(body->GetWorldSpaceSurfaceNormal(collector.GetHit().mSubShapeID2, collector.GetHit().mContactPointOn2));
			}
			else
			{
				hit.normal = vec3(0, 0, 0);
			}

			// Record the hit body.
			hit.hitbody = body;
			hit.hasHit = true;
		}
		else
		{
			// No hit: return the end point, a fraction of 1.0 and zero normal.
			hit.fraction = 1.0f;
			hit.position = end;
			hit.normal = vec3(0, 0, 0);
			hit.hitbody = nullptr;
			hit.hasHit = false;
		}

		//physicsMainLock.unlock();

		// Clean up the shape settings.
		delete sphere_shape_settings;

		return hit;
	}


};
