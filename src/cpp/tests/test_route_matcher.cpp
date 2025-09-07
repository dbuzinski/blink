#include "RouteMatcher.hpp"
#include <gtest/gtest.h>
#include <string>
#include <unordered_map>

TEST(RouteMatcherTest, SplitPath) {
    auto segments = RouteMatcher::splitPath("/api/users/123");
    EXPECT_EQ(segments.size(), 3);
    EXPECT_EQ(segments[0], "api");
    EXPECT_EQ(segments[1], "users");
    EXPECT_EQ(segments[2], "123");
}

TEST(RouteMatcherTest, SplitPathEmpty) {
    auto segments = RouteMatcher::splitPath("");
    EXPECT_TRUE(segments.empty());
}

TEST(RouteMatcherTest, SplitPathRoot) {
    auto segments = RouteMatcher::splitPath("/");
    EXPECT_TRUE(segments.empty());
}

TEST(RouteMatcherTest, IsParameter) {
    EXPECT_TRUE(RouteMatcher::isParameter(":id"));
    EXPECT_TRUE(RouteMatcher::isParameter(":userId"));
    EXPECT_FALSE(RouteMatcher::isParameter("users"));
    EXPECT_FALSE(RouteMatcher::isParameter(""));
}

TEST(RouteMatcherTest, ExtractParameterName) {
    EXPECT_EQ(RouteMatcher::extractParameterName(":id"), "id");
    EXPECT_EQ(RouteMatcher::extractParameterName(":userId"), "userId");
    EXPECT_EQ(RouteMatcher::extractParameterName("users"), "");
}

TEST(RouteMatcherTest, Matches) {
    EXPECT_TRUE(RouteMatcher::matches("/api/users", "/api/users"));
    EXPECT_TRUE(RouteMatcher::matches("/api/users/:id", "/api/users/123"));
    EXPECT_TRUE(RouteMatcher::matches("/api/users/:id/posts/:postId", "/api/users/123/posts/456"));
    
    EXPECT_FALSE(RouteMatcher::matches("/api/users", "/api/posts"));
    EXPECT_FALSE(RouteMatcher::matches("/api/users/:id", "/api/users/123/posts"));
    EXPECT_FALSE(RouteMatcher::matches("/api/users", "/api/users/123"));
}

TEST(RouteMatcherTest, ParseRouteParameters) {
    auto params = RouteMatcher::parseRouteParameters("/api/users/:id", "/api/users/123");
    
    EXPECT_EQ(params.size(), 1);
    EXPECT_EQ(params["id"], "123");
}

TEST(RouteMatcherTest, ParseRouteParametersMultiple) {
    auto params = RouteMatcher::parseRouteParameters("/api/users/:id/posts/:postId", "/api/users/123/posts/456");
    
    EXPECT_EQ(params.size(), 2);
    EXPECT_EQ(params["id"], "123");
    EXPECT_EQ(params["postId"], "456");
}

TEST(RouteMatcherTest, ParseRouteParametersNoMatch) {
    auto params = RouteMatcher::parseRouteParameters("/api/users/:id", "/api/posts/123");
    
    EXPECT_TRUE(params.empty());
}

TEST(RouteMatcherTest, ParseRouteParametersNoParameters) {
    auto params = RouteMatcher::parseRouteParameters("/api/users", "/api/users");
    
    EXPECT_TRUE(params.empty());
}
