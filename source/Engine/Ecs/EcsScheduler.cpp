#include <Ecs/EcsScheduler.h>
#include <Ecs/EventRegistry.h>

#include <algorithm>
#include <cassert>
#include <queue>
#include <sstream>
#include <stdexcept>

void EcsScheduler::Finalize()
{
    auto& data = Storage();
    assert(!data.Finalized && "Finalize() called twice");

    for (auto& sys : data.Pending)
        data.Groups[(size_t)sys->Group()].push_back(std::move(sys));
    data.Pending.clear();

    std::unordered_map<std::string, std::pair<ISystem*, TickGroup>> globalByName;
    for (size_t g = 0; g < TickGroupCount; ++g)
        for (auto& s : data.Groups[g])
            globalByName[s->Name()] = { s.get(), (TickGroup)g };

    for (size_t g = 0; g < TickGroupCount; ++g)
        data.Plans[g] = BuildExecutionOrder((TickGroup)g, data.Groups[g], globalByName);

    data.Finalized = true;
}

EcsScheduler::TickGroupPlan EcsScheduler::BuildExecutionOrder(
    TickGroup group, std::vector<std::unique_ptr<ISystem>>& systems,
    const std::unordered_map<std::string, std::pair<ISystem*, TickGroup>>& globalByName)
{
    std::unordered_map<std::string, ISystem*> byName;
    for (auto& s : systems) byName[s->Name()] = s.get();

    std::vector<SystemEdge> edges;
    for (auto& s : systems)
    {
        for (auto& dep : s->After())
        {
            ValidateReference(s.get(), dep, "After", group, byName, globalByName);
            edges.push_back({ byName[dep], s.get(),
                std::string(s->Name()) + "::After(\"" + dep + "\")  [" + s->SourceLocation() + "]" });
        }
        for (auto& dep : s->Before())
        {
            ValidateReference(s.get(), dep, "Before", group, byName, globalByName);
            edges.push_back({ s.get(), byName[dep],
                std::string(s->Name()) + "::Before(\"" + dep + "\")  [" + s->SourceLocation() + "]" });
        }
    }

    std::unordered_map<ISystem*, std::vector<SystemEdge>> outgoing;
    for (auto& e : edges) outgoing[e.From].push_back(e);

    DetectCyclesOrThrow(group, systems, outgoing);
    auto reachable = ComputeReachability(systems, outgoing);
    ValidateConcurrencyOrThrow(group, systems, byName, outgoing, reachable);

    auto sorted  = TopoSort(systems, outgoing);
    auto batches = BuildBatches(sorted, reachable);

    TickGroupPlan plan;
    plan.Batches   = std::move(batches);
    plan.DebugText = FormatDebugText(group, systems, edges, plan.Batches);
    return plan;
}

void EcsScheduler::ValidateReference(
    ISystem* owner, const std::string& refName, const char* declKind, TickGroup group,
    const std::unordered_map<std::string, ISystem*>& byName,
    const std::unordered_map<std::string, std::pair<ISystem*, TickGroup>>& globalByName)
{
    if (refName == owner->Name())
        throw std::runtime_error("EcsScheduler: '" + std::string(owner->Name()) +
            "' references itself in " + declKind + "(). [" + owner->SourceLocation() + "]");

    if (byName.count(refName)) return;

    auto git = globalByName.find(refName);
    if (git != globalByName.end())
        throw std::runtime_error("EcsScheduler: '" + std::string(owner->Name()) +
            "' declares " + declKind + "(\"" + refName + "\") in tick group '" +
            std::string(TickGroupName(group)) + "', but '" + refName + "' runs in tick group '" +
            std::string(TickGroupName(git->second.second)) + "'. Tick groups already run "
            "sequentially - cross-group ordering is implicit and doesn't need (or support) "
            "an explicit declaration. [" + owner->SourceLocation() + "]");

    throw std::runtime_error("EcsScheduler: '" + std::string(owner->Name()) +
        "' declares " + declKind + "(\"" + refName + "\"), but no system with that name is "
        "registered in tick group '" + std::string(TickGroupName(group)) + "'. [" +
        owner->SourceLocation() + "]");
}

