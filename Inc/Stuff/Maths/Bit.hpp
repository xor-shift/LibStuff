#pragma once

#include <array>
#include <bit>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace Stf {

namespace Detail {

template<typename T, size_t nth_pattern> constexpr std::enable_if_t<std::is_unsigned_v<T>, T> bit_reversal_pattern() {
    //                  inc shift val
    // 1010101010101010   2    2    1
    // 1100110011001100   4    4    3
    // 1111000011110000   8    8    15
    // 1111111100000000   16   16   255

    T ret {};

    constexpr size_t bits = sizeof(T) * CHAR_BIT;

    constexpr size_t shift_amount = 1 << (nth_pattern + 1);
    constexpr T or_val = (T { 1 } << (shift_amount / 2)) - 1;
    constexpr size_t final_shift = shift_amount / 2;

    for (size_t i = 0; i < bits; i += shift_amount) {
        ret <<= (shift_amount >= bits) ? bits - 1 : shift_amount;
        ret |= or_val;
    }

    return ret << final_shift;
}

template<typename T> constexpr std::enable_if_t<std::is_unsigned_v<T>, size_t> bit_reversal_step_count() {
    return std::conditional_t<sizeof(T) == sizeof(uint64_t), std::integral_constant<size_t, 6>,
        std::conditional_t<sizeof(T) == sizeof(uint32_t), std::integral_constant<size_t, 5>,
            std::conditional_t<sizeof(T) == sizeof(uint16_t), std::integral_constant<size_t, 4>,
                std::conditional_t<sizeof(T) == sizeof(uint8_t), std::integral_constant<size_t, 3>, void>>>>::value;
}

template<typename T, size_t current_step> constexpr std::enable_if_t<std::is_unsigned_v<T>, T> reverse_bits_patterns(T v) {
    const size_t pattern = bit_reversal_pattern<T, current_step>();

    const auto shift = 1 << current_step;

    const auto left = v & pattern;
    const auto right = v & ~pattern;

    const auto temp = (left >> shift) | (right << shift);

    if constexpr (current_step + 1 < bit_reversal_step_count<T>())
        return reverse_bits_patterns<T, current_step + 1>(temp);
    return temp;
}

template<typename T> constexpr std::enable_if_t<std::is_unsigned_v<T>, T> reverse_bits_naive(T v) {
    T out {};

    for (size_t i = 0; i < sizeof(T) * CHAR_BIT; i++) {
        const T lsb = v & T { 1 };
        v >>= 1;
        out |= lsb << (sizeof(T) * CHAR_BIT - i - 1);
    }

    return out;
}

}

template<typename T> constexpr std::enable_if_t<std::is_unsigned_v<T>, T> reverse_bits(T v) { return Detail::reverse_bits_patterns<T, 0>(v); }

template<typename T> constexpr T reverse_bytes(T v) {
    auto arr = std::bit_cast<std::array<char, sizeof(T)>>(v);
    for (size_t i = 0; i < sizeof(T) / 2; i++)
        std::swap(arr[i], arr[sizeof(T) - i - 1]);
    return std::bit_cast<T>(arr);
}

template<typename T> constexpr T convert_endian(T v, std::endian from, std::endian to = std::endian::native) {
    if (to == from)
        return v;
    return reverse_bytes(v);
}

/// the LSB-0 bit index `i` of the result will be picked from LSB-0 bit index
/// `lookup[i]` of the val, meaning that the lookup should contain a reversed
/// mapping. if T = uint8_t, val = 0xA0, lookup[0] = 7, and lookup[1] = 5, then
/// the returned value will be 0x03. \n
/// if the `lookup`'s size is not equal to the T's bit width, not all of `val`
/// will be permuted.
template<typename T> constexpr T permute_bits(T val, auto const& lookup) {
    const auto get_bit = [](T v, T index) -> bool { return ((v >> index) & 1) != 0; };

    const auto set_bit = [](T v, T index, bool bit) -> T {
        if (bit) {
            return v | (static_cast<T>(1) << index);
        }
        return v & ~(static_cast<T>(1) << index);
    };

    T ret = 0;

    for (T i = 0; i < std::min<T>(std::size(lookup), std::numeric_limits<T>::digits); i++) {
        const auto set = get_bit(val, lookup[i]);
        ret = set_bit(ret, i, set);
    }

    return ret;
}

}
