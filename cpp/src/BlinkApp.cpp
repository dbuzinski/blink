#include "BlinkApp.hpp"
#include "WebServer.hpp"
#include "Loop.h"
#include "RequestParser.hpp"
#include "HttpStatus.hpp"
#include <utility>
#include <algorithm>
#include <fstream>
#include <filesystem>
#include <string>
#include <string_view>
#include <system_error>
#include <memory>
#include <vector>
#include <stdexcept>
#include <cctype>
#include <cstdio>

namespace {

bool headerNameEqualsIgnoreCase(const std::string& left, const std::string& right) {
    if (left.size() != right.size()) {
        return false;
    }
    for (size_t i = 0; i < left.size(); ++i) {
        if (std::tolower(static_cast<unsigned char>(left[i])) != std::tolower(static_cast<unsigned char>(right[i]))) {
            return false;
        }
    }
    return true;
}

std::filesystem::path normalizePathBestEffort(const std::filesystem::path& path) {
    std::error_code error_code;
    auto abs = std::filesystem::absolute(path, error_code);
    if (error_code) {
        return path.lexically_normal();
    }
    return abs.lexically_normal();
}

bool pathHasPrefix(const std::filesystem::path& prefix, const std::filesystem::path& full) {
    auto prefix_it = prefix.begin();
    auto full_it = full.begin();
    for (; prefix_it != prefix.end(); ++prefix_it, ++full_it) {
        if (full_it == full.end() || *prefix_it != *full_it) {
            return false;
        }
    }
    return true;
}

std::string mimeTypeForPath(const std::filesystem::path& path) {
    const std::string ext = path.extension().string();
    if (ext == ".html" || ext == ".htm") {
        return "text/html; charset=utf-8";
    }
    if (ext == ".css") {
        return "text/css; charset=utf-8";
    }
    if (ext == ".js" || ext == ".mjs") {
        return "text/javascript; charset=utf-8";
    }
    if (ext == ".json") {
        return "application/json; charset=utf-8";
    }
    if (ext == ".txt") {
        return "text/plain; charset=utf-8";
    }
    if (ext == ".svg") {
        return "image/svg+xml";
    }
    if (ext == ".png") {
        return "image/png";
    }
    if (ext == ".jpg" || ext == ".jpeg") {
        return "image/jpeg";
    }
    if (ext == ".gif") {
        return "image/gif";
    }
    if (ext == ".wasm") {
        return "application/wasm";
    }
    return "application/octet-stream";
}

} // namespace

void MATLABCaller::callHandlerAsync(const std::string& handlerName,
    const RequestData& request,
    const std::function<void(std::function<void()>)>& /*deferToLoop*/,
    const std::function<void(ResponseData)>& onComplete) {
    onComplete(callHandler(handlerName, request));
}

void BlinkApp::setAsyncMatlabHandlers(bool enable) {
    asyncMatlabHandlers_ = enable;
}

void BlinkApp::setMaxRequestBodyBytes(size_t bytes) {
    maxRequestBodyBytes_ = bytes;
}

BlinkApp::BlinkApp(std::shared_ptr<MATLABCaller> matlabCaller) 
    : webServer_(std::make_unique<WebServer>()), matlabCaller_(std::move(matlabCaller)) {
}

void BlinkApp::addRoutes(const std::vector<Route>& routes) {
    registeredRoutes_.insert(registeredRoutes_.end(), routes.begin(), routes.end());
    for (const auto& route : routes) {
        if (route.http_method == "GET") {
            webServer_->get(route.path, [this, route](auto *res, auto *req) {
                handleRequest(res, req, route);
            });
        } else if (route.http_method == "POST") {
            webServer_->post(route.path, [this, route](auto *res, auto *req) {
                handleRequest(res, req, route);
            });
        } else if (route.http_method == "PUT") {
            webServer_->put(route.path, [this, route](auto *res, auto *req) {
                handleRequest(res, req, route);
            });
        } else if (route.http_method == "DELETE") {
            webServer_->del(route.path, [this, route](auto *res, auto *req) {
                handleRequest(res, req, route);
            });
        }
    }
}

