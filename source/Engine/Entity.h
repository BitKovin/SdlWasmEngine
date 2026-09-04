#pragma once

#include <vector>

#include "EObject.hpp"
#include "Helpers/JsonHelper.hpp"
#include "Helpers/JsonMacros.hpp"

#include "LevelObject.hpp"

#include "IDrawMesh.h"

#include "glm.h"

#include "Physics.h"

#include "RegisterLevelObject.h"

#include "MapData.h"

#include "Level.hpp"

#include <Ecs/EcsWorld.h>
#include <Ecs/EntityOwner.h>
#include <Ecs/ComponentRef.h>
#include <Ecs/ComponentList.h>

using namespace std;

class Entity : public LevelObject
{
public:

	vec3 Position = vec3();

	vec3 Rotation = vec3();

	vec3 Scale = vec3(1);

	vector<IDrawMesh*> Drawables;

	Body* LeadBody = nullptr;

	vector<Body*> Bodies;

	bool Unique = false;

	bool SaveGame = false;

	string ClassName = "Entity";
	string Id = "";

	string Name = "";

	float Health = 1;
	float MaxHealth = 100;

	bool AssetsLoaded = false;

	bool ConvexCollision = false;

	string OwnerId = "";

	vector<string> Tags;

	uint64_t networkProxyId = 0; //if this entity is represented by another entity in networking (like remote player that follows local one) store network id here. For entity handle.

	Delay destroyDelay = Delay(1000000000);

	BodyType DefaultBrushGroup = BodyType::WorldOpaque;
	BodyType DefaultBrushCollisionMask = BodyType::GroupCollisionTest;

	double SpawnTime = 0;

	vector<Body*> contactList;

	Entity()
	{
		m_Handle = entt::handle(EcsWorld::Registry(), EcsWorld::Create());
		m_Handle.emplace<EntityOwner>(this);
		// ECS_COMPONENT member initializers in subclasses run after this
		// constructor body completes, in declaration order - m_Handle is
		// always valid by the time any of them fire.
	}
	virtual ~Entity() 
	{
		DestroyDrawables();
		if (m_Handle.valid())
			m_Handle.destroy();
	}


	void UpdatePhysics()
	{
		if (LeadBody)
		{
			Position = FromPhysics(LeadBody->GetPosition());
			Rotation = MathHelper::ToYawPitchRoll(FromPhysics(LeadBody->GetRotation()));
		}
	}

	void Start()
	{
		SpawnTime = Time::GameTime;
	}

	virtual void FromData(EntityData data);

	virtual void Finalize()
	{

	}

	virtual LightVolPointData GetLightVolData(bool wallCheck);

	virtual void OnDamage(float Damage, Entity* DamageCauser = nullptr, Entity* Weapon = nullptr)
	{
		Health -= Damage;
	}

	virtual void Teleport(vec3 position);

	virtual void OnPointDamage(float Damage, vec3 Point, vec3 Direction, string bone = "", Entity* DamageCauser = nullptr, Entity* Weapon = nullptr)
	{
		OnDamage(Damage, DamageCauser, Weapon);
	}

	vector<IDrawMesh*> GetDrawMeshes()
	{
		return Drawables;
	}

	void DestroyDrawables()
	{
		for (IDrawMesh* mesh : Drawables)
		{
			delete(mesh);
		}

		Drawables.clear();
	}

	void FinalLevelRemove();

	void UpdateDestroyDelay()
	{
		if (destroyDelay.Wait() == false)
		{
			Destroy();
		}
	}

	virtual void DestroyWithDelay(float delay)
	{
		destroyDelay.AddDelay(delay);
	}

	virtual void Destroy();

	//destroys entity here and now if dones't exists in level list.
	virtual void DestroyOnPlace();

	void Serialize(json& target);

	void Deserialize(json& source);

