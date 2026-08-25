#pragma once
// Ecs.h
//
// Single include for the ECS layer's REUSABLE pieces. The Entity class
// itself is NOT here anymore - it's your project's own Entity.h, with entt
// wiring mixed directly into it (see EcsWorld.h, EntityOwner.h, and the
// ECS_COMPONENT macro defined at the bottom of your Entity.h). This header
// no longer includes an Entity.h of its own.
//
// Every cross-file include in this folder uses <Ecs/Header.h> - this
// assumes Ecs/ sits at your project's source root, i.e. that root is on
// your include search path. If it lives elsewhere, add that directory to
// your include paths rather than editing the includes themselves.
//
// Every class here is a static class - EcsWorld::Something(...),
// ComponentRegistry::Something(...), EcsScheduler::Something(...) - there's
// no instance to fetch and no ::Get() anywhere.
//
// Entity, not entt::entity, is the primary handle for "a game thing" -
// every real Entity has full engine lifecycle and rendering, a bare
// entt::entity is just an internal storage-row id. Reference entities in
// your own gameplay data (event fields, component fields) as `Entity*`, not
// `entt::entity`. Use ctx.ForEachEntity<Components...>() when you need to
// act on the entity itself; ctx.ForEach<Components...>() remains available
// for pure data-crunching that never needs "the entity" as an object.
// Spawn a new game-visible entity through Entity::Spawn("TechnicalName"),
// never by calling EcsWorld::Registry().create() or ComponentRegistry::
// Clone() directly - those only produce a bare ECS row with no rendering or
// lifecycle. Destroy one through its own entity->Destroy(), never through
// ctx.Destroy(entt::entity) - Destroy() also handles physics, owner
// notification, and actual removal from the level.
//
// ---------------------------------------------------------------------------
// WORKFLOWS - the entire day-to-day API surface for a gameplay programmer
// ---------------------------------------------------------------------------
//
// COMPONENT
//   struct AttackCooldown { float Remaining = 0.f; };
//   REGISTER_COMPONENT(AttackCooldown)
//
//   class Goblin : public Entity
//   {
//       ECS_COMPONENT(AttackCooldown, AttackCooldown);
//   public:
//       void OnHit() { AddComponent<Stunned>(1.5f); }  // generic AddComponent<T>() for dynamic components
//   };
//   // -> goblin->GetAttackCooldown(), goblin->HasAttackCooldown(), shows up
//   //    in Components(). Position/Health/etc. on Entity are NOT migrated -
//   //    use ECS_COMPONENT for new data, not to duplicate existing fields.
//
// SYSTEM
//   DECLARE_SYSTEM(CombatSystem, PostUpdate)
//   {
//       ctx.ForEachEntity<AttackIntent>([&](Entity* self, AttackIntent& intent)
//       {
//           ctx.Emit<DamageEvent>({ intent.Target, intent.Damage });
//           ctx.RemoveComponent<AttackIntent>(self);
//       });
//   }
//   REGISTER_SYSTEM(CombatSystem, .After({ "AI" }))
//
//   // If a system's ENTIRE body is one ForEach/ForEachEntity loop over a
//   // single component, TICK_COMPONENT/TICK_ENTITY + REGISTER_TICK skip the
//   // loop syntax entirely - see ISystem.h for the full pattern:
//   //   TICK_ENTITY(CombatSystem, PostUpdate, AttackIntent, self, intent)
//   //   {
//   //       ctx.Emit<DamageEvent>({ intent.Target, intent.Damage });
//   //       ctx.RemoveComponent<AttackIntent>(self);
//   //   }
//   //   REGISTER_TICK(CombatSystem, .After({ "AI" }))
//
// EVENT
//   struct DamageEvent { Entity* Target = nullptr; float Amount = 0.f; };
//   REGISTER_EVENT(DamageEvent)   // optional - only affects DumpAll() naming
//
//   EVENT_HANDLER(ApplyDamage, DamageEvent)
//   {
//       if (e.Target) e.Target->Health -= e.Amount;   // real field, not a component
//   }
//
//   // emitting is the SAME call from any system, no setup required:
//   //   ctx.Emit<DamageEvent>({ someEntity, 25.f });
//   // emitting from OUTSIDE a system (game code, an Entity method) uses
//   // EcsScheduler::Emit<T>() instead - same buffering, callable anywhere:
//   //   EcsScheduler::Emit<DamageEvent>({ someEntity, 25.f });
//
// One-time engine init (see Ecs/Integration/ENGINE_WIRING.md):
//   EcsScheduler::SetThreadPool(&g_LevelThreadPoolAdapter);
//   EcsScheduler::Finalize();          // after every static registrar has run
//   LogInfo(EcsScheduler::DumpAll());  // optional, prints order + event bindings
//
// Level unload is already wired: Level::CloseLevel() calls EcsWorld::Reset()
// once every entity has been torn down.
// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
// SHARED LIBRARIES: prefer manual registration
// ---------------------------------------------------------------------------
//
// Every REGISTER_* / EVENT_HANDLER macro above registers itself via a
// static object, run at static-initialization time. That's reliable in a
// single executable or a static library linked in wholesale, but NOT
// reliable once this code lives in a shared library (a DLL/.so - engine
// code separate from game code, or a hot-reloadable game module):
//
//   - Linkers commonly strip an object file's static initializer out of a
//     static library entirely if nothing else in the link references that
//     file - the .cpp compiles and links fine, and its REGISTER_* call
//     simply never runs, with no error anywhere.
//   - A Meyer's singleton like ComponentRegistry's internal storage can end
//     up duplicated - one copy per DLL - if the library boundary isn't set
//     up to share it, so something registered from one DLL is invisible to
//     code in another.
//   - Static init order ACROSS DLLs depends on OS loader behavior, which is
//     even less predictable than within one binary - a late-loaded or
//     hot-reloaded module's registrars may not have run yet by the time
//     EcsScheduler::Finalize() executes.
//
// (Every REGISTER_* macro IS already safe to place directly in a shared
// header, in the sense that it won't double-register per translation unit -
// the generated registrar is a C++17 inline variable, deduplicated by the
// linker. That solves the "one header, many .cpp includes" problem; it does
// NOT solve the three DLL-boundary problems above.)
//
// Every REGISTER_* macro has a _MANUAL counterpart that does the identical
// registration call, but as a plain function call instead of a static
// object - safe to call explicitly, in an order you control, from
// somewhere you know for certain runs (e.g. your DLL's own init entry
// point, called before EcsScheduler::Finalize()):
//
//   void RegisterGameplay()
//   {
//       REGISTER_COMPONENT_MANUAL(Health);
//       REGISTER_COMPONENT_MANUAL(AttackCooldown);
//
//       REGISTER_SYSTEM_MANUAL(CombatSystem, .After({ "AI" }));
//
//       AttackCooldownSystem_RegisterManual();   // TICK_COMPONENT/TICK_ENTITY systems
//                                                 // generate a named function instead of
//                                                 // an inline call - see REGISTER_TICK_MANUAL
//
//       REGISTER_EVENT_MANUAL(DamageEvent);
//       ApplyDamage_RegisterManual();            // EVENT_HANDLER_MANUAL handlers, same reason
//   }
//
// ECS_COMPONENT doesn't have (or need) a _MANUAL form - it's not static-init
// based at all. It runs as a normal member initializer at entity
// construction time, every time, regardless of DLL boundaries.
//
// There's no way to auto-discover this list - that's exactly the mechanism
// being avoided. If it's not called here, it's not registered. Cross-check
// against ComponentRegistry::All() / EcsScheduler::AllSystems() /
// EventRegistry::DumpText() if you're not sure something made it in.
// ---------------------------------------------------------------------------

#include <Ecs/TickGroup.h>
#include <Ecs/ComponentRegistry.h>
#include <Ecs/ComponentRef.h>
#include <Ecs/ComponentList.h>
#include <Ecs/EcsWorld.h>
#include <Ecs/EntityOwner.h>
#include <Ecs/ParallelEventBus.h>
#include <Ecs/CommandBuffer.h>
#include <Ecs/IThreadPool.h>
#include <Ecs/SystemContext.h>
#include <Ecs/ISystem.h>
#include <Ecs/EventRegistry.h>
#include <Ecs/EcsScheduler.h>
