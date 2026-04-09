#include "mex.hpp"
#include "mexAdapter.hpp"
#include "BlinkApp.hpp"
#include "RequestParser.hpp"
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include <unordered_map>

namespace {

std::string matlabScalarToUtf8(const matlab::data::Array& arr) {
    if (arr.getNumberOfElements() == 0) {
        return "";
    }
    switch (arr.getType()) {
    case matlab::data::ArrayType::CHAR:
        return static_cast<matlab::data::CharArray>(arr).toAscii();
    case matlab::data::ArrayType::MATLAB_STRING: {
        matlab::data::StringArray sa = arr;
        return std::string(sa[0]);
    }
    default:
        return "";
    }
}

void appendResponseHeadersFromMatlabStruct(
    const matlab::data::StructArray& s,
    std::vector<std::pair<std::string, std::string>>& out) {
    if (s.getNumberOfElements() < 1) {
        return;
    }
    for (const auto& fieldId : s.getFieldNames()) {
        const std::string field = static_cast<std::string>(fieldId);
        matlab::data::Array val = s[0][field];
        std::string value = matlabScalarToUtf8(val);
        std::string headerName = RequestParser::matlabFieldNameToHttpHeaderName(field);
        if (!headerName.empty()) {
            out.emplace_back(std::move(headerName), std::move(value));
        }
    }
}

} // namespace

// MATLAB caller implementation that uses MATLAB Engine directly
class MATLABEngineCaller : public MATLABCaller {
private:
    std::shared_ptr<matlab::engine::MATLABEngine> matlabPtr;
    matlab::data::ArrayFactory factory;
    /// Serializes MATLAB Engine use (thread-safe dispatch vs single-threaded MATLAB interpreter).
    std::mutex matlabMutex_;