void BlinkApp::addStaticFiles(const std::string& staticPath, const std::string& staticRoute) {
    webServer_->get(staticRoute + "/*", [this, staticPath, staticRoute](auto *res, auto *req) {
        const std::string url = std::string(req->getUrl());

        std::string rel = (url.size() >= staticRoute.size()) ? url.substr(staticRoute.length()) : std::string();
        if (!rel.empty() && rel[0] == '/') {
            rel.erase(0, 1);
        }

        const std::filesystem::path root = normalizePathBestEffort(std::filesystem::path(staticPath));
        std::filesystem::path candidate = normalizePathBestEffort(root / std::filesystem::path(rel));

        // If the path resolves outside the static root, deny.
        if (!pathHasPrefix(root, candidate)) {
            res->writeStatus(httpStatusLine(403));
            res->writeHeader("Content-Type", "text/plain; charset=utf-8");
            res->end("Forbidden");
            return;
        }

        // If the file exists, enforce symlink-safe containment too (best effort).
        std::error_code error_code;
        if (std::filesystem::exists(candidate, error_code) && !error_code) {
            auto canon_root = std::filesystem::weakly_canonical(root, error_code);
            if (!error_code) {
                auto canon_candidate = std::filesystem::weakly_canonical(candidate, error_code);
                if (!error_code && !pathHasPrefix(canon_root, canon_candidate)) {
                    res->writeStatus(httpStatusLine(403));
                    res->writeHeader("Content-Type", "text/plain; charset=utf-8");
                    res->end("Forbidden");
                    return;
                }
            }
        }

        serveStaticFile(candidate.string(), res);
    });
}

void BlinkApp::beginListen(int port) {
    registerFallbackRoute();
    bool failed_to_listen = false;
    webServer_->listen(port, [&failed_to_listen](auto* listen_socket) {
        failed_to_listen = (listen_socket == nullptr);
    });

    if (failed_to_listen) {
        throw std::runtime_error("Failed to listen on port " + std::to_string(port));
    }
}

void BlinkApp::registerFallbackRoute() {
    if (fallbackRegistered_) {
        return;
    }
    fallbackRegistered_ = true;

    webServer_->any("/*", [this](auto* res, auto* /*req*/) {
        sendErrorResponse(res, 404, "Not Found");
    });
}

void BlinkApp::runEventLoop() {
    webServer_->run();
}

void BlinkApp::stop() {
    webServer_->stopListening();
}

void BlinkApp::listen(int port) {
    beginListen(port);
    runEventLoop();
}

template<typename ResType, typename ReqType>
void BlinkApp::handleRequest(ResType* res, ReqType* req, const Route& route) {
    auto is_aborted = std::make_shared<bool>(false);
    auto body_buffer = std::make_shared<std::string>();
    
    // Handle request body using onData callback
    res->onData([this, res, req, route, is_aborted, body_buffer](std::string_view chunk, bool isLast) mutable {
        if (*is_aborted) {
            return;
        }

        if (body_buffer->size() + chunk.size() > maxRequestBodyBytes_) {
            *is_aborted = true;
            body_buffer->clear();
            res->writeStatus(httpStatusLine(413));
            res->writeHeader("Content-Type", "text/plain; charset=utf-8");
            res->end("Payload Too Large");
            return;
        }

        body_buffer->append(chunk);

        if (isLast) {
            processRequest(res, req, route, std::string_view(*body_buffer), is_aborted);
        }
    });
    
    res->onAborted([is_aborted, body_buffer]() {
        *is_aborted = true;
        body_buffer->clear();
    });
}


