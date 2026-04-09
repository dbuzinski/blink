#include "HttpStatus.hpp"
#include <gtest/gtest.h>

TEST(HttpStatusTest, KnownCodes) {
    EXPECT_EQ(httpStatusLine(200), "200 OK");
    EXPECT_EQ(httpStatusLine(404), "404 Not Found");
    EXPECT_EQ(httpStatusLine(500), "500 Internal Server Error");
}

TEST(HttpStatusTest, UnknownCodeUsesNumericPrefix) {
    EXPECT_EQ(httpStatusLine(418), "418 ");
}
