#include "BlinkApp.hpp"
#include <gtest/gtest.h>
#include <memory>
#include <set>

// Simple mock MATLAB caller for testing
class MockMATLABCaller : public MATLABCaller {
public:
    ResponseData callHandler(const std::string& handlerName, const RequestData& request) override {
        ResponseData response;
        response.body = "Mock response from " + handlerName;
        response.statusCode = 200;
        response.contentType = "text/plain";
        return response;
    }
    
private:
    std::set<std::string> registeredHandlers;
};

TEST(BlinkAppTest, BasicCreation) {
    // Test that BlinkApp can be created successfully
    auto mockCaller = std::make_shared<MockMATLABCaller>();
    BlinkApp app(mockCaller);
    
    // If we get here without throwing, the test passes
    SUCCEED();
}

TEST(BlinkAppTest, BasicRouteHandling) {
    // Test basic route addition
    auto mockCaller = std::make_shared<MockMATLABCaller>();
    BlinkApp app(mockCaller);
    
    std::vector<Route> routes = {
        {"GET", "/hello", "helloHandler"}
    };
    
    // Should not throw
    EXPECT_NO_THROW(app.addRoutes(routes));
}
