#pragma once

#include <concepts>

#include "./FloatUtils.hpp"

namespace Stf {

template<std::floating_point T> constexpr bool is_nan(T f) {
    using U = FloatParts<T>;
    U parts(f);

    return parts.exponent == U::max_exponent && parts.fraction != 0;
}

template<std::floating_point T> constexpr bool is_finite(T f) {
    using U = FloatParts<T>;
    return U(f).exponent != U::max_exponent;
}

template<std::floating_point T> constexpr bool is_inf(T f) { return !is_finite(f) && !is_nan(f); }

template<std::floating_point T> constexpr bool is_normal(T f) {
    return FloatParts<T>(f).exponent != 0 && !is_inf(f) && !is_nan(f);
}

template<std::floating_point T> constexpr bool sign_bit(T f) { return FloatParts<T> { f }.sign; }

}