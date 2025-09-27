#pragma once

#include <vector>
#include <array>
#include <cassert>

namespace core
{

const int NUM_INT_REGS = 32;
const int NUM_FLOAT_REGS = 32;

class Core
{
public:
    void set_int_reg(int num, int val)
    {
        assert(num < NUM_INT_REGS);
        int_regs[num] = val;
    }

    int get_int_reg(int num) const
    {
        assert(num < NUM_INT_REGS);
        return int_regs[num];
    }

    void set_int_reg(int num, float val)
    {
        assert(num < NUM_FLOAT_REGS);
        float_regs[num] = val;
    }

    float get_float_reg(int num)
    {
        assert(num < NUM_FLOAT_REGS);
        return float_regs[num];
    }

private:
    std::array<int, NUM_INT_REGS> int_regs;
    std::array<int, NUM_FLOAT_REGS> float_regs;

}; // class Core

} // namespace core