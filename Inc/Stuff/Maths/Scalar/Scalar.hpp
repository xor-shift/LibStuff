#pragma once

#include <bit>
#include <concepts>
#include <cstdint>

namespace Stf::Detail {

template<typename T> struct FloatParts;

template<> struct FloatParts<float> {
    using raw_repr = uint32_t;
    static constexpr raw_repr exponent_bits = 8;
    static constexpr raw_repr max_exponent = 0xFFu;
    static constexpr raw_repr fraction_bits = 23;
    static constexpr raw_repr max_fraction = 0x7FFFFFu;

    bool sign : 1;
    raw_repr exponent : 8;
    raw_repr fraction : 23;

    constexpr explicit FloatParts(float f) {
        const auto raw = std::bit_cast<raw_repr>(f);

        sign = (raw & (1u << 31u)) != 0ull;
        exponent = (raw & (0xFFu << 23u)) >> 23u;
        fraction = raw & 0x7FFFFFu;
    }
};

template<> struct FloatParts<double> {
    using raw_repr = uint64_t;
    static constexpr raw_repr exponent_bits = 11;
    static constexpr raw_repr max_exponent = 0x3FFull;
    static constexpr raw_repr fraction_bits = 52;
    static constexpr raw_repr max_fraction = 0xFFFFFFFFFFFFFull;

    bool sign : 1;
    raw_repr exponent : 11;
    raw_repr fraction : 52;

    constexpr explicit FloatParts(double f) {
        const auto raw = std::bit_cast<raw_repr>(f);

        sign = (raw & (1ull << 63ull)) != 0ull;
        exponent = (raw & (0x3FFull << 52ull)) >> 52ull;
        fraction = raw & 0xFFFFFFFFFFFFFull;
    }
};

}

namespace Stf::Detail::FP {

namespace Classification {

template<std::floating_point T> constexpr bool is_nan(T f) {
    using U = Detail::FloatParts<T>;
    U parts(f);

    return parts.exponent == U::max_exponent && parts.fraction != 0;
}

template<std::floating_point T> constexpr bool is_finite(T f) {
    using U = Detail::FloatParts<T>;
    return U(f).exponent != U::max_exponent;
}

template<std::floating_point T> constexpr bool is_inf(T f) { return !is_finite(f) && !is_nan(f); }

template<std::floating_point T> constexpr bool is_normal(T f) {
    return Detail::FloatParts<T>(f).exponent != 0 && !is_inf(f) && !is_nan(f);
}

template<std::floating_point T> constexpr bool sign_bit(T f) { return Detail::FloatParts<T> { f }.sign; }

}

namespace Manipulation {

template<std::floating_point T> constexpr T float_from_parts(FloatParts<T> parts) {
    using U = FloatParts<T>;

    typename U::raw_repr raw = parts.sign ? 1ull : 0ull;
    raw <<= U::exponent_bits;
    raw |= parts.exponent;
    raw <<= U::fraction_bits;
    raw |= parts.fraction;

    return std::bit_cast<T>(raw);
}

template<std::floating_point T> constexpr FloatParts<T> float_to_parts(T f) { return FloatParts<T>(f); }

/// this is not the same as c stdlib's ldexp, not even close. This only works with the exponent and *WILL NOT* touch
/// the mantissa.\n
/// TODO: fix the above issue
/// \return v * 2 ^ exp
template<std::floating_point T> constexpr T ldexp(T v, int exp) {
    if (v == 0 || Classification::is_inf(v) || Classification::is_nan(v) || exp == 0)
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

}

}

namespace Stf {

using namespace Detail::FP::Classification;
using namespace Detail::FP::Manipulation;

}
