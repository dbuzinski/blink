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
    int statusCode;
    std::string contentType;
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
    void listen(int port);
    
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
    
    // Dependencies
    std::unique_ptr<WebServer> webServer;
    std::shared_ptr<MATLABCaller> matlabCaller;
};