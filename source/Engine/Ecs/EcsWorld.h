#pragma once
// EcsWorld.h
//
// Static, no-instance access to the single registry backing every entity in
// the game. Every call is EcsWorld::Something(...) - there's no instance to
// fetch and no ::Get() anywhere.
//
// Naming mirrors Unity's GameObject/Component API on purpose - that's the
// vocabulary most C# gameplay programmers already have loaded:
//   TryGetComponent<T>(entity)   - returns something bool-checkable instead
//                                  of using an out parameter (C++ has none)
//   AddComponent<T>(entity, ...)
//   HasComponent<T>(entity)
//
// GetEntity() is the important one for this project specifically: it turns
// a bare entt::entity id (from a view, an event, wherever) back into the
// real, owning engine Entity - the only thing with actual identity
// (rendering, lifecycle). See EntityOwner.h.
//
// IMPORTANT: every mutating method here (AddComponent/RemoveComponent/
// Destroy) is IMMEDIATE, not deferred - safe to call from single-threaded
// contexts (event handlers, Defer callbacks, a non-parallel system's own
// code) but NOT safe to call concurrently, e.g. from inside a ParallelSafe()
// system's own ctx.ParallelFor. Inside a system, prefer ctx's versions of
// these same names - they're deferred to the tick group's flush point and
// safe from any thread.

#include <Ecs/ComponentRef.h>
#include <Ecs/EntityOwner.h>

#include <entt/entt.hpp>
#include <type_traits>
#include <utility>

class EcsWorld
{
public:
    EcsWorld() = delete;

    static entt::registry& Registry() { return Storage(); }

    static entt::entity Create() { return Storage().create(); }

    static bool IsValid(entt::entity e) { return Storage().valid(e); }

    // Destroys an id, doing the right thing based on what it actually is:
    // for one backed by a real Entity, calls the entity's own Destroy()
    // (physics, owner notification, and actual Level removal - not just
    // the ECS-side row). For one with no owning Entity (e.g. one from
    // ComponentRegistry::Clone), just clears the row directly.
    //
    // Immediate, not deferred - see the note at the top of this file.
    // Inside a system, prefer ctx.Destroy(), which defers this to the tick
    // group's flush point and is safe from any thread.
    static void Destroy(entt::entity e);

    // Recovers the owning engine Entity from a raw entt::entity id - the
    // preferred way to turn an id from iteration (ctx.ForEach, an event's
    // Target, etc.) back into something with real identity.
    //
    // Returns nullptr if e is invalid, was never a real Entity (e.g. it came
    // from ComponentRegistry::Clone rather than Entity::Spawn()), or has
    // already had Destroy() called on it.
    static Entity* GetEntity(entt::entity e)
    {
        auto owner = TryGetComponent<EntityOwner>(e);
        return owner ? owner->Owner : nullptr;
    }

    // Safe even if e is invalid or lacks T - check with `if (auto h = ...)`
    // before dereferencing, same pattern as Unity's TryGetComponent.
    template<typename T>
    static ComponentRef<T> TryGetComponent(entt::entity e) { return ComponentRef<T>(Storage(), e); }

    // Note: this is an upsert (replaces the component if one already
    // exists), unlike Unity's AddComponent, which errors on a duplicate for
    // components that don't allow multiples. Check HasComponent<T>() first
    // if you need strict add-once semantics.
    //
    // Immediate, not deferred - see the note at the top of this file.
    template<typename T, typename... Args>
    static T& AddComponent(entt::entity e, Args&&... args)
    {
        static_assert(!std::is_same_v<T, EntityOwner>,
            "EntityOwner is managed internally by Entity's constructor/destructor - never add or remove it directly.");
        return Storage().emplace_or_replace<T>(e, std::forward<Args>(args)...);
    }

    template<typename T>
    static bool HasComponent(entt::entity e) { return Storage().all_of<T>(e); }

    // Immediate, not deferred - see the note at the top of this file.
    template<typename T>
    static void RemoveComponent(entt::entity e)
    {
        static_assert(!std::is_same_v<T, EntityOwner>,
            "EntityOwner is managed internally by Entity's constructor/destructor - never add or remove it directly.");
        Storage().remove<T>(e);
    }

    // Destroys every entity and every component pool - the world comes back
    // completely empty. Called from Level::CloseLevel() after every real
    // Entity has already been torn down through its own destructor; this is
    // a defensive full wipe for anything else that might be left over.
    //
    // NOT safe to call from inside a system's Execute() or an event handler.
    static void Reset() { Storage() = entt::registry{}; }

private:
    static entt::registry& Storage()
    {
        static entt::registry registry;
        return registry;
    }
};
