#include "WebServer.hpp"
#include <iostream>

WebServer::WebServer() : app(std::make_unique<uWS::TemplatedApp<false>>()) {
}

WebServer::~WebServer() = default;

void WebServer::run() {
    app->run();
}
