#include "mex.h"
#include "CinchApp.h"


void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
    if (nrhs != 2) {
        mexErrMsgIdAndTxt("MATLAB:myfunction:invalidNumInputs",
                          "Two input arguments (port number and routes) are required.");
    }

    if (!mxIsDouble(prhs[0]) || mxIsComplex(prhs[0]) || mxGetNumberOfElements(prhs[0]) != 1 ||
        mxGetScalar(prhs[0]) < 0) {
        mexErrMsgIdAndTxt("MATLAB:myfunction:invalidInputType",
                          "Port number must be a positive integer.");
    }

    if (!mxIsClass(prhs[1], "cinch.Route")) {
        mexErrMsgIdAndTxt("MATLAB:myfunction:invalidInputType",
                          "Routes must be an array of 'cinch.Route' objects.");
    }

    CinchApp app;
    double port = mxGetScalar(prhs[0]);
    app.addRoutes(prhs[1]);
    app.ws.listen(static_cast<int>(port), [](auto *listen_socket) {}).run();

    std::cout << "Failed to start webserver" << static_cast<int>(port) << std::endl;
}
