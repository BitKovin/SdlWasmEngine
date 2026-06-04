#pragma once

#include <string>
#include <unordered_map>

class IAnalyticsHttpClient
{
public:
	IAnalyticsHttpClient(){}
	~IAnalyticsHttpClient(){}

    enum class Method {
        GET,
        POST,
        PUT,
        DELETE_,
        PATCH,
        HEAD,
        OPTIONS
    };

    struct Request {
        Method method = Method::GET;
        std::string url; // full URL including scheme, e.g. "https://example.com/path?x=1"
        std::unordered_map<std::string, std::string> headers;
        std::string body;    // optional request body (POST/PUT/PATCH/etc)
        int timeout_ms = 0;  // 0 -> library default / no explicit timeout
    };

	virtual void SendRequest(const Request request){}

private:

};
