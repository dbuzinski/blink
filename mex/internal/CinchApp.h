#pragma once
#include "App.h"
#include "mex.h"
#include "RequestParser.h"


class CinchApp {
public:
    uWS::SSLApp ws;
    void addRoutes(const mxArray *routes);

};
