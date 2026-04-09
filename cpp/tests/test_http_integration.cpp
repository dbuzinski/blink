#include "BlinkApp.hpp"
#include <gtest/gtest.h>

#include <arpa/inet.h>
#include <chrono>
#include <cstring>
#include <fstream>
#include <future>
#include <memory>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <unordered_map>
#include <unistd.h>

#include <filesystem>

namespace {

std::string tcpHttpExchange(const std::string& host, int port, std::string_view request) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        return {};
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(static_cast<uint16_t>(port));
    if (inet_pton(AF_INET, host.c_str(), &addr.sin_addr) != 1) {
        close(fd);
        return {};
    }

    if (connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) {
        close(fd);
        return {};
    }

    std::string req(request);
    if (send(fd, req.data(), req.size(), 0) != static_cast<ssize_t>(req.size())) {
        close(fd);
        return {};
    }

    std::string response;
    char buf[4096];
    for (;;) {
        ssize_t n = recv(fd, buf, sizeof(buf), 0);
        if (n <= 0) {
            break;
        }
        response.append(buf, static_cast<size_t>(n));
    }
    close(fd);
    return response;
}

class MockForGet404 : public MATLABCaller {
public:
    ResponseData callHandler(const std::string&, const RequestData&) override {
        ResponseData r;
        r.statusCode = 404;
        r.contentType = "text/plain";
        r.body = "nope";
        return r;
    }
};

class MockForPostEcho : public MATLABCaller {
public:
    std::string lastBody;

    ResponseData callHandler(const std::string&, const RequestData& request) override {
        lastBody = request.body;
        ResponseData r;
        r.statusCode = 200;
        r.contentType = "application/octet-stream";
        r.body = "ok";
        return r;
    }
};

class MockCapturesRequestHeaders : public MATLABCaller {
public:
    std::unordered_map<std::string, std::string> lastHeaders;

    ResponseData callHandler(const std::string&, const RequestData& request) override {
        lastHeaders = request.headers;
        ResponseData r;
        r.statusCode = 200;
        r.contentType = "text/plain";
        r.body = "ok";
        return r;
    }
};

class MockWithExtraHeaders : public MATLABCaller {
public:
    ResponseData callHandler(const std::string&, const RequestData&) override {
        ResponseData r;
        r.statusCode = 200;
        r.contentType = "text/plain";
        r.body = "ok";
        r.headers.push_back({"X-Blink-Test", "42"});
        return r;
    }
};

/// Used to ensure native 404/405 paths never invoke MATLAB.
class MockMustNotCall : public MATLABCaller {
public:
    ResponseData callHandler(const std::string&, const RequestData&) override {
        ADD_FAILURE() << "Handler must not be called for framework-generated 404/405";
        ResponseData r;
        r.statusCode = 500;
        r.contentType = "text/plain";
        r.body = "fail";
        return r;
    }
};

struct ServerReady {
    int port = 0;
    std::shared_ptr<BlinkApp> app;
};

/** Tries ports in range until beginListen succeeds; returns chosen port or -1. */
int tryBeginListen(BlinkApp& app) {
    for (int port = 32000; port < 32100; ++port) {
        try {
            app.beginListen(port);
            return port;
        } catch (const std::runtime_error&) {
            continue;
        }
    }
    return -1;
}

} // namespace

