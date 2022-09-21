#include <gtest/gtest.h>

#include <fmt/format.h>

#include <Stuff/Maths/Crypt/DES.hpp>

TEST(DES, Lookups) {
    // DES lookup order: 12345678
    // Stf lookup order: 76543210
    uint8_t des_identity_lookup[] { 1, 2, 3, 4, 5, 6, 7, 8 };
    std::array<uint8_t, 8> identity_table { 0, 1, 2, 3, 4, 5, 6, 7 };
    ASSERT_EQ(Stf::Crypt::DES::Detail::permutation_table(des_identity_lookup), identity_table);

    uint8_t des_compression_table_0[] { 1, 2, 3, 4 };
    std::array<uint8_t, 4> compression_table_0 { 4, 5, 6, 7 };
    ASSERT_EQ(Stf::Crypt::DES::Detail::permutation_table(des_compression_table_0), compression_table_0);

    uint8_t des_compression_table_1[] { 5, 6, 7, 8 };
    std::array<uint8_t, 4> compression_table_1 { 0, 1, 2, 3 };
    ASSERT_EQ(Stf::Crypt::DES::Detail::permutation_table(des_compression_table_1), compression_table_1);

    for (uint16_t i = 0; i <= 255; i++) {
        const auto b = static_cast<uint8_t>(i);
        ASSERT_EQ(Stf::permute_bits(b, compression_table_0), b >> 4);
        ASSERT_EQ(Stf::permute_bits(b, compression_table_1), b & 0x0F);
    }
}

TEST(DES, SBox) {
    ASSERT_EQ(Stf::Crypt::DES::Detail::k_sub_tables[0][0b1'0001'1], 0b1100);
    ASSERT_EQ(Stf::Crypt::DES::Detail::k_sub_tables[7][0b0'0000'0], 0b1101);
}

