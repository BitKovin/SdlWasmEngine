#pragma once
// LevelThreadPoolAdapter.h
//
// Routes EcsScheduler's ParallelSafe()/RunsConcurrentlyWith() batching
// through this project's real Level::asyncUpdateThreadPool - the same pool
// Level::AsyncUpdate()/PreFinalize() already use for their own per-entity
// parallel work.
//
// No pointer is cached: Level::Current is a different object (with a
// different pool) after every level load, so this always forwards to
// whichever pool is current at call time.

#include <Ecs/IThreadPool.h>
#include "Level.hpp"

class LevelThreadPoolAdapter : public IThreadPool
{
public:
    void ParallelFor(size_t count, const std::function<void(size_t)>& fn) override
    {
        if (Level::Current && Level::Current->asyncUpdateThreadPool)
            Level::Current->asyncUpdateThreadPool->ParallelFor(count, fn);
        else
            for (size_t i = 0; i < count; ++i) fn(i); // no level loaded yet - run sequentially
    }
};
