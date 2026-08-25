#pragma once
// EcsScheduler.h
//
// Static, no-instance scheduler: every call is EcsScheduler::Something(...),
// no ::Get() anywhere. Owns every system, resolves execution order once at
// Finalize(), and runs each tick group's systems - sequential or batched-
// parallel as declared - from EngineMain::GameUpdate(). Also owns the
// engine-wide ParallelEventBus, CommandBuffer, and entt::dispatcher, so game
// code never sees any of those directly - only through SystemContext.

#include <Ecs/TickGroup.h>
#include <Ecs/ISystem.h>
#include <Ecs/SystemContext.h>
#include <Ecs/ParallelEventBus.h>
#include <Ecs/CommandBuffer.h>
#include <Ecs/IThreadPool.h>

#include <entt/entt.hpp>
#include <array>
#include <cassert>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

struct SystemEdge
{
    ISystem*    From = nullptr; // must complete first
    ISystem*    To   = nullptr; // depends on From
    std::string Reason;         // e.g. `CombatSystem::After("AISystem")` - for diagnostics
};

struct SystemInfo
{
    std::string               Name;
    TickGroup                 Group;
    std::vector<std::string>  After;
    std::vector<std::string>  Before;
    bool                      ParallelSafe;
};

class EcsScheduler
{
public:
    EcsScheduler() = delete;

    static void SetThreadPool(IThreadPool* pool) { Storage().ThreadPool = pool; }

    static void AddSystem(std::unique_ptr<ISystem> system)
    {
        assert(!Storage().Finalized && "AddSystem called after Finalize() - all systems must register before Finalize()");
        Storage().Pending.push_back(std::move(system));
    }

    template<typename Event>
    static auto OnEvent() { return Storage().Dispatcher.sink<Event>(); }

    // Same as ctx.Emit<Event>() from inside a system, but callable from
    // anywhere - game code that isn't a system, an Entity method, outside
    // any tick group entirely. Safe to call from any thread; buffered the
    // same way ctx.Emit is.
    //
    // The event bus is shared across all four tick groups, not per-group -
    // an event emitted here is only flushed to its handlers at the end of
    // whichever tick group's RunTickGroup() runs NEXT, chronologically,
    // regardless of which group that happens to be. If you're calling this
    // from inside a system, prefer ctx.Emit() instead - same effect, but it
    // makes the buffering explicit at the call site.
    template<typename Event>
    static void Emit(Event e) { Storage().Events.Push(std::move(e)); }

    // Call once at engine init, after every REGISTER_SYSTEM static registrar
    // has run. Builds and validates the execution order for every tick
    // group; throws std::runtime_error naming the exact offending
    // declaration(s) if any tick group's constraints are unsatisfiable.
    static void Finalize();

    static void RunTickGroup(TickGroup group, entt::registry& registry, float dt, bool paused);

    // Human-readable execution order + declared dependencies + concurrency
    // warnings for every tick group. Safe to log at startup or wire to a
    // console command.
    static std::string DumpExecutionOrder();

    // DumpExecutionOrder() plus registered event types and their handlers.
    static std::string DumpAll();

    static std::vector<SystemInfo> AllSystems();

    static bool IsFinalized() { return Storage().Finalized; }

private:
    struct TickGroupPlan
    {
        std::vector<std::vector<ISystem*>> Batches;
        std::string                        DebugText;
    };

    struct Data
    {
        std::vector<std::unique_ptr<ISystem>>                             Pending;
        std::array<std::vector<std::unique_ptr<ISystem>>, TickGroupCount> Groups;
        std::array<TickGroupPlan, TickGroupCount>                         Plans;
        bool              Finalized = false;
        ParallelEventBus  Events;
        CommandBuffer     Commands;
        entt::dispatcher  Dispatcher;
        IThreadPool*      ThreadPool = nullptr;
    };
    static Data& Storage() { static Data data; return data; }

    static TickGroupPlan BuildExecutionOrder(
        TickGroup group, std::vector<std::unique_ptr<ISystem>>& systems,
        const std::unordered_map<std::string, std::pair<ISystem*, TickGroup>>& globalByName);

    static void ValidateReference(
        ISystem* owner, const std::string& refName, const char* declKind, TickGroup group,
        const std::unordered_map<std::string, ISystem*>& byName,
        const std::unordered_map<std::string, std::pair<ISystem*, TickGroup>>& globalByName);

    static void DetectCyclesOrThrow(
        TickGroup group, const std::vector<std::unique_ptr<ISystem>>& systems,
        const std::unordered_map<ISystem*, std::vector<SystemEdge>>& outgoing);

    static std::unordered_map<ISystem*, std::unordered_set<ISystem*>> ComputeReachability(
        const std::vector<std::unique_ptr<ISystem>>& systems,
        const std::unordered_map<ISystem*, std::vector<SystemEdge>>& outgoing);

    struct ReachPath { bool Found = false; std::vector<SystemEdge> Path; };
    static ReachPath FindPath(
        ISystem* from, ISystem* to,
        const std::unordered_map<ISystem*, std::vector<SystemEdge>>& outgoing);

    static void ValidateConcurrencyOrThrow(
        TickGroup group, const std::vector<std::unique_ptr<ISystem>>& systems,
        const std::unordered_map<std::string, ISystem*>& byName,
        const std::unordered_map<ISystem*, std::vector<SystemEdge>>& outgoing,
        const std::unordered_map<ISystem*, std::unordered_set<ISystem*>>& reachable);

    static std::vector<ISystem*> TopoSort(
        const std::vector<std::unique_ptr<ISystem>>& systems,
        const std::unordered_map<ISystem*, std::vector<SystemEdge>>& outgoing);

    static std::vector<std::vector<ISystem*>> BuildBatches(
        const std::vector<ISystem*>& sorted,
        const std::unordered_map<ISystem*, std::unordered_set<ISystem*>>& reachable);

    static std::string FormatDebugText(
        TickGroup group, const std::vector<std::unique_ptr<ISystem>>& systems,
        const std::vector<SystemEdge>& edges,
        const std::vector<std::vector<ISystem*>>& batches);
};