void EcsScheduler::DetectCyclesOrThrow(
    TickGroup group, const std::vector<std::unique_ptr<ISystem>>& systems,
    const std::unordered_map<ISystem*, std::vector<SystemEdge>>& outgoing)
{
    enum class State { White, Gray, Black };
    std::unordered_map<ISystem*, State> state;
    std::vector<ISystem*>   nodeStack;
    std::vector<SystemEdge> edgeStack;

    std::function<void(ISystem*)> dfs = [&](ISystem* node)
    {
        state[node] = State::Gray;
        nodeStack.push_back(node);

        auto it = outgoing.find(node);
        if (it != outgoing.end())
        {
            for (auto& edge : it->second)
            {
                State st = state.count(edge.To) ? state[edge.To] : State::White;

                if (st == State::White)
                {
                    edgeStack.push_back(edge);
                    dfs(edge.To);
                    edgeStack.pop_back();
                }
                else if (st == State::Gray)
                {
                    auto pos = std::find(nodeStack.begin(), nodeStack.end(), edge.To);
                    size_t startIdx = std::distance(nodeStack.begin(), pos);

                    std::ostringstream msg;
                    msg << "EcsScheduler: impossible system ordering in tick group '"
                        << TickGroupName(group) << "'.\n"
                        << "The following declarations form a cycle and cannot all be satisfied:\n";
                    for (size_t i = startIdx; i < nodeStack.size(); ++i)
                    {
                        const SystemEdge& e = (i < edgeStack.size()) ? edgeStack[i] : edge;
                        msg << "  " << e.Reason << "\n";
                    }
                    msg << "Break the cycle by removing or reworking one of the declarations above.";
                    throw std::runtime_error(msg.str());
                }
                // Black: fully explored already, no cycle down this path
            }
        }

        state[node] = State::Black;
        nodeStack.pop_back();
    };

    for (auto& s : systems)
        if (!state.count(s.get()))
            dfs(s.get());
}

std::unordered_map<ISystem*, std::unordered_set<ISystem*>> EcsScheduler::ComputeReachability(
    const std::vector<std::unique_ptr<ISystem>>& systems,
    const std::unordered_map<ISystem*, std::vector<SystemEdge>>& outgoing)
{
    std::unordered_map<ISystem*, std::unordered_set<ISystem*>> result;
    for (auto& s : systems)
    {
        auto& visited = result[s.get()];
        std::vector<ISystem*> stack{ s.get() };
        while (!stack.empty())
        {
            ISystem* cur = stack.back();
            stack.pop_back();
            auto it = outgoing.find(cur);
            if (it == outgoing.end()) continue;
            for (auto& e : it->second)
                if (visited.insert(e.To).second)
                    stack.push_back(e.To);
        }
    }
    return result; // O(V*(V+E)) - trivial at engine system counts
}

EcsScheduler::ReachPath EcsScheduler::FindPath(
    ISystem* from, ISystem* to,
    const std::unordered_map<ISystem*, std::vector<SystemEdge>>& outgoing)
{
    std::unordered_map<ISystem*, SystemEdge> cameVia;
    std::unordered_set<ISystem*> visited{ from };
    std::queue<ISystem*> q;
    q.push(from);

    while (!q.empty())
    {
        ISystem* cur = q.front(); q.pop();
        if (cur == to) break;
        auto it = outgoing.find(cur);
        if (it == outgoing.end()) continue;
        for (auto& e : it->second)
            if (visited.insert(e.To).second) { cameVia[e.To] = e; q.push(e.To); }
    }
    if (!visited.count(to)) return {};

    ReachPath result;
    result.Found = true;
    for (ISystem* cur = to; cur != from; cur = cameVia[cur].From)
        result.Path.push_back(cameVia[cur]);
    std::reverse(result.Path.begin(), result.Path.end());
    return result;
}

void EcsScheduler::ValidateConcurrencyOrThrow(
    TickGroup group, const std::vector<std::unique_ptr<ISystem>>& systems,
    const std::unordered_map<std::string, ISystem*>& byName,
    const std::unordered_map<ISystem*, std::vector<SystemEdge>>& outgoing,
    const std::unordered_map<ISystem*, std::unordered_set<ISystem*>>& reachable)
{
    for (auto& s : systems)
    {
        for (auto& otherName : s->RunsConcurrentlyWith())
        {
            if (otherName == s->Name())
                throw std::runtime_error("EcsScheduler: '" + std::string(s->Name()) +
                    "' declares RunsConcurrentlyWith itself. [" + s->SourceLocation() + "]");

            auto it = byName.find(otherName);
            if (it == byName.end())
                throw std::runtime_error("EcsScheduler: '" + std::string(s->Name()) +
                    "' declares RunsConcurrentlyWith(\"" + otherName + "\"), but no system with "
                    "that name is registered in tick group '" + std::string(TickGroupName(group)) +
                    "'. [" + s->SourceLocation() + "]");

            ISystem* other = it->second;
            bool forward  = reachable.at(s.get()).count(other) != 0;
            bool backward = reachable.at(other).count(s.get()) != 0;
            if (!forward && !backward) continue;

            auto path = forward ? FindPath(s.get(), other, outgoing)
                                 : FindPath(other, s.get(), outgoing);

            std::ostringstream msg;
            msg << "EcsScheduler: impossible combination in tick group '" << TickGroupName(group) << "'.\n"
                << "'" << s->Name() << "' declares RunsConcurrentlyWith(\"" << other->Name()
                << "\") [" << s->SourceLocation() << "], but an ordering dependency requires them to run in sequence:\n";
            for (auto& e : path.Path) msg << "  " << e.Reason << "\n";
            msg << "A system cannot be required to run concurrently with a system it is also "
                   "ordered relative to.\nRemove the RunsConcurrentlyWith declaration or the "
                   "conflicting After/Before declaration.";
            throw std::runtime_error(msg.str());
        }
    }
}

std::vector<ISystem*> EcsScheduler::TopoSort(
    const std::vector<std::unique_ptr<ISystem>>& systems,
    const std::unordered_map<ISystem*, std::vector<SystemEdge>>& outgoing)
{
    std::unordered_map<ISystem*, int> inDegree;
    for (auto& s : systems) inDegree[s.get()] = 0;
    for (auto& [from, list] : outgoing)
        for (auto& e : list) inDegree[e.To]++;

    std::queue<ISystem*> ready;
    for (auto& s : systems) if (inDegree[s.get()] == 0) ready.push(s.get());

    std::vector<ISystem*> sorted;
    while (!ready.empty())
    {
        ISystem* node = ready.front(); ready.pop();
        sorted.push_back(node);
        auto it = outgoing.find(node);
        if (it == outgoing.end()) continue;
        for (auto& e : it->second)
            if (--inDegree[e.To] == 0) ready.push(e.To);
    }
    return sorted; // guaranteed complete - DetectCyclesOrThrow already ran
}

std::vector<std::vector<ISystem*>> EcsScheduler::BuildBatches(
    const std::vector<ISystem*>& sorted,
    const std::unordered_map<ISystem*, std::unordered_set<ISystem*>>& reachable)
{
    auto ordered = [&](ISystem* a, ISystem* b)
    { return reachable.at(a).count(b) || reachable.at(b).count(a); };

    auto mutuallyConcurrent = [&](ISystem* a, ISystem* b)
    {
        auto al = a->RunsConcurrentlyWith(), bl = b->RunsConcurrentlyWith();
        return std::find(al.begin(), al.end(), std::string(b->Name())) != al.end()
            && std::find(bl.begin(), bl.end(), std::string(a->Name())) != bl.end();
    };

    std::vector<std::vector<ISystem*>> batches;
    std::vector<ISystem*> current;

    for (ISystem* s : sorted)
    {
        bool canJoin = !current.empty();
        for (ISystem* member : current)
            // ordered() should be unreachable here - ValidateConcurrencyOrThrow already
            // rejects any mutually-concurrent pair with an ordering relation. Kept as
            // defense-in-depth, not load-bearing logic.
            if (ordered(s, member) || !mutuallyConcurrent(s, member)) { canJoin = false; break; }

        if (canJoin) current.push_back(s);
        else { if (!current.empty()) batches.push_back(current); current = { s }; }
    }
    if (!current.empty()) batches.push_back(current);
    return batches;
}