TEST(DES, Utilities) {
    std::pair<uint64_t, uint64_t> compression_vector[] {
        { 0xFFFF'FFFF'FFFF'FFFF, 0x00FF'FFFF'FFFF'FFFF },
        { 0x7F7F'7F7F'7F7F'7F7F, 0x00FF'FFFF'FFFF'FFFF },

        { 0x3F7F'7F7F'7F7F'7F7F, 0x007F'FFFF'FFFF'FFFF },
        { 0x7F7F'7F7F'7F7F'7F7E, 0x00FF'FFFF'FFFF'FFFE },

        { 0x7E7F'7F7F'7F7F'7F7F, 0x00FD'FFFF'FFFF'FFFF },
        { 0x7F7F'7F7F'7F7F'7F3F, 0x00FF'FFFF'FFFF'FFBF },
    };

    for (const auto [v, expected] : compression_vector) {
        const auto res = Stf::Crypt::DES::Detail::remove_key_parity(v);
        // fmt::print("{:016X}, {:016X}, {:016X}\n", v, expected, res);
        ASSERT_EQ(expected, res);
    }

    auto const& init_p_table = Stf::Crypt::DES::Detail::k_initial_permutation_table;
    ASSERT_EQ(Stf::permute_bits(0x0002'0000'0000'0001ull, init_p_table), 0x0000'0080'0000'0002ull);
    ASSERT_EQ(Stf::permute_bits(0x1234'56AB'CD13'2536ull, init_p_table), 0x14A7'D678'18CA'18ADull);
    auto const& final_p_table = Stf::Crypt::DES::Detail::k_final_permutation_table;
    ASSERT_EQ(Stf::permute_bits(0x0000'0080'0000'0002ull, final_p_table), 0x0002'0000'0000'0001ull);

    const auto rotl_28_1 = []<typename T>(T v) { return Stf::Crypt::DES::Detail::rotl<uint64_t>(v, 28, 1); };
    ASSERT_EQ(rotl_28_1(0xFFF'FFFF), 0xFFF'FFFF);
    ASSERT_EQ(rotl_28_1(0x7FF'FFFF), 0xFFF'FFFE);
    ASSERT_EQ(rotl_28_1(0x8FF'FFF0), 0x1FF'FFE1);
    ASSERT_EQ(rotl_28_1(0xAAA'AAAA), 0x555'5555);
    const auto rotl_28_2 = []<typename T>(T v) { return Stf::Crypt::DES::Detail::rotl<uint64_t>(v, 28, 2); };
    ASSERT_EQ(rotl_28_2(0xFFF'FFFF), 0xFFF'FFFF);
    ASSERT_EQ(rotl_28_2(0x7FF'FFFF), 0xFFF'FFFD);
    ASSERT_EQ(rotl_28_2(0x8FF'FFF0), 0x3FF'FFC2);
    ASSERT_EQ(rotl_28_2(0xAAA'AAAA), 0xAAA'AAAA);
}

TEST(DES, DES) {
    const uint64_t plaintext = 0x1234'56AB'CD13'2536ull;
    const uint64_t key = 0xAABB'0918'2736'CCDDull;
    const auto encrypted = Stf::Crypt::DES::encrypt(plaintext, key);

    ASSERT_EQ(encrypted, 0xC0B7A8D05F3A829Cull);
    ASSERT_EQ(Stf::Crypt::DES::decrypt(encrypted, key), plaintext);
}

// https://github.com/kongfy/DES/blob/master/Riv85.txt
TEST(DES, Recurrence) {
    const uint64_t expected_values[16] {
        0x8DA744E0C94E5E17,
        0x0CDB25E3BA3C6D79,
        0x4784C4BA5006081F,
        0x1CF1FC126F2EF842,
        0xE4BE250042098D13,
        0x7BFC5DC6ADB5797C,
        0x1AB3B4D82082FB28,
        0xC1576A14DE707097,
        0x739B68CD2E26782A,
        0x2A59F0C464506EDB,
        0xA5C39D4251F0A81E,
        0x7239AC9A6107DDB1,
        0x070CAC8590241233,
        0x78F87B6E3DFECF61,
        0x95EC2578C2C433F0,
        0x1B1A2DDB4C642438,
    };

    uint64_t x = 0x9474'B8E8'C73B'CA7Dul;
    for (auto i = 0uz; i < 16; i++) {
        if ((i % 2) != 0) {
            x = Stf::Crypt::DES::decrypt(x, x);
        } else {
            x = Stf::Crypt::DES::encrypt(x, x);
        }
        ASSERT_EQ(x, expected_values[i]);
    }
}

// crypt(3) functions adapted from:
// https://minnie.tuhs.org/cgi-bin/utree.pl?file=V7/usr/src/libc/gen/crypt.c
constexpr std::array<uint64_t, 48> expected_expansion_table(std::string_view salt) {
    std::array<uint64_t, 48> ret {
        32, 1, 2, 3, 4, 5,      //
        4, 5, 6, 7, 8, 9,       //
        8, 9, 10, 11, 12, 13,   //
        12, 13, 14, 15, 16, 17, //
        16, 17, 18, 19, 20, 21, //
        20, 21, 22, 23, 24, 25, //
        24, 25, 26, 27, 28, 29, //
        28, 29, 30, 31, 32, 1,  //
    };

    for (auto i = 0uz; i < 2; i++) {
        auto c = salt[i];
        if (c > 'Z')
            c -= 6;
        if (c > '9')
            c -= 7;
        c -= '.';
        for (auto j = 0uz; j < 6; j++) {
            if ((c >> j) & 1) {
                const auto temp = ret[6 * i + j];
                ret[6 * i + j] = ret[6 * i + j + 24];
                ret[6 * i + j + 24] = temp;
            }
        }
    }

    return ret;
}

TEST(DESCrypt3, ExpansionTable) {
    std::string_view salt_charset =   //
        "./"                          //
        "0123456789"                  //
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"  //
        "abcdefghijklmnopqrstuvwxyz"; // 2^6

    for (auto i = 0uz; i < 64uz; i++) {
        for (auto j = 0uz; j < 64uz; j++) {
            char salt[3] { salt_charset[i], salt_charset[j], '\0' };

            using Stf::Crypt::DES::Detail::permutation_table;

            const auto expected = permutation_table(expected_expansion_table(salt)._M_elems, 32uz);
            const auto got = Stf::Crypt::DES::Detail::get_crypt_expansion_block(salt);

            ASSERT_EQ(expected, got);
        }
    }
}

TEST(DESCrypt3, Crypt) {
    ASSERT_EQ(Stf::Crypt::DES::crypt("", ".."), "..X8NBuQ4l6uQ");
    ASSERT_EQ(Stf::Crypt::DES::crypt("AAAAAAAA", ".."), "..SttI9HzezEY");
    ASSERT_EQ(Stf::Crypt::DES::crypt("BBBBBBBB", ".."), "..F6F5SQeOMm2");
    ASSERT_EQ(Stf::Crypt::DES::crypt("AAAAAAAA", "AA"), "AALDLUg7SsaxM");
    ASSERT_EQ(Stf::Crypt::DES::crypt("AAAAAAAA", "//"), "//6qeRrsgI3aE");
    ASSERT_EQ(Stf::Crypt::DES::crypt("AAAAAAAA", "/A"), "/AwuVNnFjiz2g");
    ASSERT_EQ(Stf::Crypt::DES::crypt("AAAAAAAA", "A/"), "A/jMZ7uYiPMpU");

    ASSERT_EQ(Stf::Crypt::DES::crypt(".AAAAAAA", ".."), "..PxtcFr/TCPM");
    ASSERT_EQ(Stf::Crypt::DES::crypt("..AAAAAA", ".."), "..KwR2/fQaBZk");
}

TEST(DES, Tripcode) {
    ASSERT_EQ(Stf::Crypt::DES::tripcode("...AAAAA"), "BtNKI5JOy2");
    ASSERT_EQ(Stf::Crypt::DES::tripcode("AAAAAAAA"), "DLUg7SsaxM");
    ASSERT_EQ(Stf::Crypt::DES::tripcode("Hockeyhare"), "qSck1IAj6M");
    ASSERT_EQ(Stf::Crypt::DES::tripcode("AAAAAAA"), "V4n.MW5Rd2");
    ASSERT_EQ(Stf::Crypt::DES::tripcode("A"), "hRJ9Ya./t.");
    ASSERT_EQ(Stf::Crypt::DES::tripcode("&&"), "sS3IIIdY12");
}

TEST(BitsliceDES, SBoxes) {
    using Stf::Crypt::DES::Detail::Bitslice::X86::mk_sbox;
    static constexpr void (*mk_sboxes[8])(SBOX_ARGS) = {
        mk_sbox<1>,
        mk_sbox<2>,
        mk_sbox<3>,
        mk_sbox<4>,
        mk_sbox<5>,
        mk_sbox<6>,
        mk_sbox<7>,
        mk_sbox<8>,
    };

    for (uint64_t v = 0; v < 0x3Ful; v++) {
        static constexpr uint64_t one = 0xFFFF'FFFF'FFFF'FFFFul;
        std::array<uint64_t, 6> in;

        for (uint64_t i = 0; i < 6; i++) {
            const auto is_set = ((v >> (5 - i)) & 1) != 0;
            in[i] = is_set ? one : 0;
        }

        for (size_t box = 0; box < 8; box++) {
            std::array<uint64_t, 4> out {};

            auto* fn = mk_sboxes[box];

            const auto expected = Stf::Crypt::DES::Detail::k_sub_tables[box][v];
            fn(in[0], in[1], in[2], in[3], in[4], in[5], out[0], out[1], out[2], out[3]);

            ASSERT_EQ(out[0], (expected >> 3) & 1 ? one : 0);
            ASSERT_EQ(out[1], (expected >> 2) & 1 ? one : 0);
            ASSERT_EQ(out[2], (expected >> 1) & 1 ? one : 0);
            ASSERT_EQ(out[3], (expected >> 0) & 1 ? one : 0);
        }
    }

    uint64_t val = 0x1234'5678'9ABCul;
    uint64_t expected = Stf::Crypt::DES::Detail::sbox_transform_lookup(val);

    const auto single_0 = Stf::Crypt::DES::Detail::sbox_transform_bitslice<true>(val);
    const auto single_1 = Stf::Crypt::DES::Detail::sbox_transform_bitslice<false>(val);

    // fmt::print("expected: {:016X}\n", expected);
    // fmt::print("single  : {:016X}\n", single);

    ASSERT_EQ(expected, single_0);
    ASSERT_EQ(expected, single_1);
}

TEST(BitsliceDES, Single) { }
