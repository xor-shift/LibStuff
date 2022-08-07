#include <gtest/gtest.h>

#include <fmt/format.h>

#include <Stuff/Util/UTF8.hpp>

constexpr void encoding_test(std::array<char8_t, 4> result, std::array<char8_t, 4> expected) {
    if (result != expected) {
        fmt::print("expected {{{:02X}, {:02X}, {:02X}, {:02X}}} in UTF8 encoding test but got {{{:02X}, {:02X}, {:02X}, {:02X}}}\n", //
            static_cast<unsigned char>(expected[0]),                                                                                 //
            static_cast<unsigned char>(expected[1]),                                                                                 //
            static_cast<unsigned char>(expected[2]),                                                                                 //
            static_cast<unsigned char>(expected[3]),                                                                                 //
            static_cast<unsigned char>(result[0]),                                                                                   //
            static_cast<unsigned char>(result[1]),                                                                                   //
            static_cast<unsigned char>(result[2]),                                                                                   //
            static_cast<unsigned char>(result[3]));
        ASSERT_EQ(result, expected);
    }
}

template<typename It>
constexpr void decoding_test(std::pair<char32_t, It> result, char32_t expected) {
    if (result.first != expected) {
        fmt::print("expected U+{:X} in UTF8 decoding test but got U+{:X}", static_cast<uint32_t>(expected), static_cast<uint32_t>(result.first));
        ASSERT_EQ(result.first, expected);
    }
}

TEST(UTF8, Generic) {
    for (auto c = u8'\0'; c < u8'\x7F'; c++) {
        const auto enc_res = Stf::encode_utf8(c);
        encoding_test(enc_res, { c, u8'\0', u8'\0', u8'\0' });
        const auto dec_res = Stf::decode_utf8(enc_res.cbegin(), enc_res.cend());
        decoding_test(dec_res, c);
    }

    // vector from https://en.wikipedia.org/wiki/UTF-1
    std::array<std::pair<char32_t, std::array<char8_t, 4>>, 32> random_vectors { {
        { U'\x7F', { u8'\x7f', u8'\x00', u8'\x00', u8'\x00' } },
        { U'\x80', { u8'\xC2', u8'\x80', u8'\x00', u8'\x00' } },
        { U'\x9F', { u8'\xC2', u8'\x9F', u8'\x00', u8'\x00' } },
        { U'\xA0', { u8'\xC2', u8'\xA0', u8'\x00', u8'\x00' } },
        { U'\xBF', { u8'\xC2', u8'\xBF', u8'\x00', u8'\x00' } },
        { U'\xC0', { u8'\xC3', u8'\x80', u8'\x00', u8'\x00' } },
        { U'\xFF', { u8'\xC3', u8'\xBF', u8'\x00', u8'\x00' } },
        { U'\x100', { u8'\xC4', u8'\x80', u8'\x00', u8'\x00' } },
        { U'\x15D', { u8'\xC5', u8'\x9D', u8'\x00', u8'\x00' } },
        { U'\x15E', { u8'\xC5', u8'\x9E', u8'\x00', u8'\x00' } },
        { U'\x1BD', { u8'\xC6', u8'\xBD', u8'\x00', u8'\x00' } },
        { U'\x1BE', { u8'\xC6', u8'\xBE', u8'\x00', u8'\x00' } },
        { U'\x7FF', { u8'\xDF', u8'\xBF', u8'\x00', u8'\x00' } },
        { U'\x800', { u8'\xE0', u8'\xA0', u8'\x80', u8'\x00' } },
        { U'\xFFF', { u8'\xE0', u8'\xBF', u8'\xBF', u8'\x00' } },
        { U'\x1000', { u8'\xE1', u8'\x80', u8'\x80', u8'\x00' } },
        { U'\x4015', { u8'\xE4', u8'\x80', u8'\x95', u8'\x00' } },
        { U'\x4016', { u8'\xE4', u8'\x80', u8'\x96', u8'\x00' } },
        { U'\xD7FF', { u8'\xED', u8'\x9F', u8'\xBF', u8'\x00' } },
        { U'\xE000', { u8'\xEE', u8'\x80', u8'\x80', u8'\x00' } },
        { U'\xF8FF', { u8'\xEF', u8'\xA3', u8'\xBF', u8'\x00' } },
        { U'\xFDD0', { u8'\xEF', u8'\xB7', u8'\x90', u8'\x00' } },
        { U'\xFDEF', { u8'\xEF', u8'\xB7', u8'\xAF', u8'\x00' } },
        { U'\xFEFF', { u8'\xEF', u8'\xBB', u8'\xBF', u8'\x00' } },
        { U'\xFFFD', { u8'\xEF', u8'\xBF', u8'\xBD', u8'\x00' } },
        { U'\xFFFE', { u8'\xEF', u8'\xBF', u8'\xBE', u8'\x00' } },
        { U'\xFFFF', { u8'\xEF', u8'\xBF', u8'\xBF', u8'\x00' } },
        { U'\x10000', { u8'\xF0', u8'\x90', u8'\x80', u8'\x80' } },
        { U'\x38E2D', { u8'\xF0', u8'\xB8', u8'\xB8', u8'\xAD' } },
        { U'\x38E2E', { u8'\xF0', u8'\xB8', u8'\xB8', u8'\xAE' } },
        { U'\x100000', { u8'\xF4', u8'\x80', u8'\x80', u8'\x80' } },
        { U'\x10FFFF', { u8'\xF4', u8'\x8F', u8'\xBF', u8'\xBF' } },
    } };

    for (auto [c, expected] : random_vectors) {
        const auto enc_res = Stf::encode_utf8(c);
        encoding_test(enc_res, expected);
        const auto dec_res = Stf::decode_utf8(enc_res.cbegin(), enc_res.cend());
        decoding_test(dec_res, c);
    }
}
