#include <gtest/gtest.h>

#include <cmath>

#include <Stuff/Maths/Maths.hpp>
#include <Stuff/Maths/Scalar.hpp>

#define SASSERT_FALSE(condition) \
    static_assert(!condition);   \
    ASSERT_FALSE(condition)

#define SASSERT_TRUE(condition) \
    static_assert(condition);   \
    ASSERT_TRUE(condition)

#define SASSERT_EQ(lhs, rhs)   \
    static_assert(lhs == rhs); \
    ASSERT_EQ(lhs, rhs)

TEST(Scalar, Utility) {
    const auto abs_0 = Stf::abs(-1.);
    const auto abs_1 = Stf::abs(-0.f);
    const auto abs_2 = Stf::abs(-1);

    ASSERT_FLOAT_EQ(abs_0, 1.);
    ASSERT_FLOAT_EQ(abs_1, 0.f);
    ASSERT_EQ(abs_2, 1);
    SASSERT_TRUE((std::is_same_v<int, std::decay_t<decltype(abs_2)>>));

    SASSERT_TRUE(Stf::is_close(-1.000001, -0.999999));

    SASSERT_EQ(Stf::pow(1.5f, 2), 2.25f);
    SASSERT_EQ(Stf::pow(1.5f, -2), 1.f/2.25f);
}

TEST(Float, Classification) {
    using FloatLims = std::numeric_limits<float>;
    using Parts = Stf::Detail::FloatParts<float>;

    constexpr float f_0 = 3.141592502593994140625f;
    constexpr auto parts_0 = Stf::float_to_parts(f_0);

    SASSERT_EQ(parts_0.sign, false);
    SASSERT_EQ(parts_0.exponent, 128u);
    SASSERT_EQ(parts_0.fraction, 4788186u);

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
    SASSERT_FALSE(Stf::is_finite(Stf::exp(800.f)));

    SASSERT_TRUE(Stf::is_inf(inf_f));
    SASSERT_TRUE(Stf::is_inf(Stf::exp(800.f)));
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

TEST(Float, Manipulation) {
    constexpr auto inf = std::numeric_limits<float>::infinity();

    // NOTICE: ldexp is not compliant as commented above it's definition

    SASSERT_EQ(Stf::ldexp(7.f, -4), .4375f);
    SASSERT_EQ(Stf::ldexp(-0.f, 10), -0.f);
    SASSERT_EQ(Stf::ldexp(-inf, 10), -inf);
    SASSERT_EQ(Stf::ldexp(inf, 10), inf);
    SASSERT_EQ(Stf::ldexp(1.f, 128), inf);
}

TEST(Scalar, Approximations) {
    static const constinit auto e_to_pi = Stf::exp(M_PI);
    static const constinit auto cos_of_1 = Stf::cos(1.);
    static const constinit auto sin_of_e = Stf::sin(M_E);
    static const constinit auto one_half_to_4
        = Stf::Detail::Pow::pow_impl<float, Stf::Detail::Pow::IntegralExponent<int>>(0.5f, { 4 }, 1.f, 0);

    ASSERT_FLOAT_EQ(23.140692632779, e_to_pi);
    ASSERT_FLOAT_EQ(std::pow(M_E, M_PI), e_to_pi);

    ASSERT_FLOAT_EQ(0.540302305868139, cos_of_1);
    ASSERT_FLOAT_EQ(std::cos(1.), cos_of_1);

    ASSERT_FLOAT_EQ(0.410781290502908, sin_of_e);
    ASSERT_FLOAT_EQ(std::sin(M_E), sin_of_e);

    ASSERT_FLOAT_EQ(0.0625f, one_half_to_4);
}
