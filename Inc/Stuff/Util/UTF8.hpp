#pragma once

#include <array>

namespace Stf {

constexpr std::array<char8_t, 4> encode_utf8(char32_t codepoint) {
    const std::array<std::array<char8_t, 4>, 4> patterns { { // sanic
        { 0x00, 0x00, 0x00, 0x00 }, { 0xC0, 0x80, 0x00, 0x00 }, { 0xE0, 0x80, 0x80, 0x00 }, { 0xF0, 0x80, 0x80, 0x80 } } };

    const std::array<char32_t, 4> thresholds {
        (1u << 21u) - 1u,
        (1u << 16u) - 1u,
        (1u << 11u) - 1u,
        (1u << 7u) - 1u,
    };

    std::size_t cat = 0;
    for (auto i = 0zu; i < thresholds.size(); i++) {
        if (codepoint > thresholds[i]) {
            cat = 4 - i;
            break;
        }
    }

    if (cat == 4)
        return encode_utf8(0xFFFDu);

    std::array<char8_t, 4> ret = patterns[cat];

    for (auto i = 0zu; i < cat; i++) {
        ret[cat - i] |= codepoint & 0x3F;
        codepoint >>= 6;
    }
    ret[0] |= codepoint;

    return ret;
}

template<std::input_iterator It> constexpr std::pair<char32_t, It> decode_utf8(It it, It end) {
    const auto replacement_ch = U'\xFFFE';

    if (it == end)
        return { replacement_ch, it };

    std::array<char8_t, 4> ch_bytes { static_cast<char8_t>(*it++), u8'\0', u8'\0', u8'\0' };
    auto const& leading = ch_bytes[0];

    const std::array<std::pair<char8_t, char8_t>, 4> cat_ident { {
        //                         ones     zeros
        { u8'\xF0', u8'\x08' }, // 11110000 00001000
        { u8'\xE0', u8'\x10' }, // 11100000 00010000
        { u8'\xC0', u8'\x20' }, // 11000000 00100000
        { u8'\x00', u8'\x80' }, // 00000000 10000000
    } };

    auto check_cat = [](char8_t v, char8_t ones, char8_t zeros) -> bool {
        if ((v & ones) != ones)
            return false;
        if ((v & zeros) != u8'\0')
            return false;
        return true;
    };

    std::size_t cat = 4;
    for (auto i = 0uz; i < 4uz; i++) {
        auto [ones, zeros] = cat_ident[i];
        if (check_cat(leading, ones, zeros)) {
            cat = 3 - i;
            break;
        }
    }

    if (cat == 4)
        return { replacement_ch, it };

    char32_t ret = U'\0';

    for (auto i = 0uz; i < cat; i++) {
        if (it == end)
            return { replacement_ch, it };

        ch_bytes[i + 1] = *it++;
    }

    const std::array<char8_t, 4> masks { u8'\x7F', u8'\x1F', u8'\x0F', u8'\x07' };

    ret |= ch_bytes[0] & masks[cat];

    for (auto i = 0uz; i < cat; i++) {
        ret <<= 6;
        ret |= ch_bytes[i + 1] & u8'\x3F';
    }

    return { ret, it };
}

}
