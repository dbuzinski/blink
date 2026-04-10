#pragma once
#include <string>
#include <memory>
#include <vector>
#include <functional>
#include <unordered_map>
#include <string_view>

// Forward declarations
class WebServer;
struct RequestData;
struct ResponseData;
struct Route;

// Include the data structures
#include "RequestParser.hpp"

struct ResponseData {
    std::string body;
    int status_code = 200;
    /// Primary Content-Type (MATLAB `Response.ContentType`). If empty, falls back to optional
    /// `Content-Type` in `headers` or `"text/html"` when sending.
    std::string content_type;
    /// Extra response headers (MATLAB `Response.Headers` struct → name/value pairs).
    std::vector<std::pair<std::string, std::string>> headers;
};

class MATLABCaller {
public:
    virtual ~MATLABCaller() = default;
    virtual ResponseData callHandler(const std::string& handlerName, const RequestData& request) = 0;

    /// Run the route handler and deliver the result by invoking `onComplete` on the **uWebSockets loop
    /// thread**. `deferToLoop` runs a functor on that thread (typically `uWS::Loop::defer`). Default
    /// implementation calls `callHandler` and then `onComplete` synchronously.
    virtual void callHandlerAsync(const std::string& handlerName,
        const RequestData& request,
        const std::function<void(std::function<void()>)>& deferToLoop,
        const std::function<void(ResponseData)>& onComplete);
};

struct Route {
    std::string http_method;
    std::string path;
    std::string handler_name;
};

class BlinkApp {
public:
    BlinkApp(std::shared_ptr<MATLABCaller> matlabCaller);
    ~BlinkApp();
    
    void addRoutes(const std::vector<Route>& routes);
    void addStaticFiles(const std::string& staticPath, const std::string& staticRoute);

    /// Binds and listens on `port`, then blocks until the event loop stops (see `stop()`).
    void listen(int port);

    /// Binds and listens without entering the event loop (for tests and embedding).
    void beginListen(int port);

    /// Runs the uWebSockets event loop (blocks until the loop exits).
    void runEventLoop();

    /// Closes the listen socket from another thread; used with `beginListen` + `runEventLoop`.
    void stop();

    /// When true, MATLAB work is scheduled off the event-loop thread and completion is posted with
    /// `uWS::Loop::defer` (see `MATLABEngineCaller` in `serve.cpp`). When false, `callHandler` runs
    /// synchronously on the loop thread (default; used by C++ tests).
    void setAsyncMatlabHandlers(bool enable);

    /// Maximum request body size accepted by the server (default: 10 MiB).
    void setMaxRequestBodyBytes(size_t bytes);

private:
    // Request handling
    template<typename ResType, typename ReqType>
    void handleRequest(ResType* res, ReqType* req, const Route& route);
    
    template<typename ResType, typename ReqType>
    void processRequest(ResType* res,
        ReqType* req,
        const Route& route,
        std::string_view body,
        const std::shared_ptr<bool>& cancelled);
    
    // Response handling
    template<typename ResType>
    void sendResponse(ResType* res, const ResponseData& response);
    
    template<typename ResType>
    void sendErrorResponse(ResType* res, int statusCode, const std::string& message);
    
    // Static file handling
    template<typename ResType>
    void serveStaticFile(const std::string& filePath, ResType* res);

    /// Ensure unmatched routes return a 404 response.
    void registerFallbackRoute();

    // Dependencies
    std::unique_ptr<WebServer> webServer_;
    std::shared_ptr<MATLABCaller> matlabCaller_;

    std::vector<Route> registeredRoutes_;
    bool asyncMatlabHandlers_ = false;

    size_t maxRequestBodyBytes_ = static_cast<size_t>(10) * 1024 * 1024;

    bool fallbackRegistered_ = false;
};