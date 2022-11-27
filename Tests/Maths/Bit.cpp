#include <gtest/gtest.h>

#include <random>

#include <fmt/format.h>

#include <Stuff/Maths/Bit.hpp>
#include <Stuff/Maths/Crypt/DES/Tables.hpp>

static std::random_device s_rd {};
static std::mt19937_64 s_engine { s_rd() };

TEST(Bit, Bit) {
    ASSERT_EQ(Stf::reverse_bits(0x04c11db7u), 0xedb88320u);

    uint8_t lookup_0[] = { 6, 1, 3, 7, 0, 4, 5, 2 };
    std::pair<uint8_t, uint8_t> permutation_vector_0[] {
        { 0xFF, 0xFF }, //
        { 0xAA, 0x72 }, //
        { 0x55, 0x72 }, //

        /*{ 0xA1, 0x },
        { 0x2F, 0x },
        { 0x1B, 0x },
        { 0x01, 0x },
        { 0x8C, 0x },
        { 0xFA, 0x },
        { 0xE5, 0x },
        { 0x6B, 0x },
        { 0xAE, 0x },
        { 0x47, 0x },
        { 0xC8, 0x },
        { 0xBB, 0x },*/
    };

    for (const auto [v, expected] : permutation_vector_0)
        ASSERT_EQ(Stf::permute_bits(v, lookup_0), expected);
}

TEST(Bit, Bitslice) {
    std::uniform_int_distribution<uint64_t> gen_48 { 0ul, 0xFFFF'FFFF'FFFFul };
    std::uniform_int_distribution<uint64_t> gen_32 { 0ul, 0xFFFF'FFFFul };

    for (auto i = 0uz; i < 512; i++) {
        uint64_t values[64];
        uint64_t config_0[48] {};
        uint64_t config_1[48] {};
        uint64_t config_2[48] {};
        uint64_t config_3[48] {};

        for (auto& v_target : values) {
            const auto v = gen_48(s_engine);
            v_target = v;

            Stf::bitslice_push<48, false, false>(config_0, v);
            Stf::bitslice_push<48, false, true>(config_1, v);
            Stf::bitslice_push<48, true, false>(config_2, v);
            Stf::bitslice_push<48, true, true>(config_3, v);
        }

        for (const auto expected : values) {
            const auto c_0_res = Stf::bitslice_pop<48, true, false>(config_0);
            const auto c_1_res = Stf::bitslice_pop<48, true, true>(config_1);
            const auto c_2_res = Stf::bitslice_pop<48, false, false>(config_2);
            const auto c_3_res = Stf::bitslice_pop<48, false, true>(config_3);

            ASSERT_EQ(expected, c_0_res) << fmt::format("expected: {:064b}\ngot     : {:064b}", expected, c_0_res);
            ASSERT_EQ(expected, c_1_res) << fmt::format("expected: {:064b}\ngot     : {:064b}", expected, c_1_res);
            ASSERT_EQ(expected, c_2_res) << fmt::format("expected: {:064b}\ngot     : {:064b}", expected, c_2_res);
            ASSERT_EQ(expected, c_3_res) << fmt::format("expected: {:064b}\ngot     : {:064b}", expected, c_3_res);
        }
    }
}

TEST(Bit, PermuteElems) {
    std::uniform_int_distribution<uint64_t> gen_32 { 0ul, 0xFFFF'FFFFul };
    using Stf::DES::Detail::k_f_final_p_table;

    for (auto i = 0uz; i < 512; i++) {
        static constexpr uint64_t one = 0xFFFF'FFFF'FFFF'FFFFul;
        const auto v = gen_32(s_engine);

        const auto expected = Stf::permute_bits(v, k_f_final_p_table);

        uint64_t arr[32];
        for (auto i = 0uz; i < 32; i++) {
            arr[i] = ((v >> (31 - i)) & 1) != 0 ? one : 0;
        }

        Stf::permute_elements(arr, k_f_final_p_table);

        const auto got = Stf::bitslice_pop(arr);

        ASSERT_EQ(expected, got);
    }
}
