#pragma once
// ISystem.h
//
// Base interface for every ECS system. Don't implement this by hand - use
// DECLARE_SYSTEM(Name, TickGroup) { ... body ... } followed by
// REGISTER_SYSTEM(Name, .After({...}) .Before({...}) ...). See Ecs.h for the
// full workflow summary.
//
// The fluent config methods share their name with the matching getter -
// After() reads, After(v) writes; ParallelSafe() reads, ParallelSafe(v)
// writes - the closest C++ overloading gets to a C# property's get/set pair
// sharing one identifier.
//
// NOTE: DECLARE_SYSTEM / REGISTER_SYSTEM must be invoked at namespace scope,
// never inside a function body.

#include <Ecs/TickGroup.h>

#include <memory>
#include <string>
#include <vector>

struct SystemContext;

#define ECS_STRINGIFY_IMPL(x) #x
#define ECS_STRINGIFY(x) ECS_STRINGIFY_IMPL(x)

class ISystem
{
public:
    virtual ~ISystem() = default;

    virtual void Execute(SystemContext& ctx) = 0;
    virtual const char* Name()  const = 0;
    virtual TickGroup    Group() const = 0;

    // Names of other systems, IN THE SAME TICK GROUP, that must run before /
    // after this one. Both directions are accepted and collapse into the
    // same ordering graph - use whichever reads more naturally at the call
    // site. Cross-group references are rejected: tick groups already run
    // sequentially, so cross-group ordering is implicit.
    virtual std::vector<std::string> After()  const { return {}; }
    virtual std::vector<std::string> Before() const { return {}; }

    // True only if THIS system splits its own view.each() loop across the
    // thread pool internally. Manual and asserted, never auto-derived from
    // component access - a type signature can't see an unrelated entity
    // lookup buried inside the loop.
    virtual bool ParallelSafe() const { return false; }

    // Names of other systems safe to run concurrently with this one as a
    // scheduler-level batch. Must be declared on BOTH sides to actually be
    // batched together; a one-sided declaration that contradicts an
    // After/Before relationship is a hard error at Finalize().
    virtual std::vector<std::string> RunsConcurrentlyWith() const { return {}; }

    virtual bool UpdateWhenPaused() const { return false; }

    // File:line of the DECLARE_SYSTEM call site - auto-populated, not
    // something you set. Shows up in Finalize()'s error messages and in
    // DumpAll()'s "Declared dependencies" section, so a cycle or a
    // contradictory RunsConcurrentlyWith points straight at the line to fix
    // instead of just a system name to grep for.
    virtual const char* SourceLocation() const { return "<unknown>"; }
};

