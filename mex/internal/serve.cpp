#include "mex.hpp"
#include "mexAdapter.hpp"
#include "BlinkApp.hpp"

class MexFunction : public matlab::mex::Function {
private:
    std::shared_ptr<matlab::engine::MATLABEngine> matlabPtr = getEngine();
    matlab::data::ArrayFactory factory;
public:
    void operator()(matlab::mex::ArgumentList outputs, matlab::mex::ArgumentList inputs) {
        if (inputs.size() < 2) {
            matlabPtr->feval(u"error", 0,
                std::vector<matlab::data::Array>({
                    factory.createScalar("At least two input arguments (port number and routes) are required.")
            }));
            return;
        }

        if (inputs[0].getType() != matlab::data::ArrayType::DOUBLE) {
            matlabPtr->feval(u"error", 0,
                std::vector<matlab::data::Array>({
                    factory.createScalar("Port number must be a positive integer.")
            }));
            return;
        }

        if (inputs[1].getType() != matlab::data::ArrayType::VALUE_OBJECT) {
            matlabPtr->feval(u"error", 0,
                std::vector<matlab::data::Array>({
                    factory.createScalar("Routes must be an array of 'cinch.Route' objects.")
            }));
            return;
        }

        BlinkApp app = BlinkApp(matlabPtr);
        double port = inputs[0][0];
        app.addRoutes(inputs[1]);
        if (inputs.size() == 4) {
            std::string staticFiles = static_cast<matlab::data::CharArray>(inputs[2]).toAscii();
            std::string staticRoute = static_cast<matlab::data::CharArray>(inputs[3]).toAscii();
            app.addStaticFiles(staticFiles, staticRoute);
        }
        app.ws.listen(static_cast<int>(port), [](auto *listen_socket) {}).run();
        std::cout << "Failed to start webserver" << std::endl;
    }
};
