#pragma once

#include <cstdint>
#include <cassert>
#include <cstdio>
#include <bitset>
#include <cfenv>

namespace helpers
{

uint32_t bitmask(uint32_t length)
{
    assert(length <= 32);
    std::bitset<32> mask(0);
    for (int i = 0; i < length; i++)
        mask[i] = 1;

    return mask.to_ulong();
}

uint32_t bitmask(uint32_t from, uint32_t to)
{
    assert(from <= 32);
    assert(to <= 32);
    assert(from < to);

    std::bitset<32> mask(0);
    for (int i = from; i < to; i++)
        mask[i] = 1;

    return mask.to_ulong();
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

uint8_t get_rs3(uint32_t raw)
{
    return get_field(raw, 27, 32);
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
    auto sign = get_field(raw, 31, 32);
    auto val = get_field(raw, 20, 31);
    return sign ? (val | 0xFFFFFA00U) : val;
}

uint32_t get_imm_s(uint32_t raw)
{
    return get_field(raw, 7, 12) | (get_field(raw, 25, 32) << 5);
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

template <typename To, typename From>
To bitcast(From val)
{
    static_assert(sizeof(To) == sizeof(From));
    return *(To*)(&val);
}

void set_round(uint8_t rm)
{
    switch (rm)
    {
        case 0b000:
            std::fesetround(FE_TONEAREST);
            break;
        case 0b001:
            std::fesetround(FE_TOWARDZERO);
            break;
        case 0b010:
            std::fesetround(FE_DOWNWARD);
            break;
        case 0b011:
            std::fesetround(FE_UPWARD);
            break;
        default:
            std::fesetround(FE_TONEAREST);
            break;
    }
}

bool is_nan(uint32_t u)
{
    uint32_t exp = (u >> 23) & 0xFF;
    uint32_t frac = u & 0x7FFFFFu;
    return (exp == 0xFF) && (frac != 0);
}

bool is_snan(uint32_t u)
{
    if (!is_nan(u))
    {
        return false;
    }
    return ((u & (1u << 22)) == 0);
}

bool is_f_snan(float u)
{
    return is_snan(bitcast<uint32_t>(u));
}

bool is_f_nan(float u)
{
    return is_nan(bitcast<uint32_t>(u));
}

} // namespace helpers