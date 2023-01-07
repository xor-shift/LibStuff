#pragma once

#include <algorithm>
#include <bit>
#include <climits>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <utility>

#include "Scalar/FloatUtils.hpp"

#include "Scalar/Basic.hpp"
#include "Scalar/Classification.hpp"
#include "Scalar/Exponential.hpp"
#include "Scalar/Interpolation.hpp"
#include "Scalar/Manipulation.hpp"
#include "Scalar/Power.hpp"
#include "Scalar/Trig.hpp"

namespace Stf::Detail::Pow {

template<typename T>
concept ExponentType = requires(T v, typename T::abs_type a, size_t i) {
                           { v.abs() } -> std::convertible_to<typename T::abs_type>;
                           { v.is_zero() } -> std::convertible_to<bool>;
                           { v.is_negative() } -> std::convertible_to<bool>;
                           { a.bits() } -> std::convertible_to<size_t>;
                           { a.bit_is_set(i) } -> std::convertible_to<bool>;
                       };

template<typename T> struct IntegralExponent;

template<std::unsigned_integral T> struct IntegralExponent<T> {
    using abs_type = IntegralExponent<T>;

    T v;

    constexpr size_t is_zero() const { return v == 0; }

    constexpr abs_type abs() const { return *this; }

    constexpr bool is_negative() const { return false; }

    constexpr size_t bits() const {
        constexpr auto e_bits_total = sizeof(T) * CHAR_BIT;
        const auto e_clz = __builtin_clz(v);
        const auto e_bits = e_bits_total - e_clz;
        return e_bits;
    }

    constexpr bool bit_is_set(size_t i) const { return (v & static_cast<T>(size_t { 1 } << i)) != 0; }
};

template<std::signed_integral T> struct IntegralExponent<T> {
    using abs_type = IntegralExponent<std::make_unsigned_t<T>>;

    T v;

    constexpr size_t is_zero() const { return v == 0; }

    constexpr abs_type abs() const { return abs_type { static_cast<std::make_unsigned_t<T>>(abs(v)) }; }

    constexpr bool is_negative() const { return v < 0; }

    constexpr bool bit_is_set(size_t i) const { return (v & static_cast<T>(size_t { 1 } << i)) != 0; }
};

template<typename B, ExponentType E, bool use_mod = !std::is_floating_point_v<B>>
constexpr B pow_impl(B b, E e, B mult_identity, auto mod) {
    if (e.is_zero())
        return mult_identity;

    auto exponent = e.abs();
    const bool neg_exp = e.is_negative();
    const auto e_bits = exponent.bits();

    B ret = mult_identity;
    B base = b;
    if constexpr (use_mod)
        base %= mod;

    for (size_t i = 0; i < e_bits; i++) {
        if constexpr (use_mod) {
            ret = (ret * ret) % mod;
        } else {
            ret = ret * ret;
        }

        if (exponent.bit_is_set(e_bits - i - 1)) {
            if constexpr (use_mod) {
                ret = (ret % mod * base) % mod;
            } else {
                ret = ret * base;
            }
        }
    }

    if (neg_exp)
        return static_cast<B>(1) / ret;
    return ret;
}

}