TEST(HttpIntegrationTest, AsyncMatlabPathUsesDefaultCallerAndStillResponds) {
    auto mock = std::make_shared<MockForGet404>();
    std::promise<ServerReady> readyPromise;
    auto readyFuture = readyPromise.get_future();

    std::thread server([&]() {
        try {
            auto app = std::make_shared<BlinkApp>(mock);
            app->setAsyncMatlabHandlers(true);
            app->addRoutes({{"GET", "/it-async", "h0"}});
            int port = tryBeginListen(*app);
            if (port < 0) {
                readyPromise.set_exception(std::make_exception_ptr(std::runtime_error("no free port")));
                return;
            }
            readyPromise.set_value(ServerReady{port, app});
            app->runEventLoop();
        } catch (...) {
            try {
                readyPromise.set_exception(std::current_exception());
            } catch (...) {
            }
        }
    });

    ServerReady ready;
    ASSERT_NO_THROW(ready = readyFuture.get());
    ASSERT_GE(ready.port, 32000);

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    std::string request =
        "GET /it-async HTTP/1.1\r\nHost: 127.0.0.1\r\nConnection: close\r\n\r\n";
    std::string response = tcpHttpExchange("127.0.0.1", ready.port, request);

    ASSERT_NE(response.find("HTTP/1.1 404 Not Found"), std::string::npos);
    ASSERT_NE(response.find("nope"), std::string::npos);

    ready.app->stop();
    server.join();
}

TEST(HttpIntegrationTest, GetReturnsStatusAndContentType) {
    auto mock = std::make_shared<MockForGet404>();
    std::promise<ServerReady> readyPromise;
    auto readyFuture = readyPromise.get_future();

    std::thread server([&]() {
        try {
            auto app = std::make_shared<BlinkApp>(mock);
            app->addRoutes({{"GET", "/it", "h0"}});
            int port = tryBeginListen(*app);
            if (port < 0) {
                readyPromise.set_exception(std::make_exception_ptr(std::runtime_error("no free port")));
                return;
            }
            readyPromise.set_value(ServerReady{port, app});
            app->runEventLoop();
        } catch (...) {
            try {
                readyPromise.set_exception(std::current_exception());
            } catch (...) {
            }
        }
    });

    ServerReady ready;
    ASSERT_NO_THROW(ready = readyFuture.get());
    ASSERT_GE(ready.port, 32000);
    ASSERT_TRUE(ready.app);

    // Allow the event loop thread to enter us_loop_run before we connect.
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    std::string request =
        "GET /it HTTP/1.1\r\nHost: 127.0.0.1\r\nConnection: close\r\n\r\n";
    std::string response = tcpHttpExchange("127.0.0.1", ready.port, request);

    ASSERT_NE(response.find("HTTP/1.1 404 Not Found"), std::string::npos);
    ASSERT_NE(response.find("Content-Type: text/plain"), std::string::npos);
    ASSERT_NE(response.find("nope"), std::string::npos);

    ready.app->stop();
    server.join();
}

TEST(HttpIntegrationTest, HandlerReceivesParsedRequestHeaders) {
    auto mock = std::make_shared<MockCapturesRequestHeaders>();
    std::promise<ServerReady> readyPromise;
    auto readyFuture = readyPromise.get_future();

    std::thread server([&]() {
        try {
            auto app = std::make_shared<BlinkApp>(mock);
            app->addRoutes({{"GET", "/hdr-in", "h0"}});
            int port = tryBeginListen(*app);
            if (port < 0) {
                readyPromise.set_exception(std::make_exception_ptr(std::runtime_error("no free port")));
                return;
            }
            readyPromise.set_value(ServerReady{port, app});
            app->runEventLoop();
        } catch (...) {
            try {
                readyPromise.set_exception(std::current_exception());
            } catch (...) {
            }
        }
    });

    ServerReady ready;
    ASSERT_NO_THROW(ready = readyFuture.get());
    ASSERT_GE(ready.port, 32000);

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    std::string request =
        "GET /hdr-in HTTP/1.1\r\n"
        "Host: 127.0.0.1\r\n"
        "X-Blink-Req: integration-test\r\n"
        "Connection: close\r\n\r\n";
    std::string response = tcpHttpExchange("127.0.0.1", ready.port, request);

    ASSERT_NE(response.find("HTTP/1.1 200 OK"), std::string::npos);
    auto& h = mock->lastHeaders;
    ASSERT_FALSE(h.empty());
    ASSERT_NE(h.find("x-blink-req"), h.end());
    EXPECT_EQ(h["x-blink-req"], "integration-test");
    ASSERT_NE(h.find("host"), h.end());
    EXPECT_EQ(h["host"], "127.0.0.1");

    ready.app->stop();
    server.join();
}

