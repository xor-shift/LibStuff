#pragma once

#include <array>

namespace Stf::Detail::CEMaths {

/// TODO: this is most definitely not standards compliant
template<std::floating_point T, std::integral U> constexpr T pow(T base, U iexp) {
    const bool neg_exp = iexp < 0;
    const bool pos_exp = iexp > 0;
    const bool even_exp = (iexp % 2) == 0;
    const bool odd_exp = !even_exp;
    const bool zero_exp = iexp == 0;
    const bool nz_exp = !zero_exp;

    const bool neg_base = sign_bit(base);
    const bool pos_base = !neg_base;
    const bool zero_base = base == 0;
    const bool nz_base = !zero_base;
    const bool inf_base = is_inf(base);

    const auto inf = std::numeric_limits<T>::infinity();

    // pow(+1, exp) returns 1 for any exp, even when exp is NaN
    // pow(base, ±0) returns 1 for any base, even when base is NaN
    if (base == 1 || iexp == 0)
        return 1;

    const std::pair<std::array<bool, 4>, T> exceptions[] {
        // pow(+0, exp), where exp is a negative odd integer, returns +∞ and raises FE_DIVBYZERO
        { { zero_base, pos_base, neg_exp, odd_exp }, inf },
        // pow(-0, exp), where exp is a negative odd integer, returns -∞ and raises FE_DIVBYZERO
        { { zero_base, neg_base, neg_exp, odd_exp }, -inf },
        // pow(±0, exp), where exp is negative, finite, and is an even integer or a non-integer, returns +∞ and raises FE_DIVBYZERO
        { { zero_base, true, neg_exp, even_exp }, inf },
        // pow(+0, exp), where exp is a positive odd integer, returns +0
        { { zero_base, pos_base, pos_exp, odd_exp }, static_cast<T>(+0.f) },
        // pow(-0, exp), where exp is a positive odd integer, returns -0
        { { zero_base, neg_base, pos_exp, odd_exp }, static_cast<T>(-0.f) },
        // pow(±0, exp), where exp is positive non-integer or a positive even integer, returns +0
        { { zero_base, true, pos_exp, even_exp }, static_cast<T>(+0.f) },
        // pow(-∞, exp) returns -0 if exp is a negative odd integer
        { { inf_base, neg_base, neg_exp, odd_exp }, static_cast<T>(-0.f) },
        // pow(-∞, exp) returns +0 if exp is a negative non-integer or negative even integer
        { { inf_base, neg_base, neg_exp, even_exp }, static_cast<T>(+0.f) },
        // pow(-∞, exp) returns -∞ if exp is a positive odd integer
        { { inf_base, neg_base, pos_exp, odd_exp }, -inf },
        // pow(-∞, exp) returns +∞ if exp is a positive non-integer or positive even integer
        { { inf_base, neg_base, pos_exp, even_exp }, inf },
        // pow(+∞, exp) returns +0 for any negative exp
        { { inf_base, pos_base, neg_exp, true }, static_cast<T>(+0.f) },
        // pow(+∞, exp) returns +∞ for any positive exp
        { { inf_base, pos_base, pos_exp, true }, inf },
    };

    for (auto [conditions, ret_val] : exceptions) {
        if (std::all_of(conditions.cbegin(), conditions.cend(), [](auto v) { return v; }))
            return ret_val;
    }

    // skipped exceptions:
    // pow(-1, ±∞) returns 1
    // pow(base, -∞) returns +∞ for any |base|<1
    // pow(base, -∞) returns +0 for any |base|>1
    // pow(base, +∞) returns +0 for any |base|<1
    // pow(base, +∞) returns +∞ for any |base|>1

    if (abs(iexp) < 0) { //handle the negative inf exponent cases here
        if (abs(base) < 1)
            return inf;
        if (abs(base) > 1)
            return 0;
        //zero case is handled above
        std::unreachable();
    }

    // except where specified above, if any argument is NaN, NaN is returned
    if (is_nan(base))
        return base;

    using E = std::make_unsigned_t<U>;
    const E exponent = abs(iexp);

    const int e_bits = std::numeric_limits<E>::digits - std::countl_zero(exponent);
    const bool negative_exponent = iexp < 0;

    T ret = 1;
    for (size_t i = 0; i < e_bits; i++) {
        ret *= ret;

        if ((exponent & (static_cast<E>(1) << (e_bits - i - 1))) != 0)
            ret *= base;
    }

    if (negative_exponent)
        ret = 1 / ret;

    return ret;
}

//static_assert(pow(2.f, 10) == 1024.f);

}

namespace Stf {

template<std::floating_point T, std::integral U> constexpr T pow(T base, U iexp) {
    if consteval {
        return Detail::CEMaths::pow(base, iexp);
    } else {
        return std::pow(base, iexp);
    }
}

}