#define DECLARE_SYSTEM(ClassName, Grp) \
class ClassName : public ISystem \
{ \
public: \
    void Execute(SystemContext& ctx) override; \
    const char* Name()  const override { return #ClassName; } \
    TickGroup   Group() const override { return TickGroup::Grp; } \
    const char* SourceLocation() const override { return __FILE__ ":" ECS_STRINGIFY(__LINE__); } \
    std::vector<std::string> After()                 const override { return m_After; } \
    std::vector<std::string> Before()                const override { return m_Before; } \
    std::vector<std::string> RunsConcurrentlyWith()   const override { return m_ConcurrentWith; } \
    bool ParallelSafe()     const override { return m_ParallelSafe; } \
    bool UpdateWhenPaused() const override { return m_UpdateWhenPaused; } \
    ClassName& Self() { return *this; } \
    ClassName& After(std::vector<std::string> v)                { m_After = std::move(v); return *this; } \
    ClassName& Before(std::vector<std::string> v)                { m_Before = std::move(v); return *this; } \
    ClassName& RunsConcurrentlyWith(std::vector<std::string> v)  { m_ConcurrentWith = std::move(v); return *this; } \
    ClassName& ParallelSafe(bool v)      { m_ParallelSafe = v; return *this; } \
    ClassName& UpdateWhenPaused(bool v)  { m_UpdateWhenPaused = v; return *this; } \
private: \
    std::vector<std::string> m_After, m_Before, m_ConcurrentWith; \
    bool m_ParallelSafe = false, m_UpdateWhenPaused = false; \
}; \
inline void ClassName::Execute(SystemContext& ctx)

// Trailing ... may be empty (e.g. REGISTER_SYSTEM(CombatSystem)); requires
// empty __VA_ARGS__ support, same as ECS_COMPONENT in Entity.h.
//
// Safe to put directly in a header, even one included by many .cpp files:
// the registrar is a C++17 inline variable, not a plain static one, so the
// linker collapses every translation unit's copy into a single instance
// instead of adding one duplicate system per include. EcsDetail holds only
// generated plumbing - never reference it directly.
#define REGISTER_SYSTEM(ClassName, ...) \
    namespace EcsDetail { \
        struct ClassName##_Registrar \
        { \
            ClassName##_Registrar() \
            { \
                auto sys = std::make_unique<ClassName>(); \
                sys->Self() __VA_ARGS__; \
                EcsScheduler::AddSystem(std::move(sys)); \
            } \
        }; \
        inline ClassName##_Registrar ClassName##_registrar_instance; \
    }

// Manual alternative to REGISTER_SYSTEM - use when this code may live in a
// shared library (DLL/.so). See the "SHARED LIBRARIES" note in Ecs.h for why
// static-initializer registration can silently fail to run there. Same
// call, no static object at all - call it yourself, from a function you
// control, before EcsScheduler::Finalize():
//   REGISTER_SYSTEM_MANUAL(CombatSystem, .After({ "AI" }));
#define REGISTER_SYSTEM_MANUAL(ClassName, ...) \
    do { \
        auto sys = std::make_unique<ClassName>(); \
        sys->Self() __VA_ARGS__; \
        EcsScheduler::AddSystem(std::move(sys)); \
    } while (0)

// TICK_COMPONENT / TICK_ENTITY / REGISTER_TICK
//
// Shorthand for the extremely common "one system, one component, no
// cross-referencing" shape - the ForEach/ForEachEntity call becomes the
// system's entire Execute() automatically, so there's no lambda syntax or
// entt vocabulary to write for it at all:
//
//   TICK_COMPONENT(StaminaRegenSystem, PostUpdate, Stamina, sta)
//   {
//       sta.Current = std::min(sta.Max, sta.Current + sta.RegenRate * ctx.DeltaTime);
//   }
//   REGISTER_TICK(StaminaRegenSystem, .RunsConcurrentlyWith({ "AttackCooldownSystem" }))
//
//   TICK_ENTITY(GoblinBobSystem, PostLateUpdate, Bobbing, self, bob)
//   {
//       self->Position.y += std::sin(Time::GameTime * bob.Speed) * bob.Amplitude * ctx.DeltaTime;
//   }
//   REGISTER_TICK(GoblinBobSystem)
//
// TICK_COMPONENT gives you (ComponentType& varName) - pure data, like
// ctx.ForEach. TICK_ENTITY gives you (Entity* entityVar, ComponentType&
// componentVar) - like ctx.ForEachEntity.
//
// Only for SINGLE-component systems whose entire body is that one loop -
// the moment a system needs a second component type, cross-references
// another entity (an AI system looking up the player, say), or does
// anything before/after the loop, write it with DECLARE_SYSTEM/
// REGISTER_SYSTEM directly instead. Trying to force a multi-step system
// into this shape is what DECLARE_SYSTEM is already good at - this macro
// intentionally doesn't try to generalize past the one-loop case.
//
// REGISTER_TICK takes the same optional fluent config as REGISTER_SYSTEM
// (.After/.Before/.RunsConcurrentlyWith/.ParallelSafe/.UpdateWhenPaused) and
// closes out whichever of TICK_COMPONENT/TICK_ENTITY opened it - one closer
// macro for both, since the closing code is identical either way.
#define TICK_COMPONENT(ClassName, Grp, ComponentType, VarName) \
DECLARE_SYSTEM(ClassName, Grp) \
{ \
    ctx.ForEach<ComponentType>([&](entt::entity, ComponentType& VarName)

#define TICK_ENTITY(ClassName, Grp, ComponentType, EntityVarName, ComponentVarName) \
DECLARE_SYSTEM(ClassName, Grp) \
{ \
    ctx.ForEachEntity<ComponentType>([&](Entity* EntityVarName, ComponentType& ComponentVarName)

#define REGISTER_TICK(ClassName, ...) \
    ); \
} \
REGISTER_SYSTEM(ClassName, __VA_ARGS__)

// Manual alternative to REGISTER_TICK - see the "SHARED LIBRARIES" note in
// Ecs.h. TICK_COMPONENT/TICK_ENTITY open code at namespace scope, so unlike
// REGISTER_SYSTEM_MANUAL this can't be an inline call - it generates an
// inline ClassName##_RegisterManual() function instead (safe in a shared
// header for the same reason REGISTER_SYSTEM's inline variable is - it's a
// C++17 inline function, deduplicated across translation units). Call it
// yourself, before EcsScheduler::Finalize():
//   TICK_COMPONENT(StaminaRegenSystem, PostUpdate, Stamina, sta) { ... }
//   REGISTER_TICK_MANUAL(StaminaRegenSystem)
//   // elsewhere, in code you control: StaminaRegenSystem_RegisterManual();
#define REGISTER_TICK_MANUAL(ClassName, ...) \
    ); \
} \
inline void ClassName##_RegisterManual() \
{ \
    auto sys = std::make_unique<ClassName>(); \
    sys->Self() __VA_ARGS__; \
    EcsScheduler::AddSystem(std::move(sys)); \
}
