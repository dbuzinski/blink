#include "RequestParser.h"

std::unordered_map<std::string, std::string> parseQuery(const std::string_view queryString) {
    std::unordered_map<std::string, std::string> queryMap;
    std::string_view::size_type pos = 0;
    std::string_view::size_type prev = 0;

    while ((pos = queryString.find_first_of('&', prev)) != std::string_view::npos) {
        if (prev < pos) {
            std::string_view token = queryString.substr(prev, pos - prev);
            std::string_view::size_type equalPos = token.find_first_of('=');
            if (equalPos != std::string_view::npos) {
                std::string_view key = token.substr(0, equalPos);
                std::string_view value = token.substr(equalPos + 1);
                queryMap.insert({std::string(key), std::string(value)});
            }
        }
        prev = pos + 1;
    }

    if (prev < queryString.length()) {
        std::string_view token = queryString.substr(prev);
        std::string_view::size_type equalPos = token.find_first_of('=');
        if (equalPos != std::string_view::npos) {
            std::string_view key = token.substr(0, equalPos);
            std::string_view value = token.substr(equalPos + 1);
            queryMap.insert({std::string(key), std::string(value)});
        }
    }

    return queryMap;
};

std::unordered_map<std::string, std::string> parseParams(const std::string_view url, const std::string_view path) {
    std::unordered_map<std::string, std::string> paramsMap;
    std::string_view::size_type pos = 0;
    std::string_view::size_type prev = 0;

    while ((pos = path.find_first_of('/', prev)) != std::string_view::npos) {
        if (prev < pos) {
            std::string_view token = path.substr(prev, pos - prev);
            if (token[0] == ':') {
                std::string_view key = token.substr(1);
                std::string_view value = url.substr(prev);
                paramsMap.insert({std::string(key), std::string(value)});
            }
        }
        prev = pos + 1;
    }

    if (prev < path.length()) {
        std::string_view token = path.substr(prev);
        if (token[0] == ':') {
            std::string_view key = token.substr(1);
            std::string_view value = url.substr(prev);
            paramsMap.insert({std::string(key), std::string(value)});
        }
    }

    return paramsMap;
};
