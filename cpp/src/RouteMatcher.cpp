#include "RouteMatcher.hpp"
#include <sstream>

std::unordered_map<std::string, std::string> RouteMatcher::parseRouteParameters(
    const std::string& routePath, 
    const std::string& actualPath
) {
    std::unordered_map<std::string, std::string> parameters;
    
    // First check if the paths actually match
    if (!matches(routePath, actualPath)) {
        return parameters; // Return empty parameters if paths don't match
    }
    
    // Split both paths into segments
    std::vector<std::string> routeSegments = splitPath(routePath);
    std::vector<std::string> actualSegments = splitPath(actualPath);
    
    // Match segments and extract parameters
    for (size_t i = 0; i < routeSegments.size(); ++i) {
        if (isParameter(routeSegments[i])) {
            // This is a parameter
            std::string paramName = extractParameterName(routeSegments[i]);
            parameters[paramName] = actualSegments[i];
        }
    }
    
    return parameters;
}

bool RouteMatcher::matches(const std::string& routePath, const std::string& actualPath) {
    std::vector<std::string> routeSegments = splitPath(routePath);
    std::vector<std::string> actualSegments = splitPath(actualPath);
    
    if (routeSegments.size() != actualSegments.size()) {
        return false;
    }
    
    for (size_t i = 0; i < routeSegments.size(); ++i) {
        if (!isParameter(routeSegments[i]) && routeSegments[i] != actualSegments[i]) {
            return false;
        }
    }
    
    return true;
}

std::vector<std::string> RouteMatcher::splitPath(const std::string& path) {
    return split(path, '/');
}

bool RouteMatcher::isParameter(const std::string& segment) {
    return !segment.empty() && segment[0] == ':';
}

std::string RouteMatcher::extractParameterName(const std::string& segment) {
    if (isParameter(segment)) {
        return segment.substr(1); // Remove the ':'
    }
    return "";
}

std::vector<std::string> RouteMatcher::split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    
    while (std::getline(ss, token, delimiter)) {
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    
    return tokens;
}
