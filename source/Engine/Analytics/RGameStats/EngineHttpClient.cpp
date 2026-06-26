#include "EngineHttpClient.h"

#include <http/http_client.h>

#include <EngineMain.h>

void EngineHttpClient::SendRequest(const Request request) {

    // Capture 'request' by value so the lambda owns its own copy
    EngineMain::MainInstance->MainThreadPool->QueueJob([request]()
        {
            http_client::Request req;
            req.method = static_cast<http_client::Method>(request.method);
            req.url = request.url;
            req.headers = request.headers;
            req.body = request.body;
            req.timeout_ms = request.timeout_ms;

            http_client::RequestId reqId = http_client::start_request(req);

            return; // waiting for event only when debugging

            http_client::Response response;
            while (http_client::is_done(reqId) == false)
            {
                // Note: Consider adding a short sleep here or yielding 
                // to prevent 100% CPU usage on this thread while waiting.
            }

            http_client::get_response(reqId, response, true);
            printf("Request to %s completed with status %d, body: %s\n", request.url.c_str(), response.status_code, response.body.c_str());
        }
    );
}