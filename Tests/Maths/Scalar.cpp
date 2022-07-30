#include <gtest/gtest.h>

#include <cmath>

#include <Stuff/Maths/Maths.hpp>
#include <Stuff/Maths/Scalar.hpp>
#include <Stuff/Util/Tuple.hpp>

#define SASSERT_FALSE(condition) \
    static_assert(!condition);   \
    ASSERT_FALSE(condition)

#define SASSERT_TRUE(condition) \
    static_assert(condition);   \
    ASSERT_TRUE(condition)

#define SASSERT_EQ(lhs, rhs)   \
    static_assert(lhs == rhs); \
    ASSERT_EQ(lhs, rhs)

TEST(Scalar, Basic) {
    SASSERT_TRUE(Stf::is_close(-1.000001, -0.999999));

    ASSERT_FLOAT_EQ(Stf::abs(-1.), 1.);
    ASSERT_FLOAT_EQ(Stf::abs(-0.f), 0.f);
    ASSERT_EQ(Stf::abs(-1), 1);
}

TEST(Scalar, Classification) {
    using FloatLims = std::numeric_limits<float>;

    constexpr auto nan_f = FloatLims::quiet_NaN();
    constexpr auto inf_f = FloatLims::infinity();
    constexpr auto min_f = FloatLims::min();

    ASSERT_TRUE(Stf::is_nan(nan_f));
    ASSERT_TRUE(Stf::is_nan(0.f / 0.f));
    ASSERT_TRUE(Stf::is_nan(inf_f - inf_f));
    SASSERT_FALSE(Stf::is_nan(inf_f));
    SASSERT_FALSE(Stf::is_nan(0.f));
    SASSERT_FALSE(Stf::is_nan(min_f / 2.f));

    SASSERT_TRUE(Stf::is_finite(0.f));
    SASSERT_TRUE(Stf::is_finite(min_f / 2.f));
    SASSERT_FALSE(Stf::is_finite(NAN));
    SASSERT_FALSE(Stf::is_finite(inf_f));
    ASSERT_FALSE(Stf::is_finite(Stf::exp(800.f)));

    SASSERT_TRUE(Stf::is_inf(inf_f));
    ASSERT_TRUE(Stf::is_inf(Stf::exp(800.f)));
    SASSERT_FALSE(Stf::is_inf(0.f));
    SASSERT_FALSE(Stf::is_inf(min_f / 2.f));
    SASSERT_FALSE(Stf::is_inf(NAN));

    SASSERT_TRUE(Stf::is_normal(1.f));
    SASSERT_FALSE(Stf::is_normal(0.f));
    SASSERT_FALSE(Stf::is_normal(nan_f));
    SASSERT_FALSE(Stf::is_normal(inf_f));
    SASSERT_FALSE(Stf::is_normal(min_f / 2.f));
    SASSERT_FALSE(Stf::is_normal(9.18340948595e-41f));
    SASSERT_FALSE(Stf::is_normal(7.34682567965e-40f));
    SASSERT_FALSE(Stf::is_normal(5.87747035281e-39f));
    SASSERT_FALSE(Stf::is_normal(1.17549421069e-38f));
}

template<size_t N, typename Ret, typename... Args>
using TestVectorArray = std::array<std::tuple<std::string_view, Ret (*)(Args...), Ret (*)(Args...), std::tuple<Args...>, Ret>, N>;

template<size_t N, typename Ret, typename... Args> static void vector_test(TestVectorArray<N, Ret, Args...> const& vec) {
    for (auto [name, test_fn, reference_fn, test_arguments, reference_val] : vec) {
        double lhs = Stf::tuple_call(test_fn, test_arguments);
        double rhs = reference_fn != nullptr ? Stf::tuple_call(reference_fn, test_arguments) : reference_val;

        if (!Stf::is_close(lhs, rhs)) {
            fmt::print("Mathematical function test named '{}' failed.\n", name);
            ASSERT_FLOAT_EQ(lhs, rhs);
        }
    }
}

TEST(Scalar, Exponential) {
    TestVectorArray<3, double, double> test_vector { {
        { "e^0", Stf::exp<double>, std::exp, 0.001, 1 },
        { "e^1", Stf::exp<double>, std::exp, 1, M_E },
        { "e^pi", Stf::exp<double>, std::exp, M_PI, 23.140692632779 },
    } };

    vector_test(test_vector);
}

TEST(Scalar, FloatUtils) {
    using FloatLims = std::numeric_limits<float>;
    using Parts = Stf::FloatParts<float>;

    constexpr float f_0 = 3.141592502593994140625f;
    constexpr auto parts_0 = Stf::float_to_parts(f_0);

    SASSERT_EQ(parts_0.sign, false);
    SASSERT_EQ(parts_0.exponent, 128u);
    SASSERT_EQ(parts_0.fraction, 4788186u);
}

TEST(Scalar, Power) {
    auto pow = [](float x, int y) -> float {
        return std::pow(x, y);
    };

    TestVectorArray<2, float, float, int> test_vector { {
        { "1.5 ** 2", Stf::pow, pow, {1.5f, 2}, 2.25f },
        { "1.5 ** -2", Stf::pow, pow, {1.5f, -2}, 1.f/2.25f },
    } };

    vector_test(test_vector);
}

TEST(Scalar, Approximations) {
    const auto cos_of_1 = Stf::cos(1.);
    const auto sin_of_e = Stf::sin(M_E);
    const auto one_half_to_4 = Stf::pow(0.5f, 4);

    ASSERT_FLOAT_EQ(0.540302305868139, cos_of_1);
    ASSERT_FLOAT_EQ(std::cos(1.), cos_of_1);

    ASSERT_FLOAT_EQ(0.410781290502908, sin_of_e);
    ASSERT_FLOAT_EQ(std::sin(M_E), sin_of_e);

    ASSERT_FLOAT_EQ(0.0625f, one_half_to_4);
}