    /// `fevalAsync` / `FutureResult::get()` are available on the MEX `matlab::engine::MATLABEngine` from
    /// `mex.hpp` (`cppmex/mexMatlabEngine.hpp`). Do **not** include the standalone `<MatlabEngine.hpp>`:
    /// it re-declares the same `matlab::engine::*` types and breaks the build.
    ResponseData invokeMatlab(const std::string& handlerName, const RequestData& request) {
        std::lock_guard<std::mutex> lock(matlabMutex_);
        try {
            matlab::data::ArrayFactory factory;

            std::vector<std::string> paramFieldNames;
            for (const auto& [key, value] : request.parameters) {
                paramFieldNames.push_back(key);
            }

            matlab::data::StructArray paramsStruct = factory.createStructArray({1, 1}, paramFieldNames);

            int fieldIndex = 0;
            for (const auto& [key, value] : request.parameters) {
                paramsStruct[0][paramFieldNames[fieldIndex]] = factory.createCharArray(value);
                fieldIndex++;
            }

            std::unordered_map<std::string, std::string> queryParsed =
                RequestParser::parseQueryString(request.query);
            std::unordered_map<std::string, std::string> queryByFieldName;
            for (const auto& [key, value] : queryParsed) {
                queryByFieldName[RequestParser::queryKeyToMatlabFieldName(key)] = value;
            }
            std::vector<std::string> queryFieldNames;
            std::vector<std::string> queryFieldValues;
            queryFieldNames.reserve(queryByFieldName.size());
            queryFieldValues.reserve(queryByFieldName.size());
            for (const auto& [fieldName, value] : queryByFieldName) {
                queryFieldNames.push_back(fieldName);
                queryFieldValues.push_back(value);
            }
            matlab::data::StructArray queryStruct = factory.createStructArray({1, 1}, queryFieldNames);
            for (size_t qi = 0; qi < queryFieldNames.size(); ++qi) {
                queryStruct[0][queryFieldNames[qi]] = factory.createCharArray(queryFieldValues[qi]);
            }

            std::unordered_map<std::string, std::string> headersByFieldName;
            for (const auto& [name, value] : request.headers) {
                headersByFieldName[RequestParser::queryKeyToMatlabFieldName(name)] = value;
            }
            std::vector<std::string> headerFieldNames;
            std::vector<std::string> headerFieldValues;
            headerFieldNames.reserve(headersByFieldName.size());
            headerFieldValues.reserve(headersByFieldName.size());
            for (const auto& [fieldName, value] : headersByFieldName) {
                headerFieldNames.push_back(fieldName);
                headerFieldValues.push_back(value);
            }
            matlab::data::StructArray headersStruct = factory.createStructArray({1, 1}, headerFieldNames);
            for (size_t hi = 0; hi < headerFieldNames.size(); ++hi) {
                headersStruct[0][headerFieldNames[hi]] = factory.createCharArray(headerFieldValues[hi]);
            }
            matlab::data::CharArray bodyData = factory.createCharArray(request.body);

            std::vector<matlab::data::Array> requestArgs = {paramsStruct, queryStruct, headersStruct, bodyData};
            matlab::data::Array requestObj =
                matlabPtr->fevalAsync("blink.Request", requestArgs).get();

            std::vector<matlab::data::Array> responseCtorArgs;
            matlab::data::Array responseObj =
                matlabPtr->fevalAsync("blink.Response", responseCtorArgs).get();

            auto it = functionHandles.find(handlerName);
            if (it == functionHandles.end()) {
                throw std::runtime_error("Handler not found: " + handlerName);
            }

            std::vector<matlab::data::Array> handlerArgs = {it->second, requestObj, responseObj};
            matlab::data::Array handlerResponse =
                matlabPtr->fevalAsync("feval", handlerArgs).get();

            ResponseData response;

            matlab::data::CharArray respData = matlabPtr->getProperty(handlerResponse, "Data");
            matlab::data::Array respStatus = matlabPtr->getProperty(handlerResponse, "StatusCode");
            matlab::data::Array contentTypeArr = matlabPtr->getProperty(handlerResponse, "ContentType");
            matlab::data::Array headersArr = matlabPtr->getProperty(handlerResponse, "Headers");

            response.body = respData.toAscii();
            response.statusCode = static_cast<int>(respStatus[0]);
            response.contentType = matlabScalarToUtf8(contentTypeArr);

            if (headersArr.getType() == matlab::data::ArrayType::STRUCT) {
                appendResponseHeadersFromMatlabStruct(
                    static_cast<matlab::data::StructArray>(headersArr),
                    response.headers);
            }

            return response;

        } catch (const std::exception& e) {
            ResponseData errorResponse;
            errorResponse.body = "Internal Server Error";
            errorResponse.statusCode = 500;
            errorResponse.contentType = "text/plain";
            return errorResponse;
        }
    }

public:
    MATLABEngineCaller(std::shared_ptr<matlab::engine::MATLABEngine> matlabEngine) 
        : matlabPtr(matlabEngine) {}
    
    ResponseData callHandler(const std::string& handlerName, const RequestData& request) override {
        return invokeMatlab(handlerName, request);
    }

    void callHandlerAsync(const std::string& handlerName,
        const RequestData& request,
        std::function<void(std::function<void()>)> deferToLoop,
        std::function<void(ResponseData)> onComplete) override {
        // Offloads `invokeMatlab` so the µWebSockets thread can keep accepting I/O; completion is
        // posted via `deferToLoop` (typically `uWS::Loop::defer`). MATLAB remains single-threaded;
        // see `matlabMutex_`.
        std::thread([this, handlerName, request, deferToLoop = std::move(deferToLoop), onComplete = std::move(onComplete)]() mutable {
            ResponseData r = invokeMatlab(handlerName, request);
            deferToLoop([r = std::move(r), onComplete = std::move(onComplete)]() mutable {
                onComplete(std::move(r));
            });
        }).detach();
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
        app.setAsyncMatlabHandlers(true);
        
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