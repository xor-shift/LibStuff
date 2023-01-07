#pragma once

namespace Stf::Detail::CEMaths {

template<std::floating_point T> constexpr T exp(T x) {
    if (is_nan(x))
        return x;

    if (x < 0 && is_inf(x))
        return 0;

    if (x > 0 && is_inf(x))
        return x;

    if (x == 0)
        return 1;

    const T target_delta = 0.00001f;
    const size_t max_rounds = 50;

    if (x <= 0.0001)
        return x + 1;

    T ret = 0;
    T term = 1;

    for (size_t k = 1;;) {
        ret += term;
        const auto temp = term * (x / k);
        const auto close_enough = k != 1 && is_close(temp, term, target_delta);
        term = temp;

        if (close_enough || ++k >= max_rounds)
            break;
    }

    return ret;
}

}

namespace Stf {

template<std::floating_point T> constexpr T exp(T x) {
    if consteval {
        return Detail::CEMaths::exp(x);
    } else {
        return std::exp(x);
    }
}

}
