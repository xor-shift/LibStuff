#pragma once

namespace Stf {

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

template<std::floating_point T> constexpr T map(T v, T from_a, T from_b, T to_a, T to_b) {
    const auto t = inv_lerp(v, from_a, from_b);
    return lerp(t, to_a, to_b);
}


}
