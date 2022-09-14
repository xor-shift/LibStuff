#pragma once

#include <cstddef>

#include "./FloatUtils.hpp"

namespace Stf {

template<std::floating_point T> constexpr T exp(T x) {
    const T target_delta = static_cast<T>(0.00001);
    const size_t max_rounds = 50;

    if (x <= 0.0001)
        return x + 1;

    T ret = 0;
    T term = 1;

    for (size_t k = 1;;) {
        ret += term;
        const auto temp = term * (x / static_cast<T>(k));
        const auto close_enough = k != 1 && is_close(temp, term, target_delta);
        term = temp;

        if (close_enough || ++k >= max_rounds)
            break;
    }

    return ret;
}

}