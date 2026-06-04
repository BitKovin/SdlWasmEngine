#pragma once

#include "../AnalyticsSystem.h"
#include "IAnalyticsHttpClient.h"

class RGameStatsAnalyticsProvider : public AnalyticsSystem
{
public:
    RGameStatsAnalyticsProvider();
	~RGameStatsAnalyticsProvider(){}

	IAnalyticsHttpClient* HtmlClient = nullptr;


    void SendEvent(std::string eventName, std::string jsonData) override;
	void Tick(float GameTime) override;

    std::string get_current_utc_timestamp();

	std::string GameName = "test_game";
	std::string Platform = "testing_platform";
	std::string IdentityProvider = "testing_identity_provider";
	std::string PlatformId = "";

	std::string EndpointUrl = "http://localhost:50440/api/v1/";

private:

    // Individual Event Data Transfer Object
    struct EventDto {
        std::string clientEventId;     // UUID string
        std::string sessionId;         // UUID string
        int clientSequence = 0;        // Monotonically increasing counter
        std::string clientTimestamp;   // ISO 8601 UTC timestamp string (e.g., "2026-06-04T13:36:09.217Z")
        std::string platform;          // e.g., "steam"
        std::string identityProvider;  // e.g., "steam"
        std::string platformId;        // e.g., "76561198012345678"
        std::string eventName;         // e.g., "player_died"

        // Represents arbitrary JSON payload (matching C#'s JsonElement?).
        // Pass valid raw JSON text here (e.g., "{\"hp\": 0, \"killer\": \"boss\"}").
        // If empty, it will serialize to JSON null.
        std::string eventData;
    };

    // Root Request Structure
    struct IngestEventsRequest {
        std::string gameId;
        std::vector<EventDto> events;
    };

	std::vector<EventDto> LastEvents;

	std::string sessionId; // UUID string

	float LastHeartbeatTime = 0;

	float HeartbeatInterval = 10; // seconds

	int eventSequenceCounter = 0;

	void AddEventToHistory(const std::string& eventName, const std::string& jsonData);

    std::string generateSessionJson(const std::string& sessionId, const std::string& gameId, const std::string& platform, const std::string& identityProvider, const std::string& platformId, const std::vector<std::string>& recentEventIds);

	void SendHeartbeat(float GameTime);

    std::string toJson(const EventDto& event);

    std::string serializeIngestRequest(const RGameStatsAnalyticsProvider::IngestEventsRequest& request);

};

