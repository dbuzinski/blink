#include "WebServer.hpp"
#include "Loop.h"

WebServer::WebServer() : app_(std::make_unique<uWS::TemplatedApp<false>>()) {
}

WebServer::~WebServer() = default;

uWS::Loop* WebServer::getLoop() {
    return app_->getLoop();
}

void WebServer::stopListening() {
    if (listenSocket_ == nullptr || app_ == nullptr) {
        return;
    }
    us_listen_socket_t* listen_socket = listenSocket_;
    listenSocket_ = nullptr;
    uWS::Loop* loop = app_->getLoop();
    loop->defer([listen_socket]() {
        us_listen_socket_close(0, listen_socket);
    });
}

void WebServer::run() {
    app_->run();
}
