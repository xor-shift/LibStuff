#include <gtest/gtest.h>

#include <Stuff/Maths/Bit.hpp>

TEST(Bit, Bit) {
    ASSERT_EQ(Stf::reverse_bits(0x04c11db7u), 0xedb88320u);
}
