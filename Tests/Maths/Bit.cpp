#include <gtest/gtest.h>

#include <Stuff/Maths/Bit.hpp>

TEST(Bit, Bit) {
    ASSERT_EQ(Stf::reverse_bits(0x04c11db7u), 0xedb88320u);

    uint8_t lookup_0[] = { 6, 1, 3, 7, 0, 4, 5, 2 };
    std::pair<uint8_t, uint8_t> permutation_vector_0[] {
        { 0xFF, 0xFF },
        { 0xAA, 0x72 },
        { 0x55, 0x72 },

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
