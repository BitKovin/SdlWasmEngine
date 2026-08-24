#pragma once
// IThreadPool.h
//
// Minimal interface over your engine's actual thread pool. Wrap your
// existing asyncUpdateThreadPool in a few-line adapter implementing this, or
// add a matching ParallelFor overload to it directly if you own that class.
// If no pool is set on the scheduler, work simply runs sequentially - a safe
// fallback, not an error, useful for e.g. a dedicated-server or editor
// context.

#include <cstddef>
#include <functional>

class IThreadPool
{
public:
    virtual ~IThreadPool() = default;
    virtual void ParallelFor(size_t count, const std::function<void(size_t)>& fn) = 0;
};
