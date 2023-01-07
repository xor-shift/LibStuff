#pragma once

namespace Stf::Detail::CEMaths {

template<typename T> constexpr T sin(T x) {
    const T target_delta = 0.000001f;
    const size_t max_rounds = 50;

    const T k = std::round(x * std::numbers::inv_pi_v<T> * 0.5f);
    x -= k * 2 * std::numbers::pi_v<T>;
    // sin(x) = sin(x - 2pi * round(x/2pi))

    T ret = 0;
    T term = x;

    for (size_t k = 0; k < max_rounds; k++) {
        if ((k % 2) != 0)
            ret -= term;
        else
            ret += term;

        const T divisor = 4 * k * k + 10 * k + 6;
        const T temp = term * (x * x / divisor);

        if (is_close(temp, term, target_delta))
            break;

        term = temp;
    }

    return ret;
}

}

namespace Stf {

template<typename T> constexpr T sin(T x) {
    if consteval {
        return Detail::CEMaths::sin(x);
    } else {
        return std::sin(x);
    }
}

template<typename T> constexpr T cos(T x) {
    if consteval {
        return sin(M_PI_2 + x);
    } else {
        return std::cos(x);
    }
}

template<typename T> constexpr T tan(T x) {
    if consteval {
        return sin(x) / cos(x);
    } else {
        return std::tan(x);
    };
}

template<typename T> constexpr T cot(T x) { return 1 / tan(x); }

template<typename T> constexpr T sec(T x) { return 1 / cos(x); }

template<typename T> constexpr T csc(T x) { return 1 / sin(x); }

}
