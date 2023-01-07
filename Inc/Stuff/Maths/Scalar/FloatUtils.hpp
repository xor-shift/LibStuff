#pragma once

namespace Stf {

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
    static constexpr raw_repr max_exponent = 0x7FFull;
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

}
