#include "CinchApp.hpp"


CinchApp::CinchApp(std::shared_ptr<matlab::engine::MATLABEngine> matlabPtr) {
    this->matlabPtr = matlabPtr;
};

std::unordered_map<std::string, std::string> parseHeaders(auto request) {
    std::unordered_map<std::string, std::string> headersMap;
    for (auto [key, value] : *request) {
        headersMap.insert({std::string(key), std::string(value)});
    }
    return headersMap;
};

matlab::data::Array mapToStruct(std::unordered_map<std::string, std::string> map) {
    matlab::data::ArrayFactory factory;
    std::vector<std::string> fieldNames;
    for (auto const &[key, value] : map) {
        fieldNames.push_back(key);
    }
    matlab::data::StructArray structArray = factory.createStructArray({1, 1}, fieldNames);
    for (auto const &[key, value] : map) {
        structArray[0][key] = factory.createCharArray(value.c_str());
    }
    return structArray;
};

void CinchApp::apply_handler(auto res, auto req, const char *path, const char *bodyBuffer, const matlab::data::Array &handler) {
    std::string_view query = req->getQuery();
    std::string_view url = req->getUrl();

    std::unordered_map<std::string, std::string> queryMap = parseQuery(query);
    std::unordered_map<std::string, std::string> paramsMap = parseParams(url, path);
    std::unordered_map<std::string, std::string> headersMap = parseHeaders(req);

    matlab::data::Array queryStruct = mapToStruct(queryMap);
    matlab::data::Array paramsStruct = mapToStruct(paramsMap);
    matlab::data::Array headersStruct = mapToStruct(headersMap);
    matlab::data::CharArray body = factory.createCharArray(bodyBuffer);

    std::vector<matlab::data::Array> reqArgs({paramsStruct, queryStruct, headersStruct, body});
    matlab::data::Array request = matlabPtr->feval("cinch.Request", reqArgs);

    matlab::data::Array response = matlabPtr->feval("cinch.Response", factory.createEmptyArray());

    std::vector<matlab::data::Array> handlerArgs({handler, request, response});
    matlab::data::Array handlerResp = matlabPtr->feval("feval", handlerArgs);
    matlab::data::CharArray respData = matlabPtr->getProperty(handlerResp, "Data");
    matlab::data::Array respStatus = matlabPtr->getProperty(handlerResp, "StatusCode");

    // convert int statusCodeResult to string_view
    int statusCodeResult = static_cast<int>(respStatus[0]);
    std::string statusCodeStr = std::to_string(statusCodeResult);
    std::string_view statusCode(statusCodeStr.data(), statusCodeStr.size());

    res->writeStatus(statusCodeStr);
    res->end(respData.toAscii());
};

void CinchApp::handle_request(auto res, auto req, const char *path, const matlab::data::Array &handler) {
    auto isAborted = std::make_shared<bool>(false);
    std::string_view query = req->getQuery();
    std::string_view url = req->getUrl();
    std::unordered_map<std::string, std::string> headers = parseHeaders(req);
    res->onData([this, req, res, handler, isAborted, path, headers, bodyBuffer = (std::string *)nullptr](std::string_view chunk, bool isLast) mutable {
        if (isLast && !*isAborted) {
            if (bodyBuffer) {
                bodyBuffer->append(chunk);
                apply_handler(res, req, path, (*bodyBuffer).c_str(), handler);
                delete bodyBuffer;
            } else {
                bodyBuffer = new std::string;
                bodyBuffer->append(chunk);
                apply_handler(res, req, path, (*bodyBuffer).c_str(), handler);
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

void CinchApp::addStaticFiles(std::string staticFiles, std::string staticRoute) {
    // add leading and trailing slashes if not present
    if (staticRoute.front() != '/') {
        staticRoute.insert(0, "/");
    }
    if (staticRoute.back() != '/') {
        staticRoute.append("/");
    }
    ws.get(staticRoute + "*", [staticFiles, staticRoute](auto *res, auto *req) {
        std::string filePath = staticFiles + std::string(req->getUrl().begin(), req->getUrl().end()).substr(staticRoute.length() - 1);
        if (std::filesystem::exists(filePath) && std::filesystem::is_regular_file(filePath)) {
            std::ifstream fileStream(filePath, std::ios::binary);

            if (fileStream.is_open()) {
                fileStream.seekg(0, std::ios::end);
                size_t fileSize = fileStream.tellg();
                fileStream.seekg(0, std::ios::beg);

                std::string fileContent(fileSize, ' ');
                fileStream.read(&fileContent[0], fileSize);

                res->write(fileContent);
                fileStream.close();
                res->end();
            } else {
                res->writeStatus("500 Internal Server Error");
                res->end();
            }
        } else {
            res->writeStatus("404 Not Found");
            res->end("File not found");
        }
    });
};

void CinchApp::addRoutes(const matlab::data::Array &routes) {
    int numRoutes = routes.getNumberOfElements();

    for (int i = 0; i < numRoutes; ++i) {
        matlab::data::CharArray pathField = matlabPtr->getProperty(routes, i, "Path");
        matlab::data::CharArray httpMethodField = matlabPtr->getProperty(routes, i, "HttpMethod");
        const matlab::data::Array handler = matlabPtr->getProperty(routes, i, "Handler");

        std::string pathStr = pathField.toAscii();
        std::string httpMethodStr = httpMethodField.toAscii();
        const char *path = pathStr.c_str();
        const char *httpMethod = httpMethodStr.c_str();

        if (strcmp(httpMethod, "GET") == 0) {
            ws.get(path, [this, handler, path](auto *res, auto *req) {
                handle_request(res, req, path, handler);
            });
        } else if (strcmp(httpMethod, "POST") == 0) {
            ws.post(path, [this, path, handler](auto *res, auto *req) {
                handle_request(res, req, path, handler);
            });
        } else if (strcmp(httpMethod, "PUT") == 0) {
            ws.put(path, [this, path, handler](auto *res, auto *req) {
                handle_request(res, req, path, handler);
            });
        } else if (strcmp(httpMethod, "DELETE") == 0) {
            ws.del(path, [this, path, handler](auto *res, auto *req) {
                handle_request(res, req, path, handler);
            });
        } else {
            std::cout << "Invalid HTTP method: " << httpMethod << std::endl;
        }
   }
};
