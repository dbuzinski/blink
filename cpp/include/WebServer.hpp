#pragma once
#include <string>
#include <memory>
#include <functional>
#include "App.h"

struct us_listen_socket_t;

namespace uWS {
struct Loop;
}

class WebServer {
public:
    WebServer();
    ~WebServer();
    
    // HTTP method handlers
    template<typename Handler>
    void get(const std::string& path, Handler&& handler) {
        app->get(path, std::forward<Handler>(handler));
    }
    
    template<typename Handler>
    void post(const std::string& path, Handler&& handler) {
        app->post(path, std::forward<Handler>(handler));
    }
    
    template<typename Handler>
    void put(const std::string& path, Handler&& handler) {
        app->put(path, std::forward<Handler>(handler));
    }
    
    template<typename Handler>
    void del(const std::string& path, Handler&& handler) {
        app->del(path, std::forward<Handler>(handler));
    }

    /// Registers a handler for any HTTP method on `pattern` (uWebSockets `onHttp("*", ...)`).
    template<typename Handler>
    void any(const std::string& path, Handler&& handler) {
        app->any(path, std::forward<Handler>(handler));
    }
    
    // Server control
    template<typename Handler>
    auto listen(int port, Handler&& handler)
        -> decltype(std::declval<uWS::TemplatedApp<false>&>().listen(port, std::forward<Handler>(handler))) {
        return app->listen(port, [this, h = std::forward<Handler>(handler)](us_listen_socket_t* ls) mutable {
            listenSocket = ls;
            h(ls);
        });
    }

    uWS::Loop* getLoop();

    /// Schedules closing the listen socket on the loop thread (safe from another thread).
    void stopListening();

    void run();

private:
    std::unique_ptr<uWS::TemplatedApp<false>> app;
    us_listen_socket_t* listenSocket = nullptr;
};
