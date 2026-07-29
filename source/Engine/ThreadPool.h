#pragma once
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

// --- Feature toggles / portability -------------------------------------------

// Keep backward compat with your original macro name, but prefer the fixed one.
#if defined(DISABLE_TREADPOOL) && !defined(DISABLE_THREADPOOL)
#define DISABLE_THREADPOOL
#endif

#if defined(__EMSCRIPTEN__) && !defined(USE_EMSCRIPTEN_PTHREADS)
// If not building with pthreads on Emscripten, hard-disable threads.
#ifndef DISABLE_THREADPOOL
#define DISABLE_THREADPOOL
#endif
#endif

class ThreadPool {
public:
    ThreadPool(std::string name)
        : poolName_(std::move(name)) {}
    ~ThreadPool() { Stop(); }

    void Start(uint32_t num_threads);

    // Single job submission
    void QueueJob(const std::function<void()>& job);
    void QueueJob(std::function<void()>&& job);

    // Batch job submission (optimized for multiple jobs)
    void QueueJobs(const std::vector<std::function<void()>>& jobs);
    void QueueJobs(std::vector<std::function<void()>>&& jobs);

    void Stop();                 // drains remaining jobs then joins
    bool IsBusy() const noexcept
    {
#ifdef DISABLE_THREADPOOL
        return false;
#else
        return work_count_.load(std::memory_order_acquire) != 0;
#endif
    }
    void WaitForFinish();

    template <class Func>
    void ParallelFor(size_t count, Func func) {
        if (count == 0) return;
#ifdef DISABLE_THREADPOOL
        for (size_t i = 0; i < count; ++i) func(i);
#else
        size_t numThreads = std::max<size_t>(1, threads_.size());
        size_t chunk = (count + numThreads - 1) / numThreads;

        std::vector<std::function<void()>> jobs;
        jobs.reserve(numThreads);

        for (size_t begin = 0; begin < count; begin += chunk) {
            size_t end = std::min(count, begin + chunk);
            jobs.emplace_back([func, begin, end]() {
                for (size_t i = begin; i < end; ++i) {
                    func(i);
                }
                });
        }

        QueueJobs(std::move(jobs));
        WaitForFinish(); // ParallelFor is self-contained; callers don't need to call this separately
#endif
    }

    // Optional convenience API: returns a future<T>
    template <class F, class... Args>
    auto Enqueue(F&& f, Args&&... args)
        -> std::future<std::invoke_result_t<F, Args...>>;

    static int GetMaxThreads();
    static int GetNumThreadsForPhysics();
    static int GetNumThreadsForAsyncUpdate();
    static int GetNumThreadsForThreadPool();

    static inline bool Supported() {
#ifdef DISABLE_THREADPOOL
        return false;
#else
        return true;
#endif
    }

    // Number of tasks currently executing (informational)
    std::atomic<int> performingJobs{ 0 };

	bool DisableWaitCooldown = false; // If true, Worker will not sleep to reduce CPU usage. Use with caution.

private:
    void Worker();

    std::string poolName_;

#ifndef DISABLE_THREADPOOL
    mutable std::mutex mtx_;
    std::condition_variable cv_job_;
    std::condition_variable cv_done_;

    std::vector<std::thread> threads_;
    std::queue<std::function<void()>> jobs_;

    std::atomic<uint32_t> work_count_{ 0 };  // queued + running tasks
    bool stopping_ = false;
#endif
};