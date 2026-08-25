#pragma once
// ParallelEventBus.h
//
// Lets any system - including code running inside a ParallelSafe() system's
// own internal ParallelFor - emit events from any thread, with zero setup
// per event type. The first Push<Event>() for a given type lazily registers
// how to drain it; FlushAll() drains every type that's ever been pushed and
// fans the results out through the dispatcher, single-threaded.
//
// This is engine-owned and internal - game code never touches it directly,
// only through SystemContext::Emit<T>().

#include <entt/entt.hpp>
#include <any>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <typeindex>
#include <unordered_map>
#include <vector>

class ParallelEventBus
{
public:
    template<typename Event>
    void Push(Event e)
    {
        auto& shard = Pool<Event>()[ShardIndex()];
        std::lock_guard lock(shard.Mutex);
        shard.Items.push_back(std::move(e));
    }

    // Drains every event type that has ever been pushed through this bus,
    // enqueues onto dispatcher, and calls dispatcher.update<Event>() once
    // per type. Call once per tick group, single-threaded.
    void FlushAll(entt::dispatcher& dispatcher)
    {
        for (auto& flush : m_Flushers)
            flush(dispatcher);
    }

private:
    static constexpr size_t ShardCount = 16; // comfortably above expected worker-thread counts

    template<typename T>
    struct Shard
    {
        std::mutex     Mutex;
        std::vector<T> Items;
    };

    // Thread-pool job indices aren't stable worker ids, so shard by the
    // calling thread's own id instead - correct regardless of how many jobs
    // get split across however many actual worker threads exist.
    static size_t ShardIndex()
    {
        return std::hash<std::thread::id>{}(std::this_thread::get_id()) % ShardCount;
    }

    template<typename Event>
    std::vector<Shard<Event>>& Pool()
    {
        auto key = std::type_index(typeid(Event));
        auto it = m_Storage.find(key);
        if (it == m_Storage.end())
        {
            // std::any requires whatever type it holds to be copy-
            // constructible, even though it's never actually copied here -
            // that's just what the standard demands of any's converting
            // constructor. Shard<T> owns a std::mutex, so
            // std::vector<Shard<T>> isn't copy-constructible and can't be
            // stored in the any directly. Store a shared_ptr to the pool
            // instead: shared_ptr is trivially copyable (copying just bumps
            // a refcount), so any is satisfied, and the actual vector of
            // shards - mutexes included - is never copied.
            auto pool = std::make_shared<std::vector<Shard<Event>>>(ShardCount);
            it = m_Storage.emplace(key, pool).first;
            m_Flushers.push_back([this, key](entt::dispatcher& dispatcher)
                {
                    auto& shards = *std::any_cast<std::shared_ptr<std::vector<Shard<Event>>>&>(m_Storage[key]);
                    for (auto& shard : shards)
                    {
                        std::lock_guard lock(shard.Mutex);
                        for (auto& e : shard.Items) dispatcher.enqueue(std::move(e));
                        shard.Items.clear();
                    }
                    dispatcher.update<Event>();
                });
        }
        return *std::any_cast<std::shared_ptr<std::vector<Shard<Event>>>&>(it->second);
    }

    std::unordered_map<std::type_index, std::any> m_Storage;
    std::vector<std::function<void(entt::dispatcher&)>> m_Flushers;
};