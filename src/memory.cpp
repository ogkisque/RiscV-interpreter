#include "memory.hpp"
//#include "helpers.hpp"

namespace memory
{

void SimpleMemory::store8(uint32_t addr, uint8_t val)
{
    assert(addr < data_.size());
    data_[addr] = val;
}

void SimpleMemory::store16(uint32_t addr, uint16_t val)
{
    assert(addr + 1 < data_.size());
    data_[addr] = val & 0xFF;//helpers::bitmask(8);
    data_[addr + 1] = (val >> 8) & 0xFF;//helpers::bitmask(8);
}

void SimpleMemory::store32(uint32_t addr, uint32_t val)
{
    assert(addr + 3 < data_.size());
    data_[addr] = val & 0xFF;//helpers::bitmask(8);
    data_[addr + 1] = (val >> 8) & 0xFF;//helpers::bitmask(8);
    data_[addr + 2] = (val >> 16) & 0xFF;//helpers::bitmask(8);
    data_[addr + 3] = (val >> 24) & 0xFF;//helpers::bitmask(8);
}

}