#include "RequestParser.hpp"
#include <cctype>
#include <sstream>
#include <iostream>
#include <vector>

namespace {

int fromHex(unsigned char chr) {
    if (chr >= '0' && chr <= '9') {
        return chr - '0';
    }
    if (chr >= 'a' && chr <= 'f') {
        return 10 + (chr - 'a');
    }
    if (chr >= 'A' && chr <= 'F') {
        return 10 + (chr - 'A');
    }
    return -1;
}

std::string percentDecode(std::string_view input) {
    std::string out;
    out.reserve(input.size());

    for (size_t i = 0; i < input.size(); ++i) {
        auto chr = static_cast<unsigned char>(input[i]);
        if (chr == '+') {
            out.push_back(' ');
            continue;
        }
        if (chr == '%' && i + 2 < input.size()) {
            int high_nibble = fromHex(static_cast<unsigned char>(input[i + 1]));
            int low_nibble = fromHex(static_cast<unsigned char>(input[i + 2]));
            if (high_nibble >= 0 && low_nibble >= 0) {
                out.push_back(static_cast<char>((high_nibble << 4) | low_nibble));
                i += 2;
                continue;
            }
        }
        out.push_back(static_cast<char>(chr));
    }

    return out;
}

} // namespace


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
            const std::string key_raw = pair.substr(0, pos);
            const std::string value_raw = pair.substr(pos + 1);
            params[percentDecode(key_raw)] = percentDecode(value_raw);
        }
    }
    
    return params;
}

std::string RequestParser::matlabFieldNameToHttpHeaderName(const std::string& field) {
    std::string out;
    out.reserve(field.size());
    for (unsigned char chr : field) {
        if (chr == '_') {
            out += '-';
        } else {
            out += static_cast<char>(chr);
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
    for (unsigned char chr : key) {
        if (std::isalnum(chr) != 0) {
            out += static_cast<char>(chr);
        } else {
            out += '_';
        }
    }
    if (std::isalpha(static_cast<unsigned char>(out[0])) == 0) {
        out.insert(out.begin(), 'x');
    }
    return out;
}
