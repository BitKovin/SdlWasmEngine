#pragma once

#include <stdint.h>

struct NetworkStat
{

	float packetLossPercent = 0.0f;
	float packetLossPercentVariance = 0.0f;
	uint64_t incomingBytesTotal = 0;
	uint64_t outgoingBytesTotal = 0;

	uint16_t roundTripTimeVariance = 0;
	uint16_t roundTripTime = 0; //time between sending packet and receiving confirmation

};