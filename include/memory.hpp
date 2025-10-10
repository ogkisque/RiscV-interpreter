#pragma once

#include <vector>
#include <array>
#include <cassert>
#include <memory>
#include <optional>
#include <cstring>

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
        fprintf(stderr, "Int regs:\n");
        for (int i = 0; i < NUM_INT_REGS; i++)
        {
            fprintf(stderr, "r%2d = %u (0x%08x) (%d)\n",
                            i, int_regs_[i], int_regs_[i], (int)int_regs_[i]);
        }

        //printf("\nFloat regs:\n");
        //for (int i = 0; i < NUM_INT_REGS; i++)
        //{
            //printf("r%2d = %f (0x%08x)\n", i, float_regs_[i], *(uint32_t*)(&float_regs_[i]));
        //}
        //printf("\n");
    }

private:
    std::array<uint32_t, NUM_INT_REGS> int_regs_;
    std::array<float, NUM_FLOAT_REGS> float_regs_;

}; // class Registers

class SimpleMemory
{
public:
    uint8_t load8(uint32_t addr) const
    {
        assert(addr < data_.size());
        return data_[addr];
    }

    uint16_t load16(uint32_t addr) const
    {
        assert(addr + 1 < data_.size());
        return (uint16_t) data_[addr] | ((uint16_t) data_[addr + 1] << 8);
    }

    uint32_t load32(uint32_t addr) const
    {
        //fprintf(stderr, "LOAD ADDR 0x%08x; SIZE 0x%16lx\n", addr, data_.size());
        assert(addr + 3 < data_.size());
        //fprintf(stderr, "LOAD VALUES 0x%02x 0x%02x 0x%02x 0x%02x\n",
                //data_[addr], data_[addr+1], data_[addr+2], data_[addr+3]);
        return (uint32_t) data_[addr] | ((uint32_t) data_[addr + 1] << 8) |
               ((uint32_t) data_[addr + 2] << 16) | ((uint32_t) data_[addr + 3] << 24);
    }

    void store8(uint32_t addr, uint8_t val)
    {
        ensure_capacity(addr, 1);
        data_[addr] = val;
    }

    void store16(uint32_t addr, uint16_t val)
    {
        ensure_capacity(addr, 2);
        data_[addr] = val & 0xFF;
        data_[addr + 1] = (val >> 8) & 0xFF;
    }

    void store32(uint32_t addr, uint32_t val)
    {
        ensure_capacity(addr, 4);
        //fprintf(stderr, "STORE ADDR 0x%08x; SIZE 0x%16lx; val 0x%08x\n", addr, data_.size(), val);
        data_[addr] = val & 0xFF;
        data_[addr + 1] = (val >> 8) & 0xFF;
        data_[addr + 2] = (val >> 16) & 0xFF;
        data_[addr + 3] = (val >> 24) & 0xFF;
        //fprintf(stderr, "STORE VALUES 0x%02x 0x%02x 0x%02x 0x%02x\n",
                //data_[addr], data_[addr+1], data_[addr+2], data_[addr+3]);
    }

    void read_bytes(uint32_t addr, uint8_t* dst, size_t len) const
    {
        assert(uint64_t(addr) + len <= data_.size());
        std::memcpy(dst, &data_[addr], len);
    }

    void write_bytes(uint32_t addr, const uint8_t* src, size_t len)
    {
        if (len == 0)
        {
            return;
        }
        ensure_capacity(addr, len);
        std::memcpy(&data_[addr], src, len);
    }

    void ensure_capacity(uint32_t addr, size_t len = 1)
    {
        uint64_t need = uint64_t(addr) + uint64_t(len);
        if (need == 0)
        {
            return;
        }

        if (need > data_.size())
        {
            size_t new_size = std::max<uint64_t>(need, std::max<uint64_t>(data_.size() * 2, 0x10000));
            data_.resize(new_size, 0);
        }
    }

private:
    std::vector<uint8_t> data_;

}; // class SimpleMemory

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
        fprintf(stderr, "PC = 0x%08x\n\n", pc_);
    }

    uint8_t load8(uint32_t addr) const
    {
        return simple_memory_.load8(addr);
    }

    uint16_t load16(uint32_t addr) const
    {
        return simple_memory_.load16(addr);
    }

    uint32_t load32(uint32_t addr) const
    {
        return simple_memory_.load32(addr);
    }

    void store8(uint32_t addr, uint8_t val)
    {
        simple_memory_.store8(addr, val);
    }

    void store16(uint32_t addr, uint16_t val)
    {
        simple_memory_.store16(addr, val);
    }

    void store32(uint32_t addr, uint32_t val)
    {
        simple_memory_.store32(addr, val);
    }

    void read_bytes(uint32_t addr, uint8_t* dst, size_t len) const
    {
        simple_memory_.read_bytes(addr, dst, len);
    }

    void write_bytes(uint32_t addr, const uint8_t* src, size_t len)
    {
        simple_memory_.write_bytes(addr, src, len);
    }

    void ensure_capacity(uint32_t addr, size_t len = 1)
    {
        simple_memory_.ensure_capacity(addr, len);
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

    void set_exit()
    {
        exit_ = true;
    }

    bool is_exit()
    {
        return exit_;
    }

private:
    Registers regs_;
    SimpleMemory simple_memory_;
    uint32_t pc_ = 0;
    std::optional<uint32_t> zero_pc_ = std::nullopt;
    bool exit_ = false;
}; // class memory

} // namespace memory