	void LoadAssetsIfNeeded()
	{
		if (AssetsLoaded == false)
		{
			LoadAssets();
			AssetsLoaded = true;

			if (Level::Current->IsEntityTypeLoaded(ClassName) == false)
			{
				Level::Current->AddLoadedEntityType(ClassName);
			}

		}
	}

	bool HasTag(string tag)
	{

		for (auto t : Tags)
		{
			if (tag == t)
			{
				return true;
			}
		}

		return false;
	}

	virtual void OnBodyEntered(Body* body, Entity* entity) {}
	virtual void OnBodyExited(Body* body, Entity* entity) {}

	virtual void OnAction(string action) {}

	static Entity* Spawn(std::string technicalName, bool imOwner = true);

	static void CallActionOnEveryEntityWithName(std::string name, std::string action)
	{

		if (name == "") return;

		auto entities = Level::Current->FindAllEntitiesWithName(name);

		for (Entity* entity : entities)
		{
			entity->OnAction(action);
		}

	}

	static void CallActionOnEntityWithId(std::string id, std::string action)
	{

		if (id == "") return;

		auto entity = Level::Current->FindEntityWithId(id);

		if (entity != nullptr)
		{
			entity->OnAction(action);
		}



	}

	static void PreloadEntityType(std::string technicalName);
	static void PreloadEntityTypeAsync(std::string technicalName);

	std::string GetId() override
	{
		return Id;
	}

	void DestroyPhysics();

	// --- ECS ---------------------------------------------------------------
	//
	// Generic escape hatch for components that aren't declared with
	// ECS_COMPONENT below - e.g. something added and removed at runtime
	// rather than being part of this entity's default set:
	//   AddComponent<Stunned>(1.5f);
	//   if (HasComponent<Stunned>()) ...
	//   RemoveComponent<Stunned>();
	//
	// Position/Rotation/Scale/Health above are NOT migrated to components -
	// they stay exactly as they are. Use ECS_COMPONENT/AddComponent for new
	// gameplay data only, not to duplicate what this class already tracks.
	template<typename T, typename... Args>
	T& AddComponent(Args&&... args) { return m_Handle.emplace_or_replace<T>(std::forward<Args>(args)...); }

	template<typename T>
	ComponentRef<T> GetComponent() { return ComponentRef<T>(*m_Handle.registry(), m_Handle.entity()); }

	template<typename T>
	bool HasComponent() const { return m_Handle.all_of<T>(); }

	template<typename T>
	void RemoveComponent() { m_Handle.remove<T>(); }

	entt::entity Handle() const { return m_Handle.entity(); }
	const ComponentList& Components() const { return m_Components; }

	void OnLevelRemoved();

protected:

	entt::handle  m_Handle;
	ComponentList m_Components;

	void OnDispose()
	{
		Destroy();
	}

	virtual void LoadAssets()
	{

	}

private:

};

// Type: component type, must already be REGISTER_COMPONENT'd somewhere.
// Name: accessor suffix -> GetName() / HasName(). Reads better than the
// generic GetComponent<T>()/HasComponent<T>() above for a component that's
// a core, always-present part of a specific entity subclass.
// Trailing ... : optional constructor args forwarded to the component's
// constructor (omit for value-initialized defaults).
//
// Requires empty __VA_ARGS__ support (standard in C++20, and already a
// common extension in GCC/Clang/MSVC pre-C++20) since Name-only usage like
// ECS_COMPONENT(AttackCooldown, AttackCooldown) passes zero variadic args.
//
// NOTE: must be invoked at class scope (inside a subclass body), never
// inside a function body.
#define ECS_COMPONENT(Type, Name, ...) \
	public: \
		ComponentRef<Type> Get##Name() { return ComponentRef<Type>(*m_Handle.registry(), m_Handle.entity()); } \
		bool Has##Name() const { return m_Handle.all_of<Type>(); } \
	private: \
		bool m_##Name##_ComponentInit = ( \
			m_Handle.emplace<Type>(__VA_ARGS__), \
			m_Components.Add(entt::type_id<Type>().hash(), #Type), \
			true)