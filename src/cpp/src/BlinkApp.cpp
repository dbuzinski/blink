#include "BlinkApp.hpp"
#include "WebServer.hpp"
#include "RequestParser.hpp"
#include "RouteMatcher.hpp"
#include "HttpStatus.hpp"
#include <algorithm>
#include <fstream>
#include <filesystem>
#include <string>
#include <memory>
#include <vector>
#include <stdexcept>
#include <cctype>

namespace {

bool headerNameEqualsIgnoreCase(const std::string& a, const std::string& b) {
    if (a.size() != b.size()) {
        return false;
    }
    for (size_t i = 0; i < a.size(); ++i) {
        if (std::tolower(static_cast<unsigned char>(a[i])) != std::tolower(static_cast<unsigned char>(b[i]))) {
            return false;
        }
    }
    return true;
}

} // namespace

BlinkApp::BlinkApp(std::shared_ptr<MATLABCaller> matlabCaller) 
    : webServer(std::make_unique<WebServer>()), matlabCaller(matlabCaller) {
}

BlinkApp::~BlinkApp() {
    // WebServer handles its own cleanup
}

void BlinkApp::addRoutes(const std::vector<Route>& routes) {
    registeredRoutes_.insert(registeredRoutes_.end(), routes.begin(), routes.end());
    for (const auto& route : routes) {
        if (route.httpMethod == "GET") {
            webServer->get(route.path, [this, route](auto *res, auto *req) {
                handleRequest(res, req, route);
            });
        } else if (route.httpMethod == "POST") {
            webServer->post(route.path, [this, route](auto *res, auto *req) {
                handleRequest(res, req, route);
            });
        } else if (route.httpMethod == "PUT") {
            webServer->put(route.path, [this, route](auto *res, auto *req) {
                handleRequest(res, req, route);
            });
        } else if (route.httpMethod == "DELETE") {
            webServer->del(route.path, [this, route](auto *res, auto *req) {
                handleRequest(res, req, route);
            });
        }
    }
}

void BlinkApp::addStaticFiles(const std::string& staticPath, const std::string& staticRoute) {
    staticRoutePrefixes_.push_back(staticRoute);
    webServer->get(staticRoute + "/*", [this, staticPath, staticRoute](auto *res, auto *req) {
        std::string url = std::string(req->getUrl());
        std::string filePath = staticPath + url.substr(staticRoute.length());
        serveStaticFile(filePath, res);
    });
}

void BlinkApp::setTreatWrongMethodAs405(bool enable) {
    treatWrongMethodAs405_ = enable;
}

bool BlinkApp::pathUnderStaticPrefix(const std::string& path, const std::string& staticRoute) {
    if (staticRoute.empty()) {
        return false;
    }
    if (path == staticRoute) {
        return true;
    }
    return path.size() > staticRoute.size() && path.compare(0, staticRoute.size(), staticRoute) == 0
        && path[staticRoute.size()] == '/';
}

std::vector<std::string> BlinkApp::allowedMethodsForPath(const std::string& path) const {
    std::vector<std::string> methods;
    for (const auto& route : registeredRoutes_) {
        if (RouteMatcher::matches(route.path, path)) {
            methods.push_back(route.httpMethod);
        }
    }
    for (const std::string& prefix : staticRoutePrefixes_) {
        if (pathUnderStaticPrefix(path, prefix)) {
            methods.push_back("GET");
            break;
        }
    }
    std::sort(methods.begin(), methods.end());
    methods.erase(std::unique(methods.begin(), methods.end()), methods.end());
    return methods;
}

void BlinkApp::registerFallbackRoute() {
    if (fallbackRegistered_) {
        return;
    }
    fallbackRegistered_ = true;

    webServer->any("/*", [this](auto* res, auto* req) {
        const std::string path = RequestParser::extractPath(std::string(req->getUrl()));
        const std::string method = std::string(req->getMethod());
        const std::vector<std::string> allowed = allowedMethodsForPath(path);

        if (allowed.empty()) {
            sendErrorResponse(res, 404, "Not Found");
            return;
        }

        const bool methodOk = std::find(allowed.begin(), allowed.end(), method) != allowed.end();
        if (methodOk) {
            // Defensive: path matches a route but uWebSockets did not dispatch to it (e.g. pattern edge case).
            sendErrorResponse(res, 404, "Not Found");
            return;
        }

        if (treatWrongMethodAs405_) {
            std::string allowHeader;
            for (size_t i = 0; i < allowed.size(); ++i) {
                if (i > 0) {
                    allowHeader += ", ";
                }
                allowHeader += allowed[i];
            }
            res->writeStatus(httpStatusLine(405));
            res->writeHeader("Allow", allowHeader);
            res->writeHeader("Content-Type", "text/plain; charset=utf-8");
            res->end("Method Not Allowed");
            return;
        }

        sendErrorResponse(res, 404, "Not Found");
    });
}

