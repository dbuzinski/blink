#include "mex.h"
#include "App.h"

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
    // Check the number of input arguments
    if (nrhs != 1) {
        mexErrMsgIdAndTxt("MATLAB:myfunction:invalidNumInputs",
                          "One input argument (MATLAB object) is required.");
    }

    // Check if the input is an object of the custom MATLAB class "App"
    if (!mxIsClass(prhs[0], "blaze.App")) {
        mexErrMsgIdAndTxt("MATLAB:myfunction:invalidInputType",
                          "Input argument must be an object of the 'blaze.App' class.");
    }

    // Get the PortNumber property from the MATLAB object
    mxArray *portField = mxGetProperty(prhs[0], 0, "Port");
    if (portField == nullptr || !mxIsDouble(portField) || mxIsComplex(portField) || mxGetNumberOfElements(portField) != 1) {
        mexErrMsgIdAndTxt("MATLAB:myfunction:invalidProperty",
                          "Port property must be a real scalar.");
    }

    // Get the Route property from the MATLAB object
    mxArray *routeField = mxGetProperty(prhs[0], 0, "Route");
    if (routeField == nullptr || !mxIsFunctionHandle(routeField)) {
        mexErrMsgIdAndTxt("MATLAB:myfunction:invalidProperty",
                          "Route property must be a function handle.");
    }

    // Get the port number from the PortNumber property
    double port = mxGetScalar(portField);

    // Call your C++ function
    uWS::SSLApp app({
        .key_file_name = "misc/key.pem",
        .cert_file_name = "misc/cert.pem",
        .passphrase = "1234"
    });

    app.get("/hello/:name", [routeField](auto *res, auto *req) {
        // Call your C++ function
        mxArray *lhs[1];
        mxArray *rhs[2];
        rhs[0] = routeField;
        std::string_view name = req->getParameter("name");

        rhs[1] = mxCreateString(std::string(name).c_str());
        mexCallMATLAB(1, lhs, 2, rhs, "feval");

        // Check if the result is a string
        if (!mxIsChar(lhs[0])) {
            mxDestroyArray(lhs[0]);
            mxDestroyArray(rhs[1]);
            mexErrMsgIdAndTxt("MATLAB:myfunction:invalidResult",
                              "Route function must return a string.");
        }

        // Get the string result from the function_handle
        const char *strResult = mxArrayToString(lhs[0]);

        // Cleanup allocated memory
        mxDestroyArray(lhs[0]);
        mxDestroyArray(rhs[1]);

        // Write the output of the evaluated function_handle to the response
        res->write(strResult);
        res->end("");
    });

    // Capture port by value in the lambda
    app.listen(static_cast<int>(port), [port](auto *listen_socket) {
        if (listen_socket) {
            std::cout << "Listening on port " << static_cast<int>(port) << std::endl;
        }
    }).run();

    std::cout << "Failed to listen on port " << static_cast<int>(port) << std::endl;
}
