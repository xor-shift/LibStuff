#pragma once

#include <concepts>

#include "./Scalar.hpp"

namespace Stf::Detail::FP::Other {

template<typename T>
    requires requires(T v) { // weird gcc issues, conflict with abs for vectors
                 { v == 0 } -> std::convertible_to<bool>;
                 { v > 0 } -> std::convertible_to<bool>;
                 { -v } -> std::convertible_to<T>;
             }
constexpr T abs(T v) {
    if (v == 0)
        return 0;
    return (v > 0) ? v : -v;
}

template<typename T> constexpr bool is_close(T v, T u, T error = 0.00001) {
    if (is_inf(v) || is_inf(u))
        return false;
    return abs(v - u) < error;
}

template<typename U, typename T> constexpr std::pair<U, U> to_rational(T real, T error = 0.0000001) { }

template<typename T> constexpr T exp(T x, T target_delta = static_cast<T>(0.00001), size_t max_rounds = 50) {
    T ret = 0;
    T term = 1;

    for (size_t k = 1;;) {
        ret += term;
        const auto temp = term * (x / static_cast<T>(k));

        if (is_close(temp, term, target_delta) || ++k >= max_rounds)
            break;

        term = temp;
    }

    return ret;
}

template<typename T> constexpr T inv_lerp(T v, T a, T b) {
    if (a == b)
        return 0;

    return (v - a) / (b - a);
}

template<typename T> constexpr T lerp(T t, T a, T b) {
    if ((a < 0 && b > 0) || (a > 0 && b < 0))
        return (1 - t) * a + t * b;

    if (t == 1)
        return b;

    return t * (b - a) + a;
}

template<typename T>
constexpr std::enable_if_t<std::is_floating_point_v<T>, T> map(T v, T from_a, T from_b, T to_a, T to_b) {
    const auto t = inv_lerp(v, from_a, from_b);
    return lerp(t, to_a, to_b);
}

template<typename T> constexpr std::enable_if_t<std::is_integral_v<T>, T> map(T v, T from_a, T from_b, T to_a, T to_b) {
    // TODO: this is a mess

    using U = std::conditional_t<sizeof(T) >= sizeof(uint64_t), long double,
        std::conditional_t<sizeof(T) >= sizeof(uint32_t), double, float>>;

    return static_cast<T>(map<U>(v, from_a, from_b, to_a, to_b));
}

template<typename T> constexpr T clamp(T v, T min, T max) {
    if (v <= min)
        return min;

    if (v >= max)
        return max;

    return v;
}

}

namespace Stf {

using Detail::FP::Other::abs;
using Detail::FP::Other::is_close;
using Detail::FP::Other::exp;
using Detail::FP::Other::inv_lerp;
using Detail::FP::Other::lerp;
using Detail::FP::Other::map;
using Detail::FP::Other::clamp;

}
