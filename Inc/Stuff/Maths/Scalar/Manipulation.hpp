#pragma once

namespace Stf {

/// this is not the same as c stdlib's ldexp, not even close. This only works with the exponent and *WILL NOT* touch
/// the mantissa.\n
/// TODO: fix the above issue
/// \return v * 2 ^ exp
template<std::floating_point T> constexpr T ldexp(T v, int exp) {
    if (v == 0 || is_inf(v) || is_nan(v) || exp == 0)
        return v;

    using U = FloatParts<T>;
    using Signed = std::make_signed_t<typename U::raw_repr>;

    U parts(v);
    const auto prev_exp = static_cast<Signed>(parts.exponent);
    const auto new_exp = prev_exp + exp;

    if (new_exp > static_cast<Signed>(U::max_exponent) || new_exp < 0) {
        const auto huge = std::numeric_limits<T>::max();
        return parts.sign ? -huge : huge;
    }

    parts.exponent = static_cast<typename U::raw_repr>(new_exp);

    return float_from_parts(parts);
}

template<std::floating_point T> constexpr T copysign(T mag, T sgn) {
    const auto mag_parts = float_to_parts(mag);
    const auto sgn_parts = float_to_parts(sgn);

    mag_parts.sign = sgn_parts.sign;

    return float_from_parts(mag_parts);
}

}