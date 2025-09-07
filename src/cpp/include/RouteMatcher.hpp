#pragma once
#include <string>
#include <unordered_map>
#include <vector>

class RouteMatcher {
public:
    // Extract route parameters from a route pattern and actual path
    static std::unordered_map<std::string, std::string> parseRouteParameters(
        const std::string& routePath, 
        const std::string& actualPath
    );
    
    // Check if a route pattern matches an actual path
    static bool matches(const std::string& routePath, const std::string& actualPath);
    
    // Split a path into segments
    static std::vector<std::string> splitPath(const std::string& path);
    
    // Check if a segment is a parameter (starts with ':')
    static bool isParameter(const std::string& segment);
    
    // Extract parameter name from a parameter segment
    static std::string extractParameterName(const std::string& segment);
    
private:
    // Helper to split string by delimiter
    static std::vector<std::string> split(const std::string& str, char delimiter);
};
