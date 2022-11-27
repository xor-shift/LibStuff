#include <gtest/gtest.h>

#include <Stuff/Maths/Scalar.hpp>
#include <Stuff/Maths/BLAS/Vector.hpp>
#include <Stuff/Maths/Fmt.hpp>

TEST(Vector, BasicExpressions) {
    const auto vec_0 = Stf::vector<float>(1.f, 2.f, 3.f);
    auto vec_1 = Stf::vector<float>(1.f, 1.f, -0.5f);

    ASSERT_EQ(std::tuple_size_v<decltype(vec_0)>, 3);
    ASSERT_TRUE((std::is_same_v<std::tuple_element_t<0, decltype(vec_0)>, const float>));
    ASSERT_TRUE((std::is_same_v<std::tuple_element_t<1, decltype(vec_0)>, const float>));
    ASSERT_TRUE((std::is_same_v<std::tuple_element_t<2, decltype(vec_0)>, const float>));
    ASSERT_EQ(get<1>(vec_0), 2.f);
    ASSERT_EQ(get<2>(vec_0), 3.f);

    ASSERT_EQ(std::tuple_size_v<decltype(vec_1)>, 3);
    ASSERT_TRUE((std::is_same_v<std::tuple_element_t<0, decltype(vec_1)>, float>));
    ASSERT_TRUE((std::is_same_v<std::tuple_element_t<1, decltype(vec_1)>, float>));
    ASSERT_TRUE((std::is_same_v<std::tuple_element_t<2, decltype(vec_1)>, float>));
    ASSERT_EQ(get<0>(vec_1), 1.f);
    ASSERT_EQ(get<1>(vec_1), 1.f);
    ASSERT_EQ(get<2>(vec_1), -.5f);

    const auto [x_0, y_0, z_0] = vec_0;
    const auto [x_1, y_1, z_1] = vec_1;
    ASSERT_EQ(x_0, 1.f);
    ASSERT_EQ(y_0, 2.f);
    ASSERT_EQ(z_0, 3.f);
    ASSERT_EQ(x_1, 1.f);
    ASSERT_EQ(y_1, 1.f);
    ASSERT_EQ(z_1, -.5f);

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
