#pragma once

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

template<std::floating_point T>
constexpr T fmod(T x, T y);

template<std::floating_point T>
constexpr T remainder(T x, T y);

template<std::floating_point T>
constexpr T remquo(T x, T y, int* quo);

template<std::floating_point T>
constexpr T fmax(T x, T y);

template<std::floating_point T>
constexpr T fmin(T x, T y);

template<std::floating_point T>
constexpr T trunc(T arg);

//custom ones

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
