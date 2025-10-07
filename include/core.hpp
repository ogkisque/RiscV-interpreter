#pragma once

#include <memory>

#include "decoder.hpp"
#include "memory.hpp"

namespace core
{

class Core
{
public:
    Core()
    {
        memory_ = std::make_shared<memory::Memory>();
        decoder_ = std::make_shared<decoder::Decoder>();
    }

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
        uint32_t start_pc = 0;
        instrs_ = decoder_->decode(filename, start_pc, memory_);
        memory_->set_zero_pc(instrs_[0]->get_address());
        memory_->set_pc(start_pc);
    }

    void run()
    {
        while (!memory_->is_exit())
        {
            memory_->dump_regs();
            auto instr_id = memory_->get_instr_index();
            assert(instr_id < instrs_.size());
            
            auto instr = instrs_[instr_id];
            memory_->set_pc(instr->execute(*memory_));
        }
    }

private:
    std::shared_ptr<decoder::Decoder> decoder_;
    std::shared_ptr<memory::Memory> memory_;

    std::vector<std::shared_ptr<isa::Instruction>> instrs_;

}; // class Core

} // namespace core