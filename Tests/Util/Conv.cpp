#include "gtest/gtest.h"

#include <Stuff/Util/Conv.hpp>

TEST(Conv, IntFromChars) {
    int out = 0;
    const char* str = "-10";
    const auto res = Stf::from_chars(str, str + 3, out, 16);

    std::ignore = res;
}