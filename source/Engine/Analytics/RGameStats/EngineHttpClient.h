#pragma once

#include "IAnalyticsHttpClient.h"

class EngineHttpClient : public IAnalyticsHttpClient
{
public:
	EngineHttpClient(){ }
	~EngineHttpClient(){}

	void SendRequest(const Request request) override;

private:

};

