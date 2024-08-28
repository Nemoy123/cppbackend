#include <gtest/gtest.h>

#include "../src/urlencode.h"

using namespace std::literals;

TEST(UrlEncodeTestSuite, OrdinaryCharsAreNotEncoded) {
    EXPECT_EQ(UrlEncode("hello"sv), "hello"s);
    EXPECT_EQ(UrlEncode(""sv), ""s);
    EXPECT_EQ(UrlEncode("Hello World!"sv), "Hello+World%21"s);
}

/* Напишите остальные тесты самостоятельно */
