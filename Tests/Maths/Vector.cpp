#include <gtest/gtest.h>

#include <Stuff/Maths/Maths.hpp>

TEST(Vector, BasicExpressions) {
    const auto vec_0 = Stf::vector<float>(1, 2, 3);
    const auto vec_1 = Stf::vector<float>(1, 1, -0.5);

    const auto expr = Stf::abs((vec_0 + vec_0) * vec_1);

    const auto expected = Stf::vector<float>(2, 4, 3);

    for (size_t i = 0; i < 3; i++) {
        ASSERT_TRUE(Stf::is_close(expr[i], expected[i]));
        ASSERT_FLOAT_EQ(expr[i], expected[i]);
    }

    std::array<char, 64> arr {};
    fmt::format_to_n(arr.begin(), arr.size(), "{}", vec_0);
    std::ignore = arr;
}
