#pragma once

#include <limits>

#include "./Classification.hpp"

namespace Stf {
template<typename T>
    requires requires(T v) { // weird gcc issues, conflict with abs for Stf::Vector
                 { v == 0 } -> std::convertible_to<bool>;
                 { v > 0 } -> std::convertible_to<bool>;
                 { -v } -> std::convertible_to<T>;
             }
constexpr T abs(T v) {
    if (v == 0)
        return 0;
    return (v > 0) ? v : -v;
}

template<std::floating_point T> constexpr T fmod(T x, T y);

template<std::floating_point T> constexpr T remainder(T x, T y);

template<std::floating_point T> constexpr T remquo(T x, T y, int* quo);

template<std::floating_point T> constexpr T fmax(T x, T y) {
    if (is_nan(x) && is_nan(y))
        return std::numeric_limits<T>::quiet_NaN();

    if (is_nan(x))
        return y;
    if (is_nan(y))
        return x;

    if (x == 0 && y == 0) {
        if (!sign_bit(x) || !sign_bit(y))
            return static_cast<T>(0.f);
        return static_cast<T>(-0.f);
    }

    return x > y ? x : y;
}

template<std::floating_point T> constexpr T fmin(T x, T y) {
    if (is_nan(x) && is_nan(y))
        return std::numeric_limits<T>::quiet_NaN();

    if (is_nan(x))
        return y;
    if (is_nan(y))
        return x;

    if (x == 0 && y == 0) {
        if (sign_bit(x) || sign_bit(y))
            return static_cast<T>(-0.f);
        return static_cast<T>(0.f);
    }

    return y > x ? x : y;
}

template<std::floating_point T> constexpr T trunc(T arg);

template<std::floating_point T> constexpr T fdim(T x, T y) {
    if (is_nan(x) || is_nan(y))
        return std::numeric_limits<T>::quiet_NaN();

    return fmax(x - y, 0);
}

// custom ones

template<std::floating_point T> constexpr bool is_close(T v, T u, T error = 0.00001) {
    if (is_inf(v) || is_inf(u))
        return false;
    return abs(v - u) < error;
}

template<typename T> constexpr T clamp(T v, T min, T max) {
    if (v <= min)
        return min;

    if (v >= max)
        return max;

    return v;
}

}
