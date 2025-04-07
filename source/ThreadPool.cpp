#include "ThreadPool.h"

void ThreadPool::Start() {
#ifndef DISABLE_TREADPOOL
	const uint32_t num_threads = (std::thread::hardware_concurrency() - 1) / 1.5; // Max # of threads the system supports
	for (uint32_t ii = 0; ii < num_threads; ++ii)
	{
		threads.emplace_back(std::thread(&ThreadPool::ThreadLoop, this));
	}
#endif
}


void ThreadPool::ThreadLoop() {

#ifndef DISABLE_TREADPOOL

	while (true) {
		std::function<void()> job;
		{
			std::unique_lock<std::mutex> lock(queue_mutex);
			mutex_condition.wait(lock, [this] {
				return !jobs.empty() || should_terminate;
				});
			if (should_terminate) {
				return;
			}
			job = jobs.front();
			jobs.pop();
		}
		job();
	}

#endif // !DISABLE_TREADPOOL

}

void ThreadPool::QueueJob(const std::function<void()>& job)
{

#ifdef DISABLE_TREADPOOL
	job();
#else


	std::unique_lock<std::mutex> lock(queue_mutex);
	jobs.push(job);

	mutex_condition.notify_one();
#endif
}

bool ThreadPool::IsBusy() {
#ifdef DISABLE_TREADPOOL
	return false;

#else
	bool poolbusy;
	{
		std::unique_lock<std::mutex> lock(queue_mutex);
		poolbusy = !jobs.empty();
	}
	return poolbusy;
#endif
}

void ThreadPool::Stop()
{

#ifndef DISABLE_TREADPOOL

	{
		std::unique_lock<std::mutex> lock(queue_mutex);
		should_terminate = true;
	}
	mutex_condition.notify_all();
	for (std::thread& active_thread : threads) {
		active_thread.join();
	}
	threads.clear();

#endif

}