std::string EcsScheduler::FormatDebugText(
    TickGroup group, const std::vector<std::unique_ptr<ISystem>>& systems,
    const std::vector<SystemEdge>& edges,
    const std::vector<std::vector<ISystem*>>& batches)
{
    std::ostringstream out;
    out << "== TickGroup: " << TickGroupName(group) << " ==\n";
    if (systems.empty()) { out << "  (no systems registered)\n"; return out.str(); }

    out << "Execution order (" << batches.size() << " sequential step"
        << (batches.size() == 1 ? "" : "s") << "):\n";
    for (size_t i = 0; i < batches.size(); ++i)
    {
        auto& batch = batches[i];
        if (batch.size() == 1)
        {
            out << "  " << (i + 1) << ". " << batch[0]->Name()
                << (batch[0]->ParallelSafe() ? "  [internally parallel]" : "") << "\n";
        }
        else
        {
            out << "  " << (i + 1) << ". [parallel batch]\n";
            for (auto* s : batch)
                out << "       - " << s->Name()
                    << (s->ParallelSafe() ? "  [also internally parallel]" : "") << "\n";
        }
    }

    out << "Declared dependencies:\n";
    if (edges.empty()) out << "  (none)\n";
    for (auto& e : edges) out << "  " << e.Reason << "\n";

    out << "Warnings:\n";
    bool anyWarning = false;
    std::unordered_map<std::string, ISystem*> byName;
    for (auto& s : systems) byName[s->Name()] = s.get();
    for (auto& s : systems)
        for (auto& otherName : s->RunsConcurrentlyWith())
        {
            auto it = byName.find(otherName);
            if (it == byName.end()) continue;
            auto otherList = it->second->RunsConcurrentlyWith();
            bool reciprocal = std::find(otherList.begin(), otherList.end(),
                                        std::string(s->Name())) != otherList.end();
            if (!reciprocal)
            {
                out << "  '" << s->Name() << "' declares RunsConcurrentlyWith(\"" << otherName
                    << "\"), but '" << otherName << "' doesn't declare it back - the pair "
                       "will NOT be batched together.\n";
                anyWarning = true;
            }
        }
    if (!anyWarning) out << "  (none)\n";
    return out.str();
}

void EcsScheduler::RunTickGroup(TickGroup group, entt::registry& registry, float dt, bool paused)
{
    auto& data = Storage();
    assert(data.Finalized && "RunTickGroup called before Finalize()");

    SystemContext ctx{ registry, dt, data.Events, data.Commands, data.ThreadPool };
    auto& batches = data.Plans[(size_t)group].Batches;

    auto runOne = [&](ISystem* sys)
    {
        if (paused && !sys->UpdateWhenPaused()) return;
        sys->Execute(ctx);
    };

    for (auto& batch : batches)
    {
        if (batch.size() == 1 || data.ThreadPool == nullptr)
        {
            for (auto* sys : batch) runOne(sys);
        }
        else
        {
            data.ThreadPool->ParallelFor(batch.size(), [&](size_t i) { runOne(batch[i]); });
        }
    }

    // one flush point per tick group: deferred structural changes first,
    // then fan out everything emitted this tick group to event handlers
    data.Commands.Flush(registry);
    data.Events.FlushAll(data.Dispatcher);
}

std::string EcsScheduler::DumpExecutionOrder()
{
    auto& data = Storage();
    assert(data.Finalized && "DumpExecutionOrder called before Finalize()");
    std::ostringstream out;
    for (size_t g = 0; g < TickGroupCount; ++g)
        out << data.Plans[g].DebugText << "\n";
    return out.str();
}

std::string EcsScheduler::DumpAll()
{
    std::ostringstream out;
    out << DumpExecutionOrder();
    out << EventRegistry::DumpText();
    return out.str();
}

std::vector<SystemInfo> EcsScheduler::AllSystems()
{
    std::vector<SystemInfo> out;
    auto& data = Storage();
    for (size_t g = 0; g < TickGroupCount; ++g)
        for (auto& s : data.Groups[g])
            out.push_back({ s->Name(), s->Group(), s->After(), s->Before(), s->ParallelSafe() });
    return out;
}
