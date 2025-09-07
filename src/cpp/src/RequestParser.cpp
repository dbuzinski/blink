#include "RequestParser.hpp"
#include <sstream>
#include <iostream>
#include <vector>


std::unordered_map<std::string, std::string> RequestParser::parseQueryString(const std::string& query) {
    std::unordered_map<std::string, std::string> params;
    
    if (query.empty()) {
        return params;
    }
    
    std::istringstream stream(query);
    std::string pair;
    
    while (std::getline(stream, pair, '&')) {
        size_t pos = pair.find('=');
        if (pos != std::string::npos) {
            std::string key = pair.substr(0, pos);
            std::string value = pair.substr(pos + 1);
            params[key] = value;
        }
    }
    
    return params;
}

std::string RequestParser::extractPath(const std::string& url) {
    size_t queryPos = url.find('?');
    if (queryPos != std::string::npos) {
        return url.substr(0, queryPos);
    }
    return url;
}

std::string RequestParser::extractQueryString(const std::string& url) {
    size_t queryPos = url.find('?');
    if (queryPos != std::string::npos) {
        return url.substr(queryPos + 1);
    }
    return "";
}

std::vector<std::string> RequestParser::split(const std::string& str, char delimiter) {
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
