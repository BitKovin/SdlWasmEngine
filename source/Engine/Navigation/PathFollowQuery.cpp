#include "PathFollowQuery.h"

#include "../EngineMain.h"

#include "Navigation.hpp"
#include "../DebugDraw.hpp"

#include "../Physics.h"
#include "../RandomHelper.h"

PathFollowQuery::PathFollowQuery()
{
}

PathFollowQuery::~PathFollowQuery()//since it's very rate that enemy will die and just get destroyed at the same time
{

	Canceled = true;

	while (Performing)
	{

	}
}

void PathFollowQuery::TryPerform()
{

	if (Performing) return;


	if (isPerformingDelay.Wait()) return;


	if (Canceled) return;

	Performing = true;

	if (Async)
	{
		EngineMain::MainInstance->MainThreadPool->QueueJob([this]() {this->CalculatePathOnThread(); });
	}
	else
	{
		CalculatePathOnThread();
	}

	if (ThreadPool::Supported() == false)
	{
		isPerformingDelay.AddDelay(distance(desiredStart, desiredTarget) / 300.0f + 0.04 + RandomHelper::RandomFloat() / 20.0f);
	}
	else
	{
        isPerformingDelay.AddDelay(std::min(distance(desiredStart, desiredTarget) / 300.0f,0.4f) + 0.05 + RandomHelper::RandomFloat() / 15.0f);
	}

}

void PathFollowQuery::CalculatePathOnThread()
{

	targetLocationsMutex.lock();

	vec3 s, t;


	s = desiredStart;
	t = desiredTarget;


	if (Canceled)
	{
		Performing = false;

		//Logger::Log("I have HUGE doubt that it will ever trigger, so I print text to see if it ever happens. \n");

		targetLocationsMutex.unlock();
		return;
	}

    auto path = NavigationSystem::FindSimplePath(s, t, acceptanceRadius, &reachedTarget, allowPartialPath);

    if (path.empty())
    {
        FoundTarget = false;
        Performing = false;
        targetLocationsMutex.unlock();
        CalculatedPath = true;
        return;
    }

    const float removeWithinDist = 3.0f;
    const float removeWithinDist2 = removeWithinDist * removeWithinDist;
    const float traceRadius = 0.3f;
    const vec3 traceUpOffset = vec3(0.0f, 0.5f, 0.0f);
    const float minDirLen2 = 1e-6f;

#ifdef DISABLE_TREADPOOL

    const int maxSimplifyIterations = 1;  // safety cap

#else

    const int maxSimplifyIterations = 2;  // safety cap

#endif // DISABLE_TREADPOOL




    int iterationCount = 0;

    // Simplify leading points if close and visible
    while (path.size() > 1 && iterationCount < maxSimplifyIterations)
    {
        ++iterationCount;

        // Remove redundant first point if it's basically the start
        if (distance2(s, path[0]) < 1e-4f)
        {
            path.erase(path.begin());
            continue;
        }

        float d2 = distance2(s, path[0]);
        if (d2 <= removeWithinDist2)
        {
            vec3 traceStart = s + traceUpOffset;
            vec3 traceEnd = path[1] + traceUpOffset;

            auto res = Physics::SphereTrace(traceStart, traceEnd, traceRadius, BodyType::World | BodyType::MainBody);
            if (!res.hasHit)
            {
                // Point visible — remove it and continue simplifying
                path.erase(path.begin());
                continue;
            }
        }

        // No removal — path seems fine
        break;
    }


    // Compute direction safely
    if (!path.empty())
    {
        CalculatedTargetLocation = path[0];
        FoundTarget = true;
    }
    else
    {
        FoundTarget = false;
    }

    CurrentPath = std::vector<vec3>(path);

    Performing = false;
    CalculatedPath = true;
    targetLocationsMutex.unlock();
	
}

void PathFollowQuery::UpdateStartAndTarget(vec3 start, vec3 target)
{
	targetLocationsMutex.lock();
	desiredStart = start;
	desiredTarget = target;
	targetLocationsMutex.unlock();
}

void PathFollowQuery::WaitToFinish()
{

    while (Performing)
    {

    }

}
