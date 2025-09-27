#include "helpers.hpp"
#include <gtest/gtest.h>
#include <cstdint>

TEST(Helpers, bits)
{
    auto mask1 = helpers::bitmask(5);
    ASSERT_EQ(mask1, 0b11111);

    auto mask2 = helpers::bitmask(26);
    ASSERT_EQ(mask2, 0x3FFFFFF);

    auto mask3 = helpers::bitmask(0, 1);
    ASSERT_EQ(mask3, 0b1);

    auto mask4 = helpers::bitmask(0, 5);
    ASSERT_EQ(mask4, 0b11111);

    auto mask5 = helpers::bitmask(3, 8);
    ASSERT_EQ(mask5, 0b11111000);

    auto mask6 = helpers::bitmask(3, 4) | helpers::bitmask(7, 11);
    ASSERT_EQ(mask6, 0b11110001000);

    uint32_t val = 0b1111001101;

    uint32_t field1 = helpers::get_field(val, 0, 3);
    ASSERT_EQ(field1, 0b101);

    uint32_t field2 = helpers::get_field(val, 4, 8);
    ASSERT_EQ(field2, 0b1100);

    uint32_t field3 = helpers::get_field(val, 5, 10);
    ASSERT_EQ(field3, 0b11110);

    uint32_t field4 = helpers::get_field(val, 13, 22);
    ASSERT_EQ(field4, 0b0);
}