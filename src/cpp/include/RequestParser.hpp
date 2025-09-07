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
    std::string headers;
    std::string query;
    std::unordered_map<std::string, std::string> parameters;
};

class RequestParser {
public:
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
        
        // Extract headers (simplified - just get all headers as a string for now)
        // TODO: Parse individual headers if needed
        request.headers = ""; // Placeholder
        
        // Set body from the provided data
        request.body = bodyData ? std::string(bodyData) : "";
        
        return request;
    }
    
    // Parse query string into key-value pairs
    static std::unordered_map<std::string, std::string> parseQueryString(const std::string& query);
    
    // Extract path from URL (remove query string)
    static std::string extractPath(const std::string& url);
    
    // Extract query string from URL
    static std::string extractQueryString(const std::string& url);
    
private:
    // Helper to split string by delimiter
    static std::vector<std::string> split(const std::string& str, char delimiter);
};
