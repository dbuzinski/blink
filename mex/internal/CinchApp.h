#pragma once
#include "App.h"
#include "mex.h"
#include "parsing.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>

class CinchApp {
public:
    uWS::SSLApp ws;
    void addRoutes(const mxArray *routes);
    void addStaticFiles(std::string staticPath, std::string staticRoute);
};