void BlinkApp::beginListen(int port) {
    registerFallbackRoute();
    bool failedToListen = false;
    webServer->listen(port, [&failedToListen](auto* listen_socket) {
        failedToListen = (listen_socket == nullptr);
    });

    if (failedToListen) {
        throw std::runtime_error("Failed to listen on port " + std::to_string(port));
    }
}

void BlinkApp::runEventLoop() {
    webServer->run();
}

void BlinkApp::stop() {
    webServer->stopListening();
}

void BlinkApp::listen(int port) {
    beginListen(port);
    runEventLoop();
}

template<typename ResType, typename ReqType>
void BlinkApp::handleRequest(ResType* res, ReqType* req, const Route& route) {
    auto isAborted = std::make_shared<bool>(false);
    auto bodyBuffer = std::make_shared<std::string>();
    
    // Handle request body using onData callback
    res->onData([this, res, req, route, isAborted, bodyBuffer](std::string_view chunk, bool isLast) mutable {
        if (*isAborted) {
            return;
        }

        bodyBuffer->append(chunk);

        if (isLast) {
            processRequest(res, req, route, bodyBuffer->c_str());
        }
    });
    
    res->onAborted([isAborted, bodyBuffer]() {
        *isAborted = true;
        bodyBuffer->clear();
    });
}


void BlinkApp::serveStaticFile(const std::string& filePath, auto* res) {
    if (std::filesystem::exists(filePath)) {
        std::ifstream file(filePath, std::ios::binary);
        if (file) {
            std::string content((std::istreambuf_iterator<char>(file)),
                              std::istreambuf_iterator<char>());
            res->end(content);
        } else {
            res->writeStatus("404 Not Found")->end("File not found");
        }
    } else {
        res->writeStatus("404 Not Found")->end("File not found");
    }
}

template<typename ResType, typename ReqType>
void BlinkApp::processRequest(ResType* res, ReqType* req, const Route& route, const char* bodyData) {
    try {
        // Parse request data from uWebSockets request
        RequestData request = RequestParser::parseRequest(req, bodyData);
        
        // Extract route parameters
        request.parameters = RouteMatcher::parseRouteParameters(route.path, request.path);
        
        // Call MATLAB handler through the caller interface
        ResponseData response = matlabCaller->callHandler(route.handlerName, request);
        
        // Send response back to client
        sendResponse(res, response);
        
    } catch (const std::exception& e) {
        sendErrorResponse(res, 500, "Internal Server Error");
    } catch (...) {
        sendErrorResponse(res, 500, "Internal Server Error");
    }
}

template<typename ResType>
void BlinkApp::sendResponse(ResType* res, const ResponseData& response) {
    // Status must be written before headers
    res->writeStatus(httpStatusLine(response.statusCode));

    bool wroteContentType = false;
    if (!response.contentType.empty()) {
        res->writeHeader("Content-Type", response.contentType);
        wroteContentType = true;
    }

    for (const auto& [name, value] : response.headers) {
        if (wroteContentType && headerNameEqualsIgnoreCase(name, "Content-Type")) {
            continue;
        }
        res->writeHeader(name, value);
        if (headerNameEqualsIgnoreCase(name, "Content-Type")) {
            wroteContentType = true;
        }
    }

    if (!wroteContentType) {
        res->writeHeader("Content-Type", "text/html");
    }

    res->end(response.body);
}

template<typename ResType>
void BlinkApp::sendErrorResponse(ResType* res, int statusCode, const std::string& message) {
    res->writeStatus(httpStatusLine(statusCode))->end(message);
}