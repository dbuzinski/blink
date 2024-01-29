#include "CinchApp.h"


std::unordered_map<std::string, std::string> parseHeaders(auto request) {
    std::unordered_map<std::string, std::string> headersMap;
    for (auto [key, value] : *request) {
        headersMap.insert({std::string(key), std::string(value)});
    }
    return headersMap;
};

mxArray* mapToStruct(std::unordered_map<std::string, std::string> map) {
    mwSize dims[2] = {1, 1};
    const char **fieldNames = new const char *[map.size()];
    int i = 0;
    for (auto &kv : map) {
        fieldNames[i] = kv.first.c_str();
        i++;
    }
    mxArray *structArray = mxCreateStructArray(2, dims, map.size(), fieldNames);
    for (auto &kv : map) {
        mxArray *value = mxCreateString(kv.second.c_str());
        mxSetField(structArray, 0, kv.first.c_str(), value);
    }
    return structArray;
}

mxArray* createCinchRequest(std::string_view url, std::string_view query, std::string_view path, std::unordered_map<std::string, std::string> headers, std::string data) {
    std::unordered_map<std::string, std::string> queryMap = parseQuery(query);
    std::unordered_map<std::string, std::string> paramsMap = parseParams(url, path);
    mxArray* queryStruct = mapToStruct(queryMap);
    mxArray* paramsStruct = mapToStruct(paramsMap);
    mxArray* headersStruct = mapToStruct(headers);

    // Create Request object
    mxArray *lhs[1];
    mxArray *rhs[4];
    rhs[0] = paramsStruct;
    rhs[1] = queryStruct;
    rhs[2] = headersStruct;
    rhs[3] = mxCreateString(data.c_str());
    mexCallMATLAB(1, lhs, 4, rhs, "cinch.Request");

    return lhs[0];
};

mxArray* createCinchResponse() {
    // Create Response object
    mxArray *lhs[1];
    mexCallMATLAB(1, lhs, 0, nullptr, "cinch.Response");

    return lhs[0];
};

void apply_handler(auto res, auto req, mxArray *handler, auto request, auto response) {
    mxArray *lhs[1];
    mxArray *rhs[3];
    rhs[0] = handler;
    rhs[1] = request;
    rhs[2] = response;

    mexCallMATLAB(1, lhs, 3, rhs, "feval");

    mxArray *data = mxGetProperty(lhs[0], 0, "Data");
    mxArray *statusCodeField = mxGetProperty(lhs[0], 0, "StatusCode");

    char *strResult = mxArrayToString(data);
    int statusCodeResult = static_cast<int>(mxGetScalar(statusCodeField));

    // convert int statusCodeResult to string_view
    std::string statusCodeStr = std::to_string(statusCodeResult);
    std::string_view statusCode(statusCodeStr.data(), statusCodeStr.size());

    res->writeStatus(statusCodeStr);
    res->end(strResult);

    mxDestroyArray(lhs[0]);
    mxDestroyArray(rhs[1]);
    mxDestroyArray(rhs[2]);
};

void handle_request(auto res, auto req, char *path, mxArray *handler) {
    auto isAborted = std::make_shared<bool>(false);
    std::string_view query = req->getQuery();
    std::string_view url = req->getUrl();
    std::unordered_map<std::string, std::string> headers = parseHeaders(req);
    res->onData([req, res, isAborted, handler, url, query, path, headers, bodyBuffer = (std::string *)nullptr](std::string_view chunk, bool isLast) mutable {
        if (isLast && !*isAborted) {
            if (bodyBuffer) {
                bodyBuffer->append(chunk);
                mxArray *request = createCinchRequest(url, query, path, headers, *bodyBuffer);
                mxArray *response = createCinchResponse();
                apply_handler(res, req, handler, request, response);
                delete bodyBuffer;
            } else {
                bodyBuffer = new std::string;
                bodyBuffer->append(chunk);
                mxArray *request = createCinchRequest(url, query, path, headers, *bodyBuffer);
                mxArray *response = createCinchResponse();
                apply_handler(res, req, handler, request, response);
                delete bodyBuffer;
            }
        } else {
            if (!bodyBuffer) {
                bodyBuffer = new std::string;
            }
            bodyBuffer->append(chunk);
        }

    });
    res->onAborted([isAborted]() {
        *isAborted = true;
    });
};

void CinchApp::addStaticFiles(std::string staticFiles, std::string staticRoute) {
    // add leading and trailing slashes if not present
    if (staticRoute.front() != '/') {
        staticRoute.insert(0, "/");
    }
    if (staticRoute.back() != '/') {
        staticRoute.append("/");
    }
    ws.get(staticRoute + "*", [staticFiles, staticRoute](auto *res, auto *req) {
        std::string filePath = staticFiles + std::string(req->getUrl().begin(), req->getUrl().end()).substr(staticRoute.length() - 1);
        if (std::filesystem::exists(filePath) && std::filesystem::is_regular_file(filePath)) {
            std::ifstream fileStream(filePath, std::ios::binary);

            if (fileStream.is_open()) {
                fileStream.seekg(0, std::ios::end);
                size_t fileSize = fileStream.tellg();
                fileStream.seekg(0, std::ios::beg);

                std::string fileContent(fileSize, ' ');
                fileStream.read(&fileContent[0], fileSize);

                res->write(fileContent);
                fileStream.close();
                res->end();
            } else {
                res->writeStatus("500 Internal Server Error");
                res->end();
            }
        } else {
            res->writeStatus("404 Not Found");
            res->end("File not found");
        }
    });
};

void CinchApp::addRoutes(const mxArray *routes) {
    if (!mxIsClass(routes, "cinch.Route")) {
        mexErrMsgIdAndTxt("MATLAB:myfunction:invalidInputType",
                              "Routes must be an array of objects of the 'cinch.Route' class.");
    }

    int numRoutes = mxGetNumberOfElements(routes);

    for (int i = 0; i < numRoutes; ++i) {
        mxArray *pathField = mxGetProperty(routes, i, "Path");
        mxArray *handlerField = mxGetProperty(routes, i, "Handler");
        mxArray *httpMethodField = mxGetProperty(routes, i, "HttpMethod");

        char *path = mxArrayToString(pathField);
        char *httpMethod = mxArrayToString(httpMethodField);

        if (strcmp(httpMethod, "GET") == 0) {
            ws.get(path, [handlerField, path](auto *res, auto *req) {
                handle_request(res, req, path, handlerField);
            });
        } else if (strcmp(httpMethod, "POST") == 0) {
            ws.post(path, [path, handlerField](auto *res, auto *req) {
                handle_request(res, req, path, handlerField);
            });
        } else if (strcmp(httpMethod, "PUT") == 0) {
            ws.put(path, [path, handlerField](auto *res, auto *req) {
                handle_request(res, req, path, handlerField);
            });
        } else if (strcmp(httpMethod, "DELETE") == 0) {
            ws.del(path, [path, handlerField](auto *res, auto *req) {
                handle_request(res, req, path, handlerField);
            });
        } else {
            mexErrMsgIdAndTxt("cinch:serve:invalidHttpMethod",
                                "Invalid HTTP method: %s", httpMethod);
        }
   }
};
