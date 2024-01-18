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
    mwSize dims[2] = {1, 1};
    const char* fieldNames[] = {"Parameters", "Query", "Headers", "Data"};
    mxArray* request = mxCreateStructArray(2, dims, 4, fieldNames);

    std::unordered_map<std::string, std::string> queryMap = parseQuery(query);
    std::unordered_map<std::string, std::string> paramsMap = parseParams(url, path);
    mxArray* queryStruct = mapToStruct(queryMap);
    mxArray* paramsStruct = mapToStruct(paramsMap);
    mxArray* headersStruct = mapToStruct(headers);

    mxSetField(request, 0, "Query", queryStruct);
    mxSetField(request, 0, "Parameters", paramsStruct);
    mxSetField(request, 0, "Headers", headersStruct);
    mxSetField(request, 0, "Data", mxCreateString(data.c_str()));
    return request;
};

void apply_handler(auto res, auto req, mxArray *handler, auto request) {
    mxArray *lhs[1];
    mxArray *rhs[2];
    rhs[0] = handler;
    rhs[1] = request;

    mexCallMATLAB(1, lhs, 2, rhs, "feval");

    if (!mxIsChar(lhs[0])) {
        mxDestroyArray(lhs[0]);
        mxDestroyArray(rhs[1]);
        mexErrMsgIdAndTxt("cinch:serve:invalidResult",
                            "Handler must return a string.");
    }

    const char *strResult = mxArrayToString(lhs[0]);
    mxDestroyArray(lhs[0]);
    mxDestroyArray(rhs[1]);

    res->end(strResult);
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
                apply_handler(res, req, handler, request);
                delete bodyBuffer;
            } else {
                bodyBuffer = new std::string;
                bodyBuffer->append(chunk);
                mxArray *request = createCinchRequest(url, query, path, headers, *bodyBuffer);
                apply_handler(res, req, handler, request);
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
