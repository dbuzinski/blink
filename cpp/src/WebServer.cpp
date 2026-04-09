#include "WebServer.hpp"
#include "Loop.h"

extern "C" void us_listen_socket_close(int ssl, struct us_listen_socket_t* ls);

WebServer::WebServer() : app(std::make_unique<uWS::TemplatedApp<false>>()) {
}

WebServer::~WebServer() = default;

uWS::Loop* WebServer::getLoop() {
    return app->getLoop();
}

void WebServer::stopListening() {
    if (!listenSocket || !app) {
        return;
    }
    us_listen_socket_t* ls = listenSocket;
    listenSocket = nullptr;
    uWS::Loop* loop = app->getLoop();
    loop->defer([ls]() {
        us_listen_socket_close(0, ls);
    });
}

void WebServer::run() {
    app->run();
}
