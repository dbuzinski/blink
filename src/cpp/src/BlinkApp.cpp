#include "BlinkApp.hpp"
#include "WebServer.hpp"
#include "RequestParser.hpp"
#include "RouteMatcher.hpp"
#include <fstream>
#include <filesystem>
#include <string>
#include <memory>
#include <vector>
#include <stdexcept>

BlinkApp::BlinkApp(std::shared_ptr<MATLABCaller> matlabCaller) 
    : webServer(std::make_unique<WebServer>()), matlabCaller(matlabCaller) {
}

BlinkApp::~BlinkApp() {
    // WebServer handles its own cleanup
}

void BlinkApp::addRoutes(const std::vector<Route>& routes) {
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
    webServer->get(staticRoute + "/*", [this, staticPath, staticRoute](auto *res, auto *req) {
        std::string url = std::string(req->getUrl());
        std::string filePath = staticPath + url.substr(staticRoute.length());
        serveStaticFile(filePath, res);
    });
}

void BlinkApp::listen(int port) {
    try {
        auto listenResult = webServer->listen(port, [port](auto *listen_socket) {
            if (!listen_socket) {
                throw std::runtime_error("Failed to listen on port " + std::to_string(port));
            }
        });
        listenResult.run();
    } catch (const std::exception& e) {
        throw std::runtime_error("Error in listen(): " + std::string(e.what()));
    } catch (...) {
        throw std::runtime_error("Unknown error in listen()");
    }
}

template<typename ResType, typename ReqType>
void BlinkApp::handleRequest(ResType* res, ReqType* req, const Route& route) {
    auto isAborted = std::make_shared<bool>(false);
    
    // Handle request body using onData callback
    res->onData([this, res, req, route, isAborted, bodyBuffer = (std::string*)nullptr](std::string_view chunk, bool isLast) mutable {
        if (isLast && !*isAborted) {
            if (bodyBuffer) {
                bodyBuffer->append(chunk);
                processRequest(res, req, route, bodyBuffer->c_str());
                delete bodyBuffer;
            } else {
                bodyBuffer = new std::string;
                bodyBuffer->append(chunk);
                processRequest(res, req, route, bodyBuffer->c_str());
                delete bodyBuffer;
            }
        } else {
            if (!bodyBuffer) {
                bodyBuffer = new std::string;
            }
            bodyBuffer->append(chunk);
        }
    });
    
    res->onAborted([isAborted]() {
        *isAborted = true;
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
    // Set content type if specified
    if (!response.contentType.empty()) {
        res->writeHeader("Content-Type", response.contentType);
    }
    
    // Send response with status code and body
    res->writeStatus(std::to_string(response.statusCode))->end(response.body);
}

template<typename ResType>
void BlinkApp::sendErrorResponse(ResType* res, int statusCode, const std::string& message) {
    res->writeStatus(std::to_string(statusCode))->end(message);
}