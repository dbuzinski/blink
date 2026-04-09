#include "RequestParser.hpp"
#include <cctype>
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

std::string RequestParser::matlabFieldNameToHttpHeaderName(const std::string& field) {
    std::string out;
    out.reserve(field.size());
    for (unsigned char c : field) {
        if (c == '_') {
            out += '-';
        } else {
            out += static_cast<char>(c);
        }
    }
    return out;
}

std::string RequestParser::queryKeyToMatlabFieldName(const std::string& key) {
    if (key.empty()) {
        return "empty";
    }
    std::string out;
    out.reserve(key.size());
    for (unsigned char c : key) {
        if (std::isalnum(c)) {
            out += static_cast<char>(c);
        } else if (c == '_') {
            out += '_';
        } else {
            out += '_';
        }
    }
    if (!std::isalpha(static_cast<unsigned char>(out[0]))) {
        out.insert(out.begin(), 'x');
    }
    return out;
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
