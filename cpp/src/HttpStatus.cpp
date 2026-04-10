#include "HttpStatus.hpp"

std::string httpStatusLine(int status_code) {
    switch (status_code) {
        case 200: return "200 OK";
        case 201: return "201 Created";
        case 204: return "204 No Content";
        case 301: return "301 Moved Permanently";
        case 302: return "302 Found";
        case 304: return "304 Not Modified";
        case 400: return "400 Bad Request";
        case 401: return "401 Unauthorized";
        case 403: return "403 Forbidden";
        case 404: return "404 Not Found";
        case 405: return "405 Method Not Allowed";
        case 413: return "413 Payload Too Large";
        case 415: return "415 Unsupported Media Type";
        case 429: return "429 Too Many Requests";
        case 500: return "500 Internal Server Error";
        case 501: return "501 Not Implemented";
        case 503: return "503 Service Unavailable";
        default: return std::to_string(status_code) + " ";
    }
}
