#pragma once

#include <string>

// Maps an HTTP status code to an HTTP/1.1 status line suffix (e.g. 404 -> "404 Not Found").
std::string httpStatusLine(int statusCode);
