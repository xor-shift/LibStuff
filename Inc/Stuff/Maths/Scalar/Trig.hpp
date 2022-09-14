#pragma once

#include "./FloatUtils.hpp"

namespace Stf {

template<typename T> constexpr T sin(T x) {
    const T target_delta = static_cast<T>(0.000001);
    const size_t max_rounds = 50;

    const auto k = std::round(x * std::numbers::inv_pi_v<T> * static_cast<T>(0.5));
    x -= k * static_cast<T>(2) * std::numbers::pi_v<T>;
    //sin(x) = sin(x - 2pi * round(x/2pi))

    T ret = 0;
    T term = x;

    for (size_t k = 0; k < max_rounds; k++) {
        if ((k % 2) != 0)
            ret -= term;
        else
            ret += term;

        const auto divisor = (4 * k * k + 10 * k + 6);
        const auto temp = term * (x * x / static_cast<T>(divisor));

        if (is_close(temp, term, target_delta))
            break;

        term = temp;
    }

    return ret;
}

template<typename T> constexpr T cos(T x) {
    return sin(M_PI_2 + x);
}

template<typename T> constexpr T tan(T x) { return sin(x) / cos(x); }

template<typename T> constexpr T cot(T x) { return cos(x) / sin(x); }

template<typename T> constexpr T sec(T x) { return static_cast<T>(1) / cos(x); }

template<typename T> constexpr T csc(T x) { return static_cast<T>(1) / sin(x); }

}
