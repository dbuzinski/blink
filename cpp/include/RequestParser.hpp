#pragma once
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

// Forward declaration for uWebSockets request types
// NOLINTNEXTLINE(readability-identifier-naming)
namespace uWS {
    struct HttpRequest;
}

struct RequestData {
    std::string method;
    std::string path;
    std::string body;
    /// Raw header names as produced by uWebSockets (typically lowercased); values as sent on the wire.
    std::unordered_map<std::string, std::string> headers;
    std::string query;
    std::unordered_map<std::string, std::string> parameters;
};

class RequestParser {
public:
    /// Collect all HTTP headers from a uWebSockets `HttpRequest` (or compatible `begin`/`end`).
    template<typename ReqType>
    static std::unordered_map<std::string, std::string> collectHeaders(ReqType* req) {
        std::unordered_map<std::string, std::string> out;
        for (auto it = req->begin(); it != req->end(); ++it) {
            const auto key_value = *it;
            out[std::string(key_value.first)] = std::string(key_value.second);
        }
        return out;
    }

    // Parse a uWebSockets request into our RequestData structure
    template<typename ReqType>
    static RequestData parseRequest(ReqType* req, std::string_view body) {
        RequestData request;
        
        // Extract basic request information
        request.method = std::string(req->getMethod());
        request.path = std::string(req->getUrl());
        request.query = std::string(req->getQuery());
        
        request.headers = collectHeaders(req);
        
        // Set body from the provided data (explicit length; safe for binary / embedded NULs)
        request.body.assign(body.data(), body.size());
        
        return request;
    }

    /// Extract route parameters using uWebSockets' already-matched parameters.
    /// Parameter names are derived from the route pattern (segments beginning with ':') and values
    /// are read from `req->getParameter(i)` in left-to-right order.
    template<typename ReqType>
    static std::unordered_map<std::string, std::string> collectRouteParameters(
        ReqType* req,
        const std::string& routePattern
    ) {
        std::unordered_map<std::string, std::string> out;
        size_t param_index = 0;

        size_t route_idx = 0;
        while (route_idx < routePattern.size()) {
            // Skip leading slashes
            while (route_idx < routePattern.size() && routePattern[route_idx] == '/') {
                ++route_idx;
            }
            if (route_idx >= routePattern.size()) {
                break;
            }

            const size_t seg_start = route_idx;
            while (route_idx < routePattern.size() && routePattern[route_idx] != '/') {
                ++route_idx;
            }
            const size_t seg_end = route_idx;

            if (seg_end > seg_start && routePattern[seg_start] == ':') {
                const std::string name = routePattern.substr(seg_start + 1, seg_end - (seg_start + 1));
                const std::string_view value_view = req->getParameter(static_cast<unsigned int>(param_index));
                out[name] = std::string(value_view);
                ++param_index;
            }
        }

        return out;
    }
    
    // Parse query string into key-value pairs
    static std::unordered_map<std::string, std::string> parseQueryString(const std::string& query);

    /// Map an HTTP query parameter name (or lowercased request header name) to a valid MATLAB struct field name.
    static std::string queryKeyToMatlabFieldName(const std::string& key);

    /// Map a MATLAB struct field name from `Response.Headers` to an HTTP header name (underscores → hyphens).
    static std::string matlabFieldNameToHttpHeaderName(const std::string& field);
};
