#include "ThreadPool.h"

#ifndef DISABLE_TREADPOOL
#include <thread>
#include <chrono>
#include <atomic>
#include <vector>
#include <queue>
using namespace std::chrono_literals;
#endif

void ThreadPool::Start(uint32_t num_threads) {
#ifdef DISABLE_THREADPOOL
    (void)num_threads;
#else
    std::lock_guard<std::mutex> lk(mtx_);
    if (!threads_.empty()) return;
    stopping_ = false;
    threads_.reserve(num_threads);
    for (uint32_t i = 0; i < num_threads; ++i) {
        threads_.emplace_back(&ThreadPool::Worker, this);
    }
#endif
}

void ThreadPool::Worker() {
#ifndef DISABLE_THREADPOOL
    // Thread-local queue to reduce contention
    std::queue<std::function<void()>> localQueue;
    uint32_t idle_count = 0;

    for (;;) {
        // First, process any jobs in local queue
        if (!localQueue.empty()) {
            auto job = std::move(localQueue.front());
            localQueue.pop();

            performingJobs.fetch_add(1, std::memory_order_relaxed);
            job();
            performingJobs.fetch_sub(1, std::memory_order_relaxed);

            // Batch update work_count_ to reduce atomic operations
            if (work_count_.fetch_sub(1, std::memory_order_acq_rel) == 1) {
                std::lock_guard<std::mutex> lk(mtx_);
                cv_done_.notify_all();
            }
            idle_count = 0;
            continue;
        }

        // Try to grab multiple jobs from global queue to reduce lock contention
        std::function<void()> job;
        bool has_job = false;

        {
            std::unique_lock<std::mutex> lk(mtx_);

            // Batch job extraction - grab multiple jobs if available
            constexpr uint32_t BATCH_SIZE = 8;
            uint32_t jobs_grabbed = 0;

            while (!jobs_.empty() && jobs_grabbed < BATCH_SIZE) {
                localQueue.push(std::move(jobs_.front()));
                jobs_.pop();
                jobs_grabbed++;
                has_job = true;
            }

            if (has_job) {
                idle_count = 0;
                continue; // Go process local queue
            }

            // No jobs available, wait efficiently
            if (stopping_) {
                return;
            }

            if (DisableWaitCooldown == false)
            {
				// Increase wait time based on idle count to reduce spurious wakeups
				if (idle_count < 10) {
					cv_job_.wait(lk);
				}
				else if (idle_count < 50) {
					cv_job_.wait_for(lk, 1ms);
				}
				else {
					cv_job_.wait_for(lk, 2ms);
				}

			}
            idle_count++;
        }

        if (stopping_ && jobs_.empty()) {
            return;
        }
    }
#endif
}

void ThreadPool::QueueJob(const std::function<void()>& job) {
#ifdef DISABLE_THREADPOOL
    performingJobs.fetch_add(1, std::memory_order_relaxed);
    job();
    performingJobs.fetch_sub(1, std::memory_order_relaxed);
#else
    {
        std::lock_guard<std::mutex> lk(mtx_);
        jobs_.push(job);
        work_count_.fetch_add(1, std::memory_order_acq_rel);
    }
    cv_job_.notify_one();
#endif
}

void ThreadPool::QueueJob(std::function<void()>&& job) {
#ifdef DISABLE_THREADPOOL
    performingJobs.fetch_add(1, std::memory_order_relaxed);
    job();
    performingJobs.fetch_sub(1, std::memory_order_relaxed);
#else
    {
        std::lock_guard<std::mutex> lk(mtx_);
        jobs_.push(std::move(job));
        work_count_.fetch_add(1, std::memory_order_acq_rel);
    }
    cv_job_.notify_one();
#endif
}

// Batch job submission to reduce lock contention
void ThreadPool::QueueJobs(const std::vector<std::function<void()>>& jobs) {
#ifndef DISABLE_THREADPOOL
    if (jobs.empty()) return;

    {
        std::lock_guard<std::mutex> lk(mtx_);
        for (const auto& job : jobs) {
            jobs_.push(job);
        }
        work_count_.fetch_add(jobs.size(), std::memory_order_acq_rel);
    }

    // Notify multiple threads based on job count
    const size_t threads_to_notify = std::min<size_t>(jobs.size(), threads_.size());
    for (size_t i = 0; i < threads_to_notify; ++i) {
        cv_job_.notify_one();
    }
#endif
}

void ThreadPool::QueueJobs(std::vector<std::function<void()>>&& jobs) {
#ifndef DISABLE_THREADPOOL
    if (jobs.empty()) return;

    {
        std::lock_guard<std::mutex> lk(mtx_);
        for (auto& job : jobs) {
            jobs_.push(std::move(job));
        }
        work_count_.fetch_add(jobs.size(), std::memory_order_acq_rel);
    }

    const size_t threads_to_notify = std::min<size_t>(jobs.size(), threads_.size());
    for (size_t i = 0; i < threads_to_notify; ++i) {
        cv_job_.notify_one();
    }

#else

    for (auto job : jobs)
    {
        job();
    }

#endif


}

template <class F, class... Args>
auto ThreadPool::Enqueue(F&& f, Args&&... args)
-> std::future<std::invoke_result_t<F, Args...>> {
    using R = std::invoke_result_t<F, Args...>;
    auto task = std::make_shared<std::packaged_task<R()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));
    std::future<R> fut = task->get_future();
    QueueJob([task]() mutable { (*task)(); });
    return fut;
}

void ThreadPool::WaitForFinish() {
#ifndef DISABLE_THREADPOOL
    std::unique_lock<std::mutex> lk(mtx_);
    cv_done_.wait(lk, [this] {
        return work_count_.load(std::memory_order_acquire) == 0;
        });
#endif
}

void ThreadPool::Stop() {
#ifndef DISABLE_THREADPOOL
    {
        std::lock_guard<std::mutex> lk(mtx_);
        if (threads_.empty()) return;
        stopping_ = true;
    }
    cv_job_.notify_all();
    for (auto& t : threads_) t.join();
    threads_.clear();
    {
        std::lock_guard<std::mutex> lk(mtx_);
        while (!jobs_.empty()) jobs_.pop();
        stopping_ = false;
        work_count_.store(0, std::memory_order_release);
        performingJobs.store(0, std::memory_order_release);
    }
#endif
}

// Rest of the methods remain the same...
int ThreadPool::GetMaxThreads() {
#ifdef DISABLE_THREADPOOL
    return 0;
#else
    unsigned hc = std::thread::hardware_concurrency();

#ifdef USE_EMSCRIPTEN_PTHREADS

    hc = 8;

#endif // USE_EMSCRIPTEN_PTHREADS


    int reserve = 3;
    int hint = hc ? static_cast<int>(hc) : 0;
    int usable = hint > reserve ? (hint - reserve) : 1;
    return std::max(1, usable);
#endif
}

int ThreadPool::GetNumThreadsForPhysics() {
#ifdef DISABLE_THREADPOOL
    return 0;
#endif
    int mx = GetMaxThreads();
    int n = (mx * 7) / 10;
    return std::max(n, 1);
}

int ThreadPool::GetNumThreadsForAsyncUpdate() {
    int mx = GetMaxThreads();
    int n = (mx * 7) / 10;
    return std::max(n, 1);
}

int ThreadPool::GetNumThreadsForThreadPool() {
    int mx = GetMaxThreads();
    int n = (mx * 3) / 10;
    return std::max(n, 1);
}