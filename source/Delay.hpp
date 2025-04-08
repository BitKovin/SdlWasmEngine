#pragma once

#include "Time.hpp"

class Delay
{
public:
	Delay()
	{

	}

	Delay(double delay)
	{
		AddDelay(delay);
	}

	~Delay() 
	{

	}

	double waitUntilTime = -1000;

	bool ignorePause = false;

	bool Wait()
	{
		double time = Time::GameTime;

		if (ignorePause)
		{
			time = Time::GameTimeNoPause;
		}

		return waitUntilTime >= time;

	}

	void AddDelay(double delay)
	{
		double time = Time::GameTime;

		if (ignorePause)
		{
			time = Time::GameTimeNoPause;
		}

		waitUntilTime = time + delay;

	}

	double GetRemainTime()
	{
		double time = Time::GameTime;

		if (ignorePause)
		{
			time = Time::GameTimeNoPause;
		}

		return waitUntilTime - time;

	}

private:

};