TEST(HttpIntegrationTest, GetReturnsCustomResponseHeaders) {
    auto mock = std::make_shared<MockWithExtraHeaders>();
    std::promise<ServerReady> readyPromise;
    auto readyFuture = readyPromise.get_future();

    std::thread server([&]() {
        try {
            auto app = std::make_shared<BlinkApp>(mock);
            app->addRoutes({{"GET", "/hdr", "h0"}});
            int port = tryBeginListen(*app);
            if (port < 0) {
                readyPromise.set_exception(std::make_exception_ptr(std::runtime_error("no free port")));
                return;
            }
            readyPromise.set_value(ServerReady{port, app});
            app->runEventLoop();
        } catch (...) {
            try {
                readyPromise.set_exception(std::current_exception());
            } catch (...) {
            }
        }
    });

    ServerReady ready;
    ASSERT_NO_THROW(ready = readyFuture.get());
    ASSERT_GE(ready.port, 32000);

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    std::string request =
        "GET /hdr HTTP/1.1\r\nHost: 127.0.0.1\r\nConnection: close\r\n\r\n";
    std::string response = tcpHttpExchange("127.0.0.1", ready.port, request);

    ASSERT_NE(response.find("HTTP/1.1 200 OK"), std::string::npos);
    ASSERT_NE(response.find("X-Blink-Test: 42"), std::string::npos);
    ASSERT_NE(response.find("Content-Type: text/plain"), std::string::npos);

    ready.app->stop();
    server.join();
}

TEST(HttpIntegrationTest, UnmatchedPathReturns404WithoutCallingHandler) {
    auto mock = std::make_shared<MockMustNotCall>();
    std::promise<ServerReady> readyPromise;
    auto readyFuture = readyPromise.get_future();

    std::thread server([&]() {
        try {
            auto app = std::make_shared<BlinkApp>(mock);
            app->addRoutes({{"GET", "/only-this", "h0"}});
            int port = tryBeginListen(*app);
            if (port < 0) {
                readyPromise.set_exception(std::make_exception_ptr(std::runtime_error("no free port")));
                return;
            }
            readyPromise.set_value(ServerReady{port, app});
            app->runEventLoop();
        } catch (...) {
            try {
                readyPromise.set_exception(std::current_exception());
            } catch (...) {
            }
        }
    });

    ServerReady ready;
    ASSERT_NO_THROW(ready = readyFuture.get());
    ASSERT_GE(ready.port, 32000);

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    std::string request =
        "GET /no-such-route HTTP/1.1\r\nHost: 127.0.0.1\r\nConnection: close\r\n\r\n";
    std::string response = tcpHttpExchange("127.0.0.1", ready.port, request);

    ASSERT_NE(response.find("HTTP/1.1 404 Not Found"), std::string::npos);
    ASSERT_NE(response.find("Not Found"), std::string::npos);

    ready.app->stop();
    server.join();
}

TEST(HttpIntegrationTest, WrongMethodReturns405WithAllowHeader) {
    auto mock = std::make_shared<MockMustNotCall>();
    std::promise<ServerReady> readyPromise;
    auto readyFuture = readyPromise.get_future();

    std::thread server([&]() {
        try {
            auto app = std::make_shared<BlinkApp>(mock);
            app->addRoutes({{"POST", "/echo", "h0"}});
            int port = tryBeginListen(*app);
            if (port < 0) {
                readyPromise.set_exception(std::make_exception_ptr(std::runtime_error("no free port")));
                return;
            }
            readyPromise.set_value(ServerReady{port, app});
            app->runEventLoop();
        } catch (...) {
            try {
                readyPromise.set_exception(std::current_exception());
            } catch (...) {
            }
        }
    });

    ServerReady ready;
    ASSERT_NO_THROW(ready = readyFuture.get());
    ASSERT_GE(ready.port, 32000);

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    std::string request =
        "GET /echo HTTP/1.1\r\nHost: 127.0.0.1\r\nConnection: close\r\n\r\n";
    std::string response = tcpHttpExchange("127.0.0.1", ready.port, request);

    ASSERT_NE(response.find("HTTP/1.1 405 Method Not Allowed"), std::string::npos);
    ASSERT_NE(response.find("Allow: POST"), std::string::npos);

    ready.app->stop();
    server.join();
}

