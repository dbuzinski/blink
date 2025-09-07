#include "RequestParser.hpp"
#include <gtest/gtest.h>
#include <string>
#include <unordered_map>

// Mock request class for testing
class MockRequest {
public:
    MockRequest(const std::string& method, const std::string& url) 
        : method_(method), url_(url) {}
    
    const char* getMethod() const { return method_.c_str(); }
    const char* getUrl() const { return url_.c_str(); }
    
private:
    std::string method_;
    std::string url_;
};

TEST(RequestParserTest, ExtractPath) {
    EXPECT_EQ(RequestParser::extractPath("/api/users"), "/api/users");
    EXPECT_EQ(RequestParser::extractPath("/api/users?name=john"), "/api/users");
    EXPECT_EQ(RequestParser::extractPath("/api/users?name=john&age=25"), "/api/users");
    EXPECT_EQ(RequestParser::extractPath(""), "");
}

TEST(RequestParserTest, ExtractQueryString) {
    EXPECT_EQ(RequestParser::extractQueryString("/api/users"), "");
    EXPECT_EQ(RequestParser::extractQueryString("/api/users?name=john"), "name=john");
    EXPECT_EQ(RequestParser::extractQueryString("/api/users?name=john&age=25"), "name=john&age=25");
}

TEST(RequestParserTest, ParseQueryString) {
    auto params = RequestParser::parseQueryString("name=john&age=25&city=new%20york");
    
    EXPECT_EQ(params.size(), 3);
    EXPECT_EQ(params["name"], "john");
    EXPECT_EQ(params["age"], "25");
    EXPECT_EQ(params["city"], "new%20york");
}

TEST(RequestParserTest, ParseQueryStringEmpty) {
    auto params = RequestParser::parseQueryString("");
    EXPECT_TRUE(params.empty());
}

TEST(RequestParserTest, ParseQueryStringNoEquals) {
    auto params = RequestParser::parseQueryString("invalid&name=john");
    
    EXPECT_EQ(params.size(), 1);
    EXPECT_EQ(params["name"], "john");
}

TEST(RequestParserTest, ParseRequest) {
    MockRequest req("GET", "/api/users?name=john&age=25");
    
    RequestData data = RequestParser::parseRequest(&req, "request body");
    
    EXPECT_EQ(data.method, "GET");
    EXPECT_EQ(data.path, "/api/users");
    EXPECT_EQ(data.query, "name=john&age=25");
    EXPECT_EQ(data.body, "request body");
}

TEST(RequestParserTest, ParseRequestNoBody) {
    MockRequest req("POST", "/api/users");
    
    RequestData data = RequestParser::parseRequest(&req, nullptr);
    
    EXPECT_EQ(data.method, "POST");
    EXPECT_EQ(data.path, "/api/users");
    EXPECT_EQ(data.query, "");
    EXPECT_EQ(data.body, "");
}
