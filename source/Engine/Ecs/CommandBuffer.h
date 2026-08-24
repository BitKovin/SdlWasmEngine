#pragma once
// CommandBuffer.h
//
// Defers structural registry changes (component add/remove, entity destroy,
// or arbitrary registry mutation) so they're safe to request from any thread
// during a parallel system, then applied in one single-threaded pass at the
// end of the tick group.
//
// This is engine-owned and internal - game code never touches it directly,
// only through SystemContext.

#include <entt/entt.hpp>
#include <functional>
#include <mutex>
#include <vector>

class CommandBuffer
{
public:
    void Destroy(entt::entity e)
    {
        std::lock_guard lock(m_Mutex);
        m_Destroys.push_back(e);
    }

    template<typename T, typename... Args>
    void Emplace(entt::entity e, Args&&... args)
    {
        std::lock_guard lock(m_Mutex);
        m_Pending.push_back([e, ...capturedArgs = std::forward<Args>(args)](entt::registry& r) mutable
        {
            r.emplace_or_replace<T>(e, std::move(capturedArgs)...);
        });
    }

    template<typename T>
    void Remove(entt::entity e)
    {
        std::lock_guard lock(m_Mutex);
        m_Pending.push_back([e](entt::registry& r) { r.remove<T>(e); });
    }

    // Escape hatch for anything the typed helpers above don't cover - e.g.
    // spawning a brand new entity mid-parallel-phase. Runs during the same
    // single-threaded Flush() as everything else, so registry.create() and
    // friends are safe to call inside fn.
    void Enqueue(std::function<void(entt::registry&)> fn)
    {
        std::lock_guard lock(m_Mutex);
        m_Pending.push_back(std::move(fn));
    }

    void Flush(entt::registry& r)
    {
        for (auto& fn : m_Pending) fn(r);
        for (auto e : m_Destroys) if (r.valid(e)) r.destroy(e);
        m_Pending.clear();
        m_Destroys.clear();
    }

private:
    std::mutex m_Mutex;
    std::vector<std::function<void(entt::registry&)>> m_Pending;
    std::vector<entt::entity> m_Destroys;
};