template<typename ResType>
void BlinkApp::serveStaticFile(const std::string& filePath, ResType* res) {
    std::error_code error_code;
    if (!std::filesystem::is_regular_file(filePath, error_code) || error_code) {
        res->writeStatus(httpStatusLine(404));
        res->writeHeader("Content-Type", "text/plain; charset=utf-8");
        res->end("File not found");
        return;
    }

    const std::filesystem::path file_path(filePath);
    const auto file_size = std::filesystem::file_size(file_path, error_code);
    if (error_code) {
        res->writeStatus(httpStatusLine(404));
        res->writeHeader("Content-Type", "text/plain; charset=utf-8");
        res->end("File not found");
        return;
    }

    FILE* raw = std::fopen(filePath.c_str(), "rb");
    if (!raw) {
        res->writeStatus(httpStatusLine(404));
        res->writeHeader("Content-Type", "text/plain; charset=utf-8");
        res->end("File not found");
        return;
    }

    res->writeStatus(httpStatusLine(200));
    res->writeHeader("Content-Type", mimeTypeForPath(file_path));
    res->writeHeader("Content-Length", std::to_string(file_size));

    auto file = std::shared_ptr<FILE>(raw, [](FILE* file_ptr) {
        if (file_ptr) {
            std::fclose(file_ptr);
        }
    });
    auto buffer = std::make_shared<std::string>(64 * 1024, '\0');

    res->onAborted([file, buffer]() mutable {
        file.reset();
        buffer.reset();
    });

    auto fill_stream = [res, file, buffer, file_size]() mutable {
        if (!file) {
            return;
        }

        bool is_ok = true;
        bool done = false;
        while (is_ok && !done) {
            const uintmax_t offset = res->getWriteOffset();
            const uintmax_t remaining = file_size > offset ? (file_size - offset) : 0;
            if (remaining == 0) {
                return;
            }

            const size_t to_read = static_cast<size_t>(std::min<uintmax_t>(buffer->size(), remaining));
            if (std::fseek(file.get(), static_cast<long>(offset), SEEK_SET) != 0) {
                res->writeStatus(httpStatusLine(500));
                res->writeHeader("Content-Type", "text/plain; charset=utf-8");
                res->end("Internal Server Error");
                file.reset();
                buffer.reset();
                return;
            }

            const size_t actually_read = std::fread(buffer->data(), 1, to_read, file.get());
            if (actually_read == 0) {
                res->writeStatus(httpStatusLine(500));
                res->writeHeader("Content-Type", "text/plain; charset=utf-8");
                res->end("Internal Server Error");
                file.reset();
                buffer.reset();
                return;
            }

            auto try_end_result = res->tryEnd(std::string_view(buffer->data(), actually_read), file_size);
            is_ok = try_end_result.first;
            done = try_end_result.second;
        }

        if (done) {
            file.reset();
            buffer.reset();
        }
    };

    res->onWritable([fill_stream](uintmax_t /*offset*/) mutable {
        fill_stream();
        return true;
    });

    fill_stream();
}

template<typename ResType, typename ReqType>
void BlinkApp::processRequest(ResType* res,
    ReqType* req,
    const Route& route,
    std::string_view body,
    const std::shared_ptr<bool>& cancelled) {
    try {
        RequestData request = RequestParser::parseRequest(req, body);
        request.parameters = RequestParser::collectRouteParameters(req, route.path);

        if (asyncMatlabHandlers_) {
            uWS::Loop* loop = webServer_->getLoop();
            std::function<void(std::function<void()>)> defer_to_loop =
                [loop](std::function<void()> func) { loop->defer(std::move(func)); };
            matlabCaller_->callHandlerAsync(route.handler_name, request, defer_to_loop,
                [this, res, cancelled](const ResponseData& response) {
                    if (cancelled && *cancelled) {
                        return;
                    }
                    sendResponse(res, response);
                });
            return;
        }

        ResponseData response = matlabCaller_->callHandler(route.handler_name, request);
        if (cancelled && *cancelled) {
            return;
        }
        sendResponse(res, response);

    } catch (const std::exception& e) {
        if (!cancelled || !*cancelled) {
            sendErrorResponse(res, 500, "Internal Server Error");
        }
    } catch (...) {
        if (!cancelled || !*cancelled) {
            sendErrorResponse(res, 500, "Internal Server Error");
        }
    }
}

template<typename ResType>
void BlinkApp::sendResponse(ResType* res, const ResponseData& response) {
    // Status must be written before headers
    res->writeStatus(httpStatusLine(response.status_code));

    bool wrote_content_type = false;
    if (!response.content_type.empty()) {
        res->writeHeader("Content-Type", response.content_type);
        wrote_content_type = true;
    }

    for (const auto& [name, value] : response.headers) {
        if (wrote_content_type && headerNameEqualsIgnoreCase(name, "Content-Type")) {
            continue;
        }
        res->writeHeader(name, value);
        if (headerNameEqualsIgnoreCase(name, "Content-Type")) {
            wrote_content_type = true;
        }
    }

    if (!wrote_content_type) {
        res->writeHeader("Content-Type", "text/html");
    }

    res->end(response.body);
}

template<typename ResType>
void BlinkApp::sendErrorResponse(ResType* res, int statusCode, const std::string& message) {
    res->writeStatus(httpStatusLine(statusCode))->end(message);
}