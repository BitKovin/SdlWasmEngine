#include "RGameStatsAnalyticsProvider.h"

#include <sstream>
#include <UUID.hpp>
#include <chrono>

// Helper function to safely escape standard string values for JSON
static std::string escapeJsonString(const std::string& input) {
    std::ostringstream ss;
    for (char c : input) {
        switch (c) {
        case '\\': ss << "\\\\"; break;
        case '"':  ss << "\\\""; break;
        case '\b': ss << "\\b"; break;
        case '\f': ss << "\\f"; break;
        case '\n': ss << "\\n"; break;
        case '\r': ss << "\\r"; break;
        case '\t': ss << "\\t"; break;
        default:
            // Filter out non-printable control characters if any exist
            if (c >= 0 && c < 32) {
                // Ignored or handled as needed
            }
            else {
                ss << c;
            }
            break;
        }
    }
    return ss.str();
}

// Serializes a single EventDto into a JSON object string
std::string RGameStatsAnalyticsProvider::toJson(const RGameStatsAnalyticsProvider::EventDto& event) {
    std::ostringstream ss;
    ss << "{";
    ss << "\"clientEventId\":\"" << escapeJsonString(event.clientEventId) << "\",";
    ss << "\"sessionId\":\"" << escapeJsonString(event.sessionId) << "\",";
    ss << "\"clientSequence\":" << event.clientSequence << ",";
    ss << "\"clientTimestamp\":\"" << escapeJsonString(event.clientTimestamp) << "\",";
    ss << "\"platform\":\"" << escapeJsonString(event.platform) << "\",";
    ss << "\"identityProvider\":\"" << escapeJsonString(event.identityProvider) << "\",";
    ss << "\"platformId\":\"" << escapeJsonString(event.platformId) << "\",";
    ss << "\"eventName\":\"" << escapeJsonString(event.eventName) << "\",";

    // Because eventData mimics a freeform JsonElement, we inject it directly 
    // without wrapping quotes if it already contains valid raw JSON.
    ss << "\"eventData\":";
    if (event.eventData.empty()) {
        ss << "null";
    }
    else {
        ss << event.eventData;
    }

    ss << "}";
    return ss.str();
}

// Serializes the entire request payload into a JSON POST request body string
std::string RGameStatsAnalyticsProvider::serializeIngestRequest(const RGameStatsAnalyticsProvider::IngestEventsRequest& request) {
    std::ostringstream ss;
    ss << "{";
    ss << "\"gameId\":\"" << escapeJsonString(request.gameId) << "\",";
    ss << "\"events\":[";

    for (size_t i = 0; i < request.events.size(); ++i) {
        ss << toJson(request.events[i]);
        if (i + 1 < request.events.size()) {
            ss << ",";
        }
    }

    ss << "]";
    ss << "}";
    return ss.str();
}

RGameStatsAnalyticsProvider::RGameStatsAnalyticsProvider()
{

	sessionId = UUID::generate_uuid();

}

void RGameStatsAnalyticsProvider::SendEvent(std::string eventName, std::string jsonData)
{

	AddEventToHistory(eventName, jsonData);

	IngestEventsRequest eventRequest;
	eventRequest.gameId = GameName;
    eventRequest.events = LastEvents;


	IAnalyticsHttpClient::Request request;
	request.url = EndpointUrl + "events";
	request.body = serializeIngestRequest(eventRequest);
	request.headers["Content-Type"] = "application/json";
	request.headers["accept"] = "*/*";
	request.method = IAnalyticsHttpClient::Method::POST;

	HtmlClient->SendRequest(request);

}

void RGameStatsAnalyticsProvider::Tick(float GameTime)
{

	if (LastHeartbeatTime == 0 || GameTime - LastHeartbeatTime >= HeartbeatInterval)
	{
		SendHeartbeat(GameTime);
	}

}

std::string RGameStatsAnalyticsProvider::get_current_utc_timestamp()
{
    using namespace std::chrono;

    const auto now = system_clock::now();
    const auto time = system_clock::to_time_t(now);

    std::tm utc_tm{};

#if defined(_WIN32)
    gmtime_s(&utc_tm, &time);
#else
    gmtime_r(&time, &utc_tm);
#endif

    std::ostringstream oss;
    oss << std::put_time(&utc_tm, "%Y-%m-%dT%H:%M:%SZ");

    return oss.str();
}

//history stores last 10 events
void RGameStatsAnalyticsProvider::AddEventToHistory(const std::string& eventName, const std::string& jsonData)
{

    EventDto event;
    event.clientEventId = UUID::generate_uuid();
    event.sessionId = sessionId;
    event.clientTimestamp = get_current_utc_timestamp();
    event.clientSequence = eventSequenceCounter++;
    event.eventName = eventName;
    event.eventData = jsonData;
	event.platform = Platform;
	event.identityProvider = IdentityProvider;
	event.platformId = PlatformId;

    LastEvents.push_back(event);
    if (LastEvents.size() > 10) {
        LastEvents.erase(LastEvents.begin());
    }
}

std::string RGameStatsAnalyticsProvider::generateSessionJson(
    const std::string& sessionId,
    const std::string& gameId,
    const std::string& platform,
    const std::string& identityProvider,
    const std::string& platformId,
    const std::vector<std::string>& recentEventIds
) {
    std::ostringstream ss;

    ss << "{";
    ss << "\"sessionId\":\"" << escapeJsonString(sessionId) << "\",";
    ss << "\"gameId\":\"" << escapeJsonString(gameId) << "\",";
    ss << "\"platform\":\"" << escapeJsonString(platform) << "\",";
    ss << "\"identityProvider\":\"" << escapeJsonString(identityProvider) << "\",";
    ss << "\"platformId\":\"" << escapeJsonString(platformId) << "\",";


    // Serialize the recentEventIds string array
    ss << "\"recentEventIds\":[";
    for (size_t i = 0; i < recentEventIds.size(); ++i) {
        ss << "\"" << escapeJsonString(recentEventIds[i]) << "\"";
        if (i + 1 < recentEventIds.size()) {
            ss << ",";
        }
    }
    ss << "]";

    ss << "}";
    return ss.str();
}

void RGameStatsAnalyticsProvider::SendHeartbeat(float GameTime)
{

	LastHeartbeatTime = GameTime;

	std::vector<std::string> recentEventIds;
	recentEventIds.reserve(LastEvents.size());
    for (auto& event : LastEvents)
    {
        recentEventIds.push_back(event.clientEventId);
    }

    IAnalyticsHttpClient::Request request;
    request.url = EndpointUrl + "sessions/heartbeat";
    request.body = generateSessionJson(sessionId, GameName, Platform, IdentityProvider, PlatformId, recentEventIds);
    request.headers["Content-Type"] = "application/json";
    request.headers["accept"] = "*/*";
    request.method = IAnalyticsHttpClient::Method::POST;

    HtmlClient->SendRequest(request);

}
