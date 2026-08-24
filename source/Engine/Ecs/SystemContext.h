#pragma once
// SystemContext.h
//
// Everything a system's Execute() gets to touch. Deliberately the entire API
// surface - no raw event buffer, no thread index, no direct dispatcher
// access, and no entt vocabulary required for the common cases:
//
//   ctx.ForEachEntity<AttackIntent>([&](Entity* self, AttackIntent& intent) {
//       ctx.Emit<DamageEvent>({ intent.Target, intent.Damage });
//   });
//
//   if (auto health = ctx.TryGetComponent<Health>(target))
//       health->Current -= amount;
//
//   ctx.Destroy(someEntity); // defers to the entity's own real Destroy()
//
// Registry is still exposed directly as an escape hatch for anything the
// helpers above don't cover.
//
// ForEach<Components...>() vs ForEachEntity<Components...>(): use ForEach
// for pure data-crunching that never needs to reference "the entity" as a
// thing (e.g. bulk stat regen). Use ForEachEntity when you need to act on
// or refer to the entity itself - spawn something relative to it, remove a
// component from it, put it in an event. ForEachEntity silently skips any
// row with no owning Entity (see EcsWorld::GetEntity).

#include <Ecs/ParallelEventBus.h>
#include <Ecs/CommandBuffer.h>
#include <Ecs/ComponentRef.h>
#include <Ecs/IThreadPool.h>
#include <Ecs/EcsWorld.h>
#include <Ecs/EntityOwner.h>

// The project's real entity type - adjust this include if it lives at a
// different path in your project's include search path.
#include "Entity.h"

#include <entt/entt.hpp>
#include <cstddef>
#include <functional>
#include <type_traits>
#include <utility>

struct SystemContext
{
    entt::registry&   Registry;
    float             DeltaTime;
    ParallelEventBus& Events;
    CommandBuffer&    Commands;
    IThreadPool*      ThreadPool = nullptr; // may be null - see ParallelFor() below

    // Calls fn(entt::entity, Components&...) for every entity that has all
    // of Components... - no view/each vocabulary required. Same idea as
    // List<T>.ForEach(Action<T>) in C#. If you'd rather not spell out
    // entt::entity, `auto` works fine too: [](auto, Health& h) {...} - you
    // only need the real type if you're going to do something with the id.
    template<typename... Components, typename Fn>
    void ForEach(Fn&& fn) { Registry.view<Components...>().each(std::forward<Fn>(fn)); }

    // Same as ForEach, but resolves the owning Entity first and calls
    // fn(Entity*, Components&...) instead of fn(entt::entity, Components&...).
    // Rows with no owning Entity (nothing spawned through Entity::Spawn())
    // are silently skipped - use ForEach if you deliberately need those too.
    template<typename... Components, typename Fn>
    void ForEachEntity(Fn&& fn)
    {
        Registry.view<Components...>().each([&](entt::entity e, Components&... comps)
        {
            if (Entity* self = EcsWorld::GetEntity(e))
                fn(self, comps...);
        });
    }

    // Recovers the owning Entity from a raw entt::entity id - see
    // EcsWorld::GetEntity().
    Entity* GetEntity(entt::entity e) const { return EcsWorld::GetEntity(e); }

    // Safe read of a possibly-absent or possibly-dead component:
    //   if (auto h = ctx.TryGetComponent<Health>(e)) h->Current -= dmg;
    // Prefer entity->GetComponent<T>() when you already have an Entity*.
    template<typename T>
    ComponentRef<T> TryGetComponent(entt::entity e) const { return ComponentRef<T>(Registry, e); }

    // Safe to call from any thread. Buffered and fanned out to every
    // connected handler at the end of the current tick group.
    template<typename Event>
    void Emit(Event e) { Events.Push(std::move(e)); }

    // Deferred, applied at the end of the current tick group, safe from any
    // thread. For an id backed by a real Entity, this defers a call to the
    // entity's own Destroy() - physics, owner notification, and actual
    // Level removal, not just the ECS-side row. For an id with no owning
    // Entity (e.g. one from ComponentRegistry::Clone), it just clears the
    // row directly.
    void Destroy(entt::entity e)
    {
        if (Entity* owner = EcsWorld::GetEntity(e))
            Destroy(owner);
        else
            Commands.Destroy(e);
    }
    void Destroy(Entity* entity) { Commands.Enqueue([entity](entt::registry&) { entity->Destroy(); }); }

    template<typename T, typename... Args>
    void AddComponent(entt::entity e, Args&&... args)
    {
        static_assert(!std::is_same_v<T, EntityOwner>,
            "EntityOwner is managed internally by Entity's constructor/destructor - never add or remove it directly.");
        Commands.Emplace<T>(e, std::forward<Args>(args)...);
    }

    template<typename T, typename... Args>
    void AddComponent(Entity* entity, Args&&... args)
    {
        static_assert(!std::is_same_v<T, EntityOwner>,
            "EntityOwner is managed internally by Entity's constructor/destructor - never add or remove it directly.");
        Commands.Emplace<T>(entity->Handle(), std::forward<Args>(args)...);
    }

    template<typename T>
    void RemoveComponent(entt::entity e)
    {
        static_assert(!std::is_same_v<T, EntityOwner>,
            "EntityOwner is managed internally by Entity's constructor/destructor - never add or remove it directly.");
        Commands.Remove<T>(e);
    }

    template<typename T>
    void RemoveComponent(Entity* entity)
    {
        static_assert(!std::is_same_v<T, EntityOwner>,
            "EntityOwner is managed internally by Entity's constructor/destructor - never add or remove it directly.");
        Commands.Remove<T>(entity->Handle());
    }

    // Escape hatch for anything not covered above - e.g. spawning a new
    // entity. Accepts either a zero-arg lambda or one taking entt::registry&
    // - use whichever you actually need; the common case (spawning) doesn't
    // touch the registry directly at all, so it shouldn't have to name it.
    template<typename Fn>
    void Defer(Fn&& fn)
    {
        if constexpr (std::is_invocable_v<Fn, entt::registry&>)
            Commands.Enqueue(std::forward<Fn>(fn));
        else
            Commands.Enqueue([fn = std::forward<Fn>(fn)](entt::registry&) mutable { fn(); });
    }

    // Same idea as System.Threading.Tasks.Parallel.For in C#: splits count
    // independent jobs across the engine thread pool if one is configured,
    // otherwise runs them sequentially on the calling thread.
    //
    // Don't call this from a system that's also batched via
    // RunsConcurrentlyWith with another system - that nests a ParallelFor
    // call inside a worker thread of the outer batch's own ParallelFor,
    // which most thread pool implementations aren't reentrant for.
    void ParallelFor(size_t count, const std::function<void(size_t)>& fn) const
    {
        if (ThreadPool) ThreadPool->ParallelFor(count, fn);
        else for (size_t i = 0; i < count; ++i) fn(i);
    }
};
