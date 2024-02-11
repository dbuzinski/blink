#pragma once
#include "App.h"
#include "mex.hpp"
#include "parsing.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>


class BlinkApp {
public:
    BlinkApp(std::shared_ptr<matlab::engine::MATLABEngine> matlabPtr);
    uWS::SSLApp ws;
    void addRoutes(const matlab::data::Array& routes);
    void addStaticFiles(std::string staticPath, std::string staticRoute);
private:
    matlab::data::ArrayFactory factory;
    std::shared_ptr<matlab::engine::MATLABEngine> matlabPtr;
    void apply_handler(auto res, auto req, std::string path, const char *bodyBuffer, const matlab::data::Array &handler);
    void handle_request(auto res, auto req, std::string path, const matlab::data::Array &handler);
};
