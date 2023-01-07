#pragma once

namespace Stf::Detail::CEMaths {

template<typename T>
    requires requires(T v) {
                 { v == 0 } -> std::convertible_to<bool>;
                 { v > 0 } -> std::convertible_to<bool>;
                 { -v } -> std::convertible_to<T>;
             }
constexpr T abs(T v) {
    if (v == 0)
        return 0;

    return (v > 0) ? v : -v;
}

template<std::floating_point T> constexpr T fmax(T x, T y) { return x > y ? x : y; }

template<std::floating_point T> constexpr T fmin(T x, T y) { return x < y ? x : y; }

}

namespace Stf {

template<std::floating_point T> constexpr T abs(T v) {
    if consteval {
        return Detail::CEMaths::abs(v);
    } else {
        return std::abs(v);
    }
}

template<std::floating_point T> constexpr T fmax(T x, T y) {
    if consteval {
        return Detail::CEMaths::fmax(x, y);
    } else {
        return std::fmax(x, y);
    }
}

template<std::floating_point T> constexpr T fmin(T x, T y) {
    if consteval {
        return Detail::CEMaths::fmin(x, y);
    } else {
        return std::fmin(x, y);
    }
}

template<std::floating_point T> constexpr T fdim(T x, T y) { return fmax(0, x - y); }

template<std::floating_point T> constexpr bool is_close(T v, T u, T error = 0.00001) {
    if (is_inf(v) || is_inf(u))
        return false;

    return abs(v - u) < error;
}

template<typename T> constexpr T clamp(T v, T min, T max) { return fmin(max, fmax(min, v)); }

}
