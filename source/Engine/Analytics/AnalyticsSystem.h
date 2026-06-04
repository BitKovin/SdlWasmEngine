#pragma once

#include <string>
#include <stdexcept>
#include <unordered_map>
#include <json.hpp>

class AnalyticsSystem
{
public:
	AnalyticsSystem(){}
	~AnalyticsSystem(){}

	virtual void SendEvent(std::string eventName, std::string jsonData) {}

	void SendEvent(std::string eventName, std::unordered_map<std::string, std::string> data) 
	{

		nlohmann::json jsonData = {};

		for (auto& pair : data)
		{
			jsonData[pair.first] = pair.second;
		}

		SendEvent(eventName, jsonData.dump());

	}

	virtual void Tick(float GameTime) {}

	static AnalyticsSystem& Get()
	{
		if (MainInstance == nullptr)
		{
			throw std::runtime_error("AnalyticsSystem not initialized");
		}
		return *MainInstance;
	}

	static void Set(AnalyticsSystem* instance)
	{

		if (MainInstance)
		{
			delete (MainInstance);
		}

		MainInstance = instance;
	}

private:

	static inline AnalyticsSystem* MainInstance = nullptr;

};

#define ANALYTICS_SEND_EVENT(eventName, ...) \
    AnalyticsSystem::Get().SendEvent(eventName, __VA_ARGS__)