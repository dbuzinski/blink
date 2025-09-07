#include "mex.hpp"
#include "mexAdapter.hpp"
#include "BlinkApp.hpp"
#include <string>
#include <vector>
#include <unordered_map>

// MATLAB caller implementation that uses MATLAB Engine directly
class MATLABEngineCaller : public MATLABCaller {
private:
    std::shared_ptr<matlab::engine::MATLABEngine> matlabPtr;
    matlab::data::ArrayFactory factory;
    
public:
    MATLABEngineCaller(std::shared_ptr<matlab::engine::MATLABEngine> matlabEngine) 
        : matlabPtr(matlabEngine) {}
    
    ResponseData callHandler(const std::string& handlerName, const RequestData& request) override {
        try {
            // Create MATLAB Request object similar to legacy implementation
            matlab::data::ArrayFactory factory;
            
            // Create request parameters struct with route parameters
            std::vector<std::string> paramFieldNames;
            
            for (const auto& [key, value] : request.parameters) {
                paramFieldNames.push_back(key);
            }
            
            matlab::data::StructArray paramsStruct = factory.createStructArray({1, 1}, paramFieldNames);
            
            // Set the parameter values
            int fieldIndex = 0;
            for (const auto& [key, value] : request.parameters) {
                paramsStruct[0][paramFieldNames[fieldIndex]] = factory.createCharArray(value);
                fieldIndex++;
            }
            matlab::data::StructArray queryStruct = factory.createStructArray({1, 1}, {});
            matlab::data::StructArray headersStruct = factory.createStructArray({1, 1}, {});
            matlab::data::CharArray bodyData = factory.createCharArray(request.body);
            
            // Create Request object using the constructor
            std::vector<matlab::data::Array> requestArgs = {paramsStruct, queryStruct, headersStruct, bodyData};
            matlab::data::Array requestObj = matlabPtr->feval("blink.Request", requestArgs);
            
            // Create Response object using the constructor
            std::vector<matlab::data::Array> responseArgs = {};
            matlab::data::Array responseObj = matlabPtr->feval("blink.Response", responseArgs);
            
            // Look up the function handle by name
            auto it = functionHandles.find(handlerName);
            if (it == functionHandles.end()) {
                throw std::runtime_error("Handler not found: " + handlerName);
            }
            
            // Call the MATLAB function handle
            std::vector<matlab::data::Array> handlerArgs = {it->second, requestObj, responseObj};
            matlab::data::Array handlerResponse = matlabPtr->feval("feval", handlerArgs);
            
            // Extract response data from the responseObj (which was modified by the handler)
            ResponseData response;
            
            matlab::data::CharArray respData = matlabPtr->getProperty(handlerResponse, "Data");
            matlab::data::Array respStatus = matlabPtr->getProperty(handlerResponse, "StatusCode");
           
            response.body = respData.toAscii();
            response.statusCode = static_cast<int>(respStatus[0]);
            response.contentType = "text/html";
            
            return response;
            
        } catch (const std::exception& e) {
            ResponseData errorResponse;
            errorResponse.body = "Internal Server Error";
            errorResponse.statusCode = 500;
            errorResponse.contentType = "text/plain";
            return errorResponse;
        }
    }
    
    void storeFunctionHandle(const std::string& handlerName, const matlab::data::Array& functionHandle) {
        functionHandles[handlerName] = functionHandle;
    }
    
private:
    std::unordered_map<std::string, matlab::data::Array> functionHandles;
};

class MexFunction : public matlab::mex::Function {
private:
    std::shared_ptr<matlab::engine::MATLABEngine> matlabPtr = getEngine();
    matlab::data::ArrayFactory factory;
    
public:
    void operator()(matlab::mex::ArgumentList outputs, matlab::mex::ArgumentList inputs) {
        if (inputs.size() < 2) {
            throwError("At least two input arguments (port number and routes) are required.");
            return;
        }

        if (inputs[0].getType() != matlab::data::ArrayType::DOUBLE) {
            throwError("Port number must be a positive integer.");
            return;
        }

        if (inputs[1].getType() != matlab::data::ArrayType::VALUE_OBJECT) {
            throwError("Routes must be an array of Route objects.");
            return;
        }

        // Create MATLAB caller and BlinkApp
        auto matlabCaller = std::make_shared<MATLABEngineCaller>(matlabPtr);
        BlinkApp app(matlabCaller);
        
        // Get and validate port number
        double port = inputs[0][0];
        if (port <= 0 || port > 65535) {
            throwError("Port number must be between 1 and 65535.");
            return;
        }
        
        // Convert MATLAB Route objects to C++ Route structs
        std::vector<Route> routes = this->convertRoutesToRoute(inputs[1], matlabCaller);
        app.addRoutes(routes);
        
        // Handle static files if provided
        if (inputs.size() == 4) {
            std::string staticFiles = this->extractString(inputs[2], "StaticFiles");
            std::string staticRoute = this->extractString(inputs[3], "StaticRoute");
            app.addStaticFiles(staticFiles, staticRoute);
        }
        
        // Start the server (this will block)
        app.listen(static_cast<int>(port));
        
        // This should never be reached as listen() blocks
        throwError("Server failed to start or stopped unexpectedly.");
    }
    
private:
    void throwError(const std::string& message) {
        matlabPtr->feval(u"error", 0,
            std::vector<matlab::data::Array>({factory.createScalar(message)}));
    }
    
    std::string extractString(const matlab::data::Array& array, const std::string& fieldName) {
        if (array.getType() == matlab::data::ArrayType::CHAR) {
            return static_cast<matlab::data::CharArray>(array).toAscii();
        } else if (array.getType() == matlab::data::ArrayType::MATLAB_STRING) {
            matlab::data::StringArray stringArray = static_cast<matlab::data::StringArray>(array);
            return stringArray[0];
        } else {
            throwError(fieldName + " must be a string or char array.");
            return "";
        }
    }
    
    std::vector<Route> convertRoutesToRoute(const matlab::data::Array& routesArray, std::shared_ptr<MATLABCaller> matlabCaller) {
        std::vector<Route> routes;
        
        try {
            // Direct access to Route objects like in the original BlinkApp.cpp
            int numRoutes = routesArray.getNumberOfElements();
            
            for (int i = 0; i < numRoutes; ++i) {
                try {
                    // Extract route properties directly from the Route object
                    matlab::data::CharArray pathField = matlabPtr->getProperty(routesArray, i, "Path");
                    matlab::data::CharArray httpMethodField = matlabPtr->getProperty(routesArray, i, "HttpMethod");
                    const matlab::data::Array handler = matlabPtr->getProperty(routesArray, i, "Handler");
                    
                    // Convert to strings
                    std::string path = pathField.toAscii();
                    std::string httpMethod = httpMethodField.toAscii();
                    
                    // Generate a unique handler name
                    std::string handlerName = "handler_" + std::to_string(i);
                    
                    // Store the function handle directly in the MATLAB caller
                    // Cast to the concrete type to access the functionHandles map
                    auto* concreteCaller = static_cast<MATLABEngineCaller*>(matlabCaller.get());
                    concreteCaller->storeFunctionHandle(handlerName, handler);
                    
                    // Create and add Route object
                    Route route;
                    route.httpMethod = httpMethod;
                    route.path = path;
                    route.handlerName = handlerName;
                    routes.push_back(route);
                    
                } catch (const std::exception& e) {
                    throw;
                }
            }
        } catch (const std::exception& e) {
            throwError("Error converting Route objects: " + std::string(e.what()));
        } catch (...) {
            throwError("Unknown error converting Route objects.");
        }
        
        return routes;
    }
};