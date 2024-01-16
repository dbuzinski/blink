#pragma once
#include "App.h"
#include "mex.h"
#include "parsing.h"
#include<string>


class CinchApp {
public:
    uWS::SSLApp ws;
    void addRoutes(const mxArray *routes);
};
