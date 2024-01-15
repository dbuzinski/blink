#include "mex.h"
#include "CinchApp.h"


void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
    // Check the number of input arguments
    if (nrhs != 2) {
        mexErrMsgIdAndTxt("MATLAB:myfunction:invalidNumInputs",
                          "Two input arguments (port number and Routes) are required.");
    }

    // Check if the input is a real scalar for port number
    if (!mxIsDouble(prhs[0]) || mxIsComplex(prhs[0]) || mxGetNumberOfElements(prhs[0]) != 1) {
        mexErrMsgIdAndTxt("MATLAB:myfunction:invalidInputType",
                          "Port number must be a real scalar.");
    }

    // Get the port number from the first input argument
    double port = mxGetScalar(prhs[0]);

    // Create cinchApp instance
    CinchApp app;

    // Check if the second input is an array of Route objects
    app.addRoutes(prhs[1]);
    app.ws.listen(static_cast<int>(port), [](auto *listen_socket) {}).run();

    std::cout << "Failed to start webserver" << static_cast<int>(port) << std::endl;
}
