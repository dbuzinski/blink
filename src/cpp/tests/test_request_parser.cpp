#include "RequestParser.hpp"
#include <gtest/gtest.h>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

// Minimal stand-in for uWebSockets `HttpRequest` header iteration (`begin`/`end`).
class MockRequest {
public:
    MockRequest(const std::string& method, const std::string& url,
                const std::string& query = "",
                std::vector<std::pair<std::string, std::string>> headers = {})
        : method_(method), url_(std::move(url)), query_(std::move(query)), headers_(std::move(headers)) {}

    const char* getMethod() const { return method_.c_str(); }
    const char* getUrl() const { return url_.c_str(); }
    const char* getQuery() const { return query_.c_str(); }

    struct Iterator {
        const std::vector<std::pair<std::string, std::string>>* vec = nullptr;
        size_t index = 0;

        bool operator!=(const Iterator& other) const { return index != other.index; }

        Iterator& operator++() {
            ++index;
            return *this;
        }

        std::pair<std::string_view, std::string_view> operator*() const {
            const auto& p = (*vec)[index];
            return {p.first, p.second};
        }
    };

    Iterator begin() { return Iterator{&headers_, 0}; }
    Iterator end() { return Iterator{&headers_, headers_.size()}; }

private:
    std::string method_;
    std::string url_;
    std::string query_;
    std::vector<std::pair<std::string, std::string>> headers_;
};

TEST(RequestParserTest, ParseQueryString) {
    auto params = RequestParser::parseQueryString("name=john&age=25&city=new%20york");
    
    EXPECT_EQ(params.size(), 3);
    EXPECT_EQ(params["name"], "john");
    EXPECT_EQ(params["age"], "25");
    EXPECT_EQ(params["city"], "new york");
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
    MockRequest req("GET", "/api/users", "name=john&age=25");
    
    RequestData data = RequestParser::parseRequest(&req, "request body");
    
    EXPECT_EQ(data.method, "GET");
    EXPECT_EQ(data.path, "/api/users");
    EXPECT_EQ(data.query, "name=john&age=25");
    EXPECT_EQ(data.body, "request body");
}

TEST(RequestParserTest, ParseRequestNoBody) {
    MockRequest req("POST", "/api/users");
    
    RequestData data = RequestParser::parseRequest(&req, "");
    
    EXPECT_EQ(data.method, "POST");
    EXPECT_EQ(data.path, "/api/users");
    EXPECT_EQ(data.query, "");
    EXPECT_EQ(data.body, "");
    EXPECT_TRUE(data.headers.empty());
}

TEST(RequestParserTest, ParseRequestCollectsHeaders) {
    MockRequest req("GET", "/x", "", {{"user-agent", "gtest"}, {"accept", "text/plain"}});

    RequestData data = RequestParser::parseRequest(&req, "");

    ASSERT_EQ(data.headers.size(), 2u);
    EXPECT_EQ(data.headers["user-agent"], "gtest");
    EXPECT_EQ(data.headers["accept"], "text/plain");
}

TEST(RequestParserTest, MatlabFieldNameToHttpHeaderName) {
    EXPECT_EQ(RequestParser::matlabFieldNameToHttpHeaderName("Content_Type"), "Content-Type");
    EXPECT_EQ(RequestParser::matlabFieldNameToHttpHeaderName("Location"), "Location");
    EXPECT_EQ(RequestParser::matlabFieldNameToHttpHeaderName("X_Custom_Header"), "X-Custom-Header");
}

TEST(RequestParserTest, QueryKeyToMatlabFieldName) {
    EXPECT_EQ(RequestParser::queryKeyToMatlabFieldName("foo"), "foo");
    EXPECT_EQ(RequestParser::queryKeyToMatlabFieldName("foo_bar"), "foo_bar");
    EXPECT_EQ(RequestParser::queryKeyToMatlabFieldName("foo-bar"), "foo_bar");
    EXPECT_EQ(RequestParser::queryKeyToMatlabFieldName("123"), "x123");
    EXPECT_EQ(RequestParser::queryKeyToMatlabFieldName(""), "empty");
}

TEST(RequestParserTest, ParseQueryStringThenMatlabFieldNames) {
    auto raw = RequestParser::parseQueryString("page=1&sort=name&foo-bar=z");
    std::unordered_map<std::string, std::string> byField;
    for (const auto& [k, v] : raw) {
        byField[RequestParser::queryKeyToMatlabFieldName(k)] = v;
    }
    EXPECT_EQ(byField["page"], "1");
    EXPECT_EQ(byField["sort"], "name");
    EXPECT_EQ(byField["foo_bar"], "z");
}
