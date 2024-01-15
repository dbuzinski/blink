#include "CinchApp.h"
#include "mex.h"


void CinchApp::addRoutes(const mxArray *routes) {
    if (!mxIsClass(routes, "cinch.Route")) {
        mexErrMsgIdAndTxt("MATLAB:myfunction:invalidInputType",
                              "Routes must be an array of objects of the 'cinch.Route' class.");
    }

    int numRoutes = mxGetNumberOfElements(routes);

    for (int i = 0; i < numRoutes; ++i) {
        mxArray *pathField = mxGetProperty(routes, i, "Path");
        mxArray *handlerField = mxGetProperty(routes, i, "Handler");

        if (!mxIsChar(pathField) || !mxIsFunctionHandle(handlerField)) {
            mexErrMsgIdAndTxt("cinch:serve:invalidProperty",
                              "Path must be a string, and Handler must be a function handle.");
        }

        char *path = mxArrayToString(pathField);

        ws.get(path, [handlerField, path](auto *res, auto *req) {
            mxArray *lhs[1];
            mxArray *rhs[2];
            rhs[0] = handlerField;

            auto query = req->getQuery();
            auto queryMap = parseQuery(query);

            mwSize dims[2] = {1, 1};
            const char* fieldNames[] = {"query", "params"};
            rhs[1] = mxCreateStructArray(2, dims, 2, fieldNames);

            const char **queryFieldNames = new const char *[queryMap.size()];
            int i = 0;
            for (auto &kv : queryMap) {
                queryFieldNames[i] = kv.first.c_str();
                i++;
            }
            mxArray *queryStruct = mxCreateStructArray(2, dims, queryMap.size(), queryFieldNames);
            for (auto &kv : queryMap) {
                mxArray *value = mxCreateString(kv.second.c_str());
                mxSetField(queryStruct, 0, kv.first.c_str(), value);
            }
    
            auto url = req->getUrl();
            auto paramsMap = parseParams(url, path);
            const char **paramsFieldNames = new const char *[paramsMap.size()];
            int j = 0;
            for (auto &kv : paramsMap) {
                paramsFieldNames[j] = kv.first.c_str();
                j++;
            }
            mxArray *paramsStruct = mxCreateStructArray(2, dims, paramsMap.size(), paramsFieldNames);
            for (auto &kv : paramsMap) {
                mxArray *value = mxCreateString(kv.second.c_str());
                mxSetField(paramsStruct, 0, kv.first.c_str(), value);
            }

            // Set the field names for the outer struct
            mxSetField(rhs[1], 0, "query", queryStruct);
            mxSetField(rhs[1], 0, "params", paramsStruct);

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
            res->end("");
        });
    }
};
