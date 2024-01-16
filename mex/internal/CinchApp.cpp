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

mxArray* createCinchRequest(std::string_view url, std::string_view query, std::string_view path, std::unordered_map<std::string, std::string> headers) {
    mwSize dims[2] = {1, 1};
    const char* fieldNames[] = {"Parameters", "Query", "Headers", "Data"};
    mxArray* request = mxCreateStructArray(2, dims, 4, fieldNames);

    std::unordered_map<std::string, std::string> queryMap = parseQuery(query);
    std::unordered_map<std::string, std::string> paramsMap = parseParams(url, path);
    mxArray* queryStruct = mapToStruct(queryMap);
    mxArray* paramsStruct = mapToStruct(paramsMap);
    mxArray* headersStruct = mapToStruct(headers);

    mxSetField(request, 0, "Data", mxCreateString(""));
    mxSetField(request, 0, "Query", queryStruct);
    mxSetField(request, 0, "Parameters", paramsStruct);
    mxSetField(request, 0, "Headers", headersStruct);
    return request;
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
        // mxArray *httpMethodField = mxGetProperty(routes, i, "HttpMethod");

        char *path = mxArrayToString(pathField);

        ws.get(path, [handlerField, path](auto *res, auto *req) {
            std::string_view query = req->getQuery();
            std::string_view url = req->getUrl();
            std::unordered_map<std::string, std::string> headers = parseHeaders(req);
            mxArray *lhs[1];
            mxArray *rhs[2];
            rhs[0] = handlerField;
            rhs[1] = createCinchRequest(url, query, path, headers);

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

            res->write(strResult);
            res->end();
        });
    }
};
