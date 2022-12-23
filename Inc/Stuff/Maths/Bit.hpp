#pragma once

#include <array>
#include <bit>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <span>
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

template<typename T, size_t N, bool SliceMSB0 = true, bool LookupMSB0 = false> constexpr void permute_elements(std::span<T, N> arr, auto const& lookup) {
    T temporary[N];

    for (auto i = 0uz; i < N; i++) {
        auto const& src = arr[N - lookup[N - i - 1] - 1];
        auto& dst = temporary[i];
        dst = src;
    }

    std::ranges::copy(temporary, arr.begin());
}

template<typename T, size_t N, bool SliceMSB0 = true, bool LookupMSB0 = false> constexpr void permute_elements(T (&arr)[N], auto const& lookup) {
    return permute_elements(std::span<T, N> { arr, arr + N }, lookup);
}

/// Pushes a number to a bitslice of 64 width
/// \tparam Bits The number of bits per slice
/// \tparam ToMSB Whether to push to the MSB of the slices
/// \tparam MSB0 Whether to treat v's MSB bit as the index #0 for the slice
/// \param slices 64 bit slices of Bits width
/// \param v The value to push
template<size_t Bits, bool ToMSB = false, bool MSB0 = true> constexpr void bitslice_push(uint64_t (&slices)[Bits], uint64_t v) {
    for (auto i = 0uz; i < Bits; i++) {
        auto& target = slices[i];
        const auto b = MSB0                  //
            ? (((v >> (Bits - 1)) & 1) != 0) //
            : ((v & 1) != 0);

        if constexpr (MSB0)
            v <<= 1;
        else
            v >>= 1;

        if constexpr (ToMSB) {
            target >>= 1;
            target |= b * (1ul << 63);
        } else {
            target <<= 1;
            target |= b;
        }
    }
}

template<size_t Bits, bool FromMSB = false, bool MSB0 = true> constexpr uint64_t bitslice_pop(uint64_t (&slices)[Bits]) {
    uint64_t ret = 0;

    for (auto i = 0uz; i < Bits; i++) {
        auto& source = slices[MSB0 ? i : Bits - i - 1];
        const auto b = FromMSB              //
            ? ((source & (1ul << 63)) != 0) //
            : ((source & 1) != 0);

        if constexpr (FromMSB)
            source <<= 1;
        else
            source >>= 1;

        ret <<= 1;
        ret |= b;
    }

    return ret;
}

}
