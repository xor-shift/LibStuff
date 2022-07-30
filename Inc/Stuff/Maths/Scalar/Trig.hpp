#pragma once

#include "./FloatUtils.hpp"

namespace Stf {

template<typename T> constexpr T sin(T x, T target_delta = static_cast<T>(0.00001), size_t max_rounds = 0) {
    T ret = 0;
    T term = x;

    for (size_t k = 0; max_rounds == 0 ? true : k < max_rounds; k++) {
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

template<typename T> constexpr T cos(T x, T target_delta = static_cast<T>(0.00001), size_t max_rounds = 0) {
    return sin(M_PI_2 + x, target_delta, max_rounds);
}

template<typename T> constexpr T tan(T x) { return sin(x) / cos(x); }

template<typename T> constexpr T cot(T x) { return cos(x) / sin(x); }

template<typename T> constexpr T sec(T x) { return static_cast<T>(1) / cos(x); }

template<typename T> constexpr T csc(T x) { return static_cast<T>(1) / sin(x); }

}
