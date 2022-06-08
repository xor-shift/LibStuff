#pragma once

#include <climits>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace Stf {

namespace Detail {

template<typename T, size_t nth_pattern>
constexpr std::enable_if_t<std::is_unsigned_v<T>, T> bit_reversal_pattern() {
    //                  inc shift val
    //1010101010101010   2    2    1
    //1100110011001100   4    4    3
    //1111000011110000   8    8    15
    //1111111100000000   16   16   255

    T ret{};

    constexpr size_t bits = sizeof(T) * CHAR_BIT;

    constexpr size_t shift_amount = 1 << (nth_pattern + 1);
    constexpr T or_val = (T{1} << (shift_amount / 2)) - 1;
    constexpr size_t final_shift = shift_amount / 2;

    for (size_t i = 0; i < bits; i += shift_amount) {
        ret <<= (shift_amount >= bits) ? bits - 1 : shift_amount;
        ret |= or_val;
    }

    return ret << final_shift;
}

template<typename T>
constexpr std::enable_if_t<std::is_unsigned_v<T>, size_t> bit_reversal_step_count() {
    return std::conditional_t<sizeof(T) == sizeof(uint64_t),
      std::integral_constant<size_t, 6>,
      std::conditional_t<sizeof(T) == sizeof(uint32_t),
        std::integral_constant<size_t, 5>,
        std::conditional_t<sizeof(T) == sizeof(uint16_t),
          std::integral_constant<size_t, 4>,
          std::conditional_t<sizeof(T) == sizeof(uint8_t),
            std::integral_constant<size_t, 3>,
            void>>
      >
    >::value;
}

template<typename T, size_t current_step>
constexpr std::enable_if_t<std::is_unsigned_v<T>, T> reverse_bits_patterns(T v) {
    const size_t pattern = bit_reversal_pattern<T, current_step>();

    const auto shift = 1 << current_step;

    const auto left = v & pattern;
    const auto right = v & ~pattern;

    const auto temp = (left >> shift) | (right << shift);

    if constexpr (current_step + 1 < bit_reversal_step_count<T>())
        return reverse_bits_patterns<T, current_step + 1>(temp);
    return temp;
}

template<typename T>
constexpr std::enable_if_t<std::is_unsigned_v<T>, T> reverse_bits_naive(T v) {
    T out{};

    for (size_t i = 0; i < sizeof(T) * CHAR_BIT; i++) {
        const T lsb = v & T{1};
        v >>= 1;
        out |= lsb << (sizeof(T) * CHAR_BIT - i - 1);
    }

    return out;
}

}

template<typename T>
constexpr std::enable_if_t<std::is_unsigned_v<T>, T> reverse_bits(T v) {
    return Detail::reverse_bits_patterns<T, 0>(v);
}

}