TEST(HttpIntegrationTest, WrongMethodOnStaticMountReturns405) {
    auto mock = std::make_shared<MockMustNotCall>();
    std::promise<ServerReady> readyPromise;
    auto readyFuture = readyPromise.get_future();

    std::filesystem::path tmpDir =
        std::filesystem::temp_directory_path() / ("blink_static405_" + std::to_string(getpid()));
    std::error_code ec;
    std::filesystem::create_directories(tmpDir, ec);
    ASSERT_FALSE(ec);
    {
        std::ofstream f(tmpDir / "x.txt");
        ASSERT_TRUE(f) << "temp file";
        f << "ok";
    }

    std::thread server([&]() {
        try {
            auto app = std::make_shared<BlinkApp>(mock);
            app->addStaticFiles(tmpDir.string(), "/static");
            int port = tryBeginListen(*app);
            if (port < 0) {
                readyPromise.set_exception(std::make_exception_ptr(std::runtime_error("no free port")));
                return;
            }
            readyPromise.set_value(ServerReady{port, app});
            app->runEventLoop();
        } catch (...) {
            try {
                readyPromise.set_exception(std::current_exception());
            } catch (...) {
            }
        }
    });

    ServerReady ready;
    ASSERT_NO_THROW(ready = readyFuture.get());
    ASSERT_GE(ready.port, 32000);

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    std::string request =
        "DELETE /static/x.txt HTTP/1.1\r\nHost: 127.0.0.1\r\nConnection: close\r\n\r\n";
    std::string response = tcpHttpExchange("127.0.0.1", ready.port, request);

    ASSERT_NE(response.find("HTTP/1.1 405 Method Not Allowed"), std::string::npos);
    ASSERT_NE(response.find("Allow: GET"), std::string::npos);

    ready.app->stop();
    server.join();

    std::filesystem::remove_all(tmpDir, ec);
}

TEST(HttpIntegrationTest, PostBodyReachesHandler) {
    auto mock = std::make_shared<MockForPostEcho>();
    std::promise<ServerReady> readyPromise;
    auto readyFuture = readyPromise.get_future();

    std::thread server([&]() {
        try {
            auto app = std::make_shared<BlinkApp>(mock);
            app->addRoutes({{"POST", "/echo", "h0"}});
            int port = tryBeginListen(*app);
            if (port < 0) {
                readyPromise.set_exception(std::make_exception_ptr(std::runtime_error("no free port")));
                return;
            }
            readyPromise.set_value(ServerReady{port, app});
            app->runEventLoop();
        } catch (...) {
            try {
                readyPromise.set_exception(std::current_exception());
            } catch (...) {
            }
        }
    });

    ServerReady ready;
    ASSERT_NO_THROW(ready = readyFuture.get());
    ASSERT_GE(ready.port, 32000);

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    const char* payload = "hello-body";
    std::string request = std::string("POST /echo HTTP/1.1\r\nHost: 127.0.0.1\r\nConnection: "
                                      "close\r\nContent-Type: text/plain\r\nContent-Length: ")
        + std::to_string(std::strlen(payload)) + "\r\n\r\n" + payload;

    std::string response = tcpHttpExchange("127.0.0.1", ready.port, request);
    ASSERT_NE(response.find("HTTP/1.1 200 OK"), std::string::npos);
    EXPECT_EQ(mock->lastBody, "hello-body");

    ready.app->stop();
    server.join();
}
