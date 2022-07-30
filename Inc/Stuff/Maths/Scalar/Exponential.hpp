#pragma once

#include <cstddef>

#include "./FloatUtils.hpp"

namespace Stf {

template<std::floating_point T> constexpr T exp(T x) {
    const T target_delta = static_cast<T>(0.000001);
    const size_t max_rounds = 50;

    if (x <= 0.0001)
        return x + 1;

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

}