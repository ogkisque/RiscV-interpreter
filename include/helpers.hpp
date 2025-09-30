#pragma once

#include <cstdint>
#include <cassert>

namespace helpers
{

uint32_t bitmask(uint32_t length)
{
    assert(length <= 32);
    if (length == 0)
    {
        return 0U;
    }

    return (1U << length) - 1;
}

uint32_t bitmask(uint32_t from, uint32_t to)
{
    assert(from <= 32);
    assert(to <= 32);
    assert(from < to);

    auto mask1 = bitmask(from);
    auto mask2 = bitmask(to);

    return mask2 & (~mask1);
}

uint32_t get_field(uint32_t val, uint32_t from, uint32_t to)
{
    return (val & bitmask(from, to)) >> from;
}

uint8_t get_opcode(uint32_t raw)
{
    return get_field(raw, 0, 7);
}

uint8_t get_rd(uint32_t raw)
{
    return get_field(raw, 7, 12);
}

uint8_t get_rs1(uint32_t raw)
{
    return get_field(raw, 15, 20);
}

uint8_t get_rs2(uint32_t raw)
{
    return get_field(raw, 20, 25);
}

uint8_t get_funct3(uint32_t raw)
{
    return get_field(raw, 12, 15);
}

uint8_t get_funct7(uint32_t raw)
{
    return get_field(raw, 25, 32);
}

uint32_t get_imm_i(uint32_t raw)
{
    return get_field(raw, 20, 32);
}

uint32_t get_imm_s(uint32_t raw)
{
    return get_field(raw, 7, 12) | get_field(raw, 25, 32);
}

uint32_t get_imm_u(uint32_t raw)
{
    return get_field(raw, 12, 32) << 12;
}

uint32_t get_imm_j(uint32_t raw)
{
    return (get_field(raw, 21, 31) << 1) | (get_field(raw, 20, 21) << 11) |
           (get_field(raw, 12, 20) << 12) | (get_field(raw, 31, 32) << 20);
}

uint32_t get_imm_b(uint32_t raw)
{
    return (get_field(raw, 8, 12) << 1) | (get_field(raw, 25, 31) << 5) |
           (get_field(raw, 7, 8) << 11) | (get_field(raw, 31, 32) << 12);
}

} // namespace helpers