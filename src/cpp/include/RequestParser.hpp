#pragma once
#include <string>
#include <unordered_map>
#include <vector>

// Forward declaration for uWebSockets request types
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
            const auto kv = *it;
            out[std::string(kv.first)] = std::string(kv.second);
        }
        return out;
    }

    // Parse a uWebSockets request into our RequestData structure
    template<typename ReqType>
    static RequestData parseRequest(ReqType* req, const char* bodyData) {
        RequestData request;
        
        // Extract basic request information
        request.method = std::string(req->getMethod());
        request.path = std::string(req->getUrl());
        
        // Extract query string and path
        request.query = extractQueryString(request.path);
        request.path = extractPath(request.path);
        
        request.headers = collectHeaders(req);
        
        // Set body from the provided data
        request.body = bodyData ? std::string(bodyData) : "";
        
        return request;
    }
    
    // Parse query string into key-value pairs
    static std::unordered_map<std::string, std::string> parseQueryString(const std::string& query);

    /// Map an HTTP query parameter name (or lowercased request header name) to a valid MATLAB struct field name.
    static std::string queryKeyToMatlabFieldName(const std::string& key);

    /// Map a MATLAB struct field name from `Response.Headers` to an HTTP header name (underscores → hyphens).
    static std::string matlabFieldNameToHttpHeaderName(const std::string& field);
    
    // Extract path from URL (remove query string)
    static std::string extractPath(const std::string& url);
    
    // Extract query string from URL
    static std::string extractQueryString(const std::string& url);
    
private:
    // Helper to split string by delimiter
    static std::vector<std::string> split(const std::string& str, char delimiter);
};
