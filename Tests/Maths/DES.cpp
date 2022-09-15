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
    uint64_t val = 0x1234'5678'9ABCul;
    uint64_t expected = Stf::Crypt::DES::Detail::sbox_transform_lookup(val);

    std::array<uint64_t, 64> vectorised { val };
    Stf::Crypt::DES::Detail::sbox_transform_bitslice_x86(vectorised);

    fmt::print("expected: {:016X}\n", expected);
    fmt::print("got     : {:016X}\n", vectorised[0]);

    ASSERT_EQ(expected, vectorised[0]);
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

    ASSERT_EQ(Stf::Crypt::DES::Detail::k_sub_tables[0][0b1'0001'1], 0b1100);
    ASSERT_EQ(Stf::Crypt::DES::Detail::k_sub_tables[7][0b0'0000'0], 0b1101);
}

TEST(DES, DES) {
    const uint64_t plaintext = 0x1234'56AB'CD13'2536ull;
    const uint64_t key = 0xAABB'0918'2736'CCDDull;
    const auto encrypted = Stf::Crypt::DES::crypt(plaintext, key);

    ASSERT_EQ(encrypted, 0xC0B7A8D05F3A829Cull);
    ASSERT_EQ(Stf::Crypt::DES::decrypt(encrypted, key), plaintext);
}
