/*
 * Older GCC versions used by the STM's IDE does not support std::from_chars and std::to_chars
 */

#pragma once

#include <bit>
#include <charconv>
#include <optional>
#include <string_view>

namespace Stf {

namespace Conv {

template<bool validate = false> constexpr uint8_t from_hex_digit(char c) {
    if (c >= '0' && c <= '9') {
        return static_cast<uint8_t>(c - '0');
    }

    if constexpr (validate) {
        if (!('f' >= c && c >= 'a') && !('F' >= c && c >= 'A'))
            return 0x10;
    }

    if (c >= 'a')
        c -= 'a' - 'A';

    return 10 + static_cast<uint8_t>(c - 'A');
}

template<std::integral T> constexpr char to_hex_digit(T b) {
    b = b & 0xF;

    if (b < 10)
        return static_cast<char>(b) + '0';

    return static_cast<char>(b - 10) + 'A';
}

}

template<typename T, bool validate = false>
constexpr std::enable_if_t<std::is_unsigned_v<T>, std::conditional_t<validate, std::optional<T>, T>> fast_hex_sv_to_int(
    std::string_view view) {
    T res {};

    for (size_t i = 0; i < view.size(); i++) {
        char c = view[i];

        res *= T { 16 };

        if (c >= '0' && c <= '9') {
            res += static_cast<T>(c - '0');
        } else {
            if constexpr (validate) {
                if (!('f' >= c && c >= 'a') && !('F' >= c && c >= 'A'))
                    return std::nullopt;
            }

            if (c >= 'a')
                c -= 'a' - 'A';

            res += T { 10 } + static_cast<T>(c - 'A');
        }
    }

    return res;
}

static_assert(fast_hex_sv_to_int<uint16_t, true>("asd") == std::nullopt);
static_assert(fast_hex_sv_to_int<uint16_t, true>("123") == 0x123);
static_assert(fast_hex_sv_to_int<uint32_t, true>("DEADBEEF") == 0xDEADBEEFU);
static_assert(fast_hex_sv_to_int<uint32_t, true>("1234CAFE") == 0x1234CAFEU);

template<typename T, bool care_about_negatives = false, bool validate = false>
constexpr std::conditional_t<validate, std::optional<T>, T> fast_sv_to_int(std::string_view view) {
    static_assert((std::is_unsigned_v<T> && care_about_negatives) == false);

    bool negative = false;
    if constexpr (care_about_negatives) {
        if ((negative = view[0] == '-'))
            view = view.substr(1);
    }

    T res {};

    for (size_t i = 0; i < view.size(); i++) {
        if constexpr (validate) {
            if (view[i] > '9' || view[i] < '0')
                return std::nullopt;
        }

        res *= T { 10 };
        res += static_cast<T>(view[i] - '0');
    }

    if constexpr (care_about_negatives) {
        if (negative)
            return -res;
    }

    return res;
}

static_assert(fast_sv_to_int<int, true, true>("123"));
static_assert(fast_sv_to_int<int, true, true>("123") == 123);
static_assert(fast_sv_to_int<int, true, true>("-123") == -123);

template<typename T, bool validation = false>
std::enable_if_t<std::is_floating_point_v<T>, std::conditional_t<validation, std::optional<T>, T>> sv_to_float(
    std::string_view view, float& out) {
    const auto dot_index = view.find('.');

    if (dot_index == std::string_view::npos) {
        const auto ret = fast_sv_to_int<long long, true, true>(view);
        ;

        if constexpr (!validation) {
            return ret;
        }

        if (!ret)
            return std::nullopt;
        return *ret;
    }

    const auto lhs = view.substr(0, dot_index);

    const auto rhs = view.substr(dot_index + 1);
}

#if defined __cpp_lib_to_chars && false

using to_chars_result = std::to_chars_result;
using from_chars_result = std::from_chars_result;

#else

struct to_chars_result {
    char* ptr = nullptr;
    std::errc ec {};

    friend constexpr bool operator==(const to_chars_result&, const to_chars_result&) = default;
};

struct from_chars_result {
    const char* ptr = nullptr;
    std::errc ec {};

    friend constexpr bool operator==(const from_chars_result&, const from_chars_result&) = default;
};

#endif

namespace Conv::Detail {

constexpr char digit_from_char(char c) {
    constexpr std::array<char, 256> lookup = ([] {
        std::array<char, 256> ret = {};
        std::fill(ret.begin(), ret.end(), 127);

        for (char i = 0; i < 10; i++)
            ret[i + '0'] = i;

        for (char i = 0; i < 26; i++)
            ret[i + 'a'] = ret[i + 'A'] = 10 + i;

        return ret;
    })();

    return lookup[c];
}

}

template<std::integral T>
constexpr from_chars_result from_chars(const char* first, const char* last, T& value, int base = 10) {
    if (first == last) {
        return {
            .ptr = first,
            .ec = std::errc::invalid_argument,
        };
    }

    bool is_negative = false;
    if constexpr (std::is_signed_v<T>) {
        is_negative = *first == '-';
        if (is_negative)
            ++first;
    }

    using U = std::make_unsigned_t<T>;
    using TLims = std::numeric_limits<T>;
    using ULims = std::numeric_limits<U>;

    U v = 0;

    const int bits_per_digit = std::bit_width(static_cast<unsigned>(base));
    auto remaining_bits = TLims::digits;

    auto* it = first;
    for (; it != last; it++) {
        const auto digit = Conv::Detail::digit_from_char(*it);

        if (digit > base)
            break;

        const from_chars_result overflow_result {
            .ptr = first,
            .ec = std::errc::result_out_of_range,
        };

        remaining_bits -= bits_per_digit;

        if (remaining_bits >= 0) [[likely]] {
            v = v * base + digit;
        } else [[unlikely]] {
            if (__builtin_mul_overflow(v, base, &v) || __builtin_add_overflow(v, base, &v)) {
                return overflow_result;
            }
        }
    }

    if (it == first) {
        return {
            .ptr = it,
            .ec = std::errc::invalid_argument,
        };
    }

    if (std::is_signed_v<T> && is_negative) {
        if (ULims::max() > TLims::max()) {
            if (v > TLims::max()) {
                return {
                    .ptr = first,
                    .ec = std::errc::result_out_of_range,
                };
            } else {
                value = -static_cast<T>(v);
            }
        }
    } else {
        value = v;
    }

    return {
        .ptr = first,
        .ec = std::errc(),
    };
}

template<std::integral T>
constexpr from_chars_result from_chars(std::string_view s, T& value, int base = 10) {
    return from_chars(s.begin(), s.end(), value, base);
}

}
