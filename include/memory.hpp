#pragma once

#include <vector>
#include <array>
#include <cassert>
#include <memory>
#include <optional>

namespace memory
{

const int NUM_INT_REGS = 32;
const int NUM_FLOAT_REGS = 32;

class Registers
{
public:
    void set_int_reg(int num, uint32_t val)
    {
        assert(num < NUM_INT_REGS);
        assert(num != 0);
        int_regs_[num] = val;
    }

    uint32_t get_int_reg(int num) const
    {
        assert(num < NUM_INT_REGS);
        if (num == 0)
        {
            return 0;
        }
        return int_regs_[num];
    }

    void set_float_reg(int num, float val)
    {
        assert(num < NUM_FLOAT_REGS);
        float_regs_[num] = val;
    }

    float get_float_reg(int num) const
    {
        assert(num < NUM_FLOAT_REGS);
        return float_regs_[num];
    }

    void dump_regs() const
    {
        printf("Int regs:\n");
        for (int i = 0; i < NUM_INT_REGS; i++)
        {
            printf("r%2d = %u (0x%08x)\n", i, int_regs_[i], int_regs_[i]);
        }

        printf("\nFloat regs:\n");
        for (int i = 0; i < NUM_INT_REGS; i++)
        {
            printf("r%2d = %f (0x%08x)\n", i, float_regs_[i], *(uint32_t*)(&float_regs_[i]));
        }
        printf("\n");
    }

private:
    std::array<uint32_t, NUM_INT_REGS> int_regs_;
    std::array<float, NUM_FLOAT_REGS> float_regs_;

}; // class Registers

class Memory
{
public:
    void set_int_reg(int num, uint32_t val)
    {
        regs_.set_int_reg(num, val);
    }

    uint32_t get_int_reg(int num) const
    {
        return regs_.get_int_reg(num);
    }

    void set_float_reg(int num, float val)
    {
        regs_.set_float_reg(num, val);
    }

    float get_float_reg(int num) const
    {
        return regs_.get_float_reg(num);
    }

    void dump_regs() const
    {
        regs_.dump_regs();
        printf("PC = %u\n", pc_);
    }

    uint32_t get_pc() const
    {
        return pc_;
    }

    void set_pc(uint32_t val)
    {
        pc_ = val;
    }

    void set_zero_pc(uint32_t pc)
    {
        zero_pc_ = pc;
    }

    uint32_t get_instr_index() const
    {
        assert(zero_pc_.has_value());
        return (pc_ - zero_pc_.value()) / 4;
    }
private:
    Registers regs_;
    uint32_t pc_ = 0;
    std::optional<uint32_t> zero_pc_ = std::nullopt;
}; // class memory

} // namespace memory