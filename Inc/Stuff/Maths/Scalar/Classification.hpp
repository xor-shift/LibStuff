#pragma once

namespace Stf {

template<std::floating_point T> constexpr bool is_nan(T f) {
    using U = FloatParts<T>;
    const U parts { f };

    return parts.exponent == U::max_exponent && parts.fraction != 0;
}

template<std::integral T> constexpr bool is_nan(T i) { return false; }

template<std::floating_point T> constexpr bool is_finite(T f) {
    using U = FloatParts<T>;
    const U parts { f };

    return parts.exponent != U::max_exponent;
}

template<std::integral T> constexpr bool is_finite(T i) { return true; }

template<typename T> constexpr bool is_inf(T f) { return !is_finite(f) && !is_nan(f); }

template<std::floating_point T> constexpr bool is_normal(T f) { return FloatParts<T>(f).exponent != 0 && !is_inf(f) && !is_nan(f); }

template<std::floating_point T> constexpr bool sign_bit(T f) { return FloatParts<T> { f }.sign; }

template<std::unsigned_integral T> constexpr bool sign_bit(T i) { return false; }

template<std::signed_integral T> constexpr bool sign_bit(T i) { return i < 0; }

}