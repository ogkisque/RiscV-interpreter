#pragma once

#include <memory>

#include "decoder.hpp"
#include "memory.hpp"

namespace core
{

class Core
{
public:
    void dump_instr() const
    {
        for (auto& instr : instrs_)
        {
            printf("address: 0x%08x; code: 0x%08x; name: %s\n",
                    instr->get_address(), instr->get_raw_code(), instr->get_name().c_str());
        }
    }

    void decode(const std::string& filename)
    {
        instrs_ = decoder_->decode(filename);
    }

    void run()
    {
        int num_to_rum = 1;
        for (int i = 0; i < num_to_rum; i++)
        {
            auto instr = instrs_[memory_->get_pc()];
            instr->
        }
    }

private:
    std::shared_ptr<decoder::Decoder> decoder_;
    std::shared_ptr<memory::Memory> memory_;

    std::vector<std::shared_ptr<isa::Instruction>> instrs_;

}; // class Core

} // namespace core