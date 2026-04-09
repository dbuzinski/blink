#pragma once
#include <string>
#include <memory>
#include <vector>
#include <functional>
#include <unordered_map>

// Forward declarations
class WebServer;
struct RequestData;
struct ResponseData;
struct Route;

// Include the data structures
#include "RequestParser.hpp"

struct ResponseData {
    std::string body;
    int statusCode = 200;
    /// Primary Content-Type (MATLAB `Response.ContentType`). If empty, falls back to optional
    /// `Content-Type` in `headers` or `"text/html"` when sending.
    std::string contentType;
    /// Extra response headers (MATLAB `Response.Headers` struct → name/value pairs).
    std::vector<std::pair<std::string, std::string>> headers;
};

class MATLABCaller {
public:
    virtual ~MATLABCaller() = default;
    virtual ResponseData callHandler(const std::string& handlerName, const RequestData& request) = 0;
};

struct Route {
    std::string httpMethod;
    std::string path;
    std::string handlerName;
};

class BlinkApp {
public:
    BlinkApp(std::shared_ptr<MATLABCaller> matlabCaller);
    ~BlinkApp();
    
    void addRoutes(const std::vector<Route>& routes);
    void addStaticFiles(const std::string& staticPath, const std::string& staticRoute);

    /// When true (default), requests that match a registered path (including static mounts) but use
    /// a non-allowed HTTP method receive 405 Method Not Allowed and an Allow header. When false,
    /// those requests receive 404 Not Found.
    void setTreatWrongMethodAs405(bool enable);

    /// Binds and listens on `port`, then blocks until the event loop stops (see `stop()`).
    void listen(int port);

    /// Binds and listens without entering the event loop (for tests and embedding).
    void beginListen(int port);

    /// Runs the uWebSockets event loop (blocks until the loop exits).
    void runEventLoop();

    /// Closes the listen socket from another thread; used with `beginListen` + `runEventLoop`.
    void stop();

private:
    // Request handling
    template<typename ResType, typename ReqType>
    void handleRequest(ResType* res, ReqType* req, const Route& route);
    
    template<typename ResType, typename ReqType>
    void processRequest(ResType* res, ReqType* req, const Route& route, const char* bodyData);
    
    // Response handling
    template<typename ResType>
    void sendResponse(ResType* res, const ResponseData& response);
    
    template<typename ResType>
    void sendErrorResponse(ResType* res, int statusCode, const std::string& message);
    
    // Static file handling
    void serveStaticFile(const std::string& filePath, auto* res);

    void registerFallbackRoute();

    std::vector<std::string> allowedMethodsForPath(const std::string& path) const;

    static bool pathUnderStaticPrefix(const std::string& path, const std::string& staticRoute);

    // Dependencies
    std::unique_ptr<WebServer> webServer;
    std::shared_ptr<MATLABCaller> matlabCaller;

    std::vector<Route> registeredRoutes_;
    std::vector<std::string> staticRoutePrefixes_;
    bool fallbackRegistered_ = false;
    bool treatWrongMethodAs405_ = true;
};