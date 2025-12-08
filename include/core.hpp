#pragma once

#include <memory>

#include "decoder.hpp"
#include "memory.hpp"

namespace core
{

const uint32_t  DEFAULT_STACK_TOP = 0x1000000;
const int       SP_REG = 2;

class Core
{
public:
    Core()
    {
        memory_ = std::make_shared<memory::Memory>(memory::DEFAULT_SIZE);
        decoder_ = std::make_shared<decoder::Decoder>();
    }

    void dump_instrs() const
    {
        for (auto& instr : instrs_)
        {
            fprintf(stderr, "address: 0x%08x; code: 0x%08x; name: %s\n",
                    instr->get_address(), instr->get_raw_code(), instr->get_name().c_str());
        }
    }

    void dump_instr(int idx) const
    {
        auto instr = instrs_[idx];
        fprintf(stderr, "address: 0x%08x; code: 0x%08x; name: %s\n",
                instr->get_address(), instr->get_raw_code(), instr->get_name().c_str());
    }

    void decode(const std::string& filename)
    {
        uint32_t start_pc = 0;
        instrs_ = decoder_->decode(filename, start_pc, memory_);
        memory_->set_zero_pc(instrs_[0]->get_address());
        memory_->set_pc(start_pc);

        memory_->set_int_reg(SP_REG, DEFAULT_STACK_TOP);
    }

    int run()
    {
        while (!memory_->is_exit())
        {
            auto instr_id = memory_->get_instr_index();
            assert(instr_id < instrs_.size());
            
            auto instr = instrs_[instr_id];
            memory_->set_pc(instr->execute(*memory_));
        }
        
        uint32_t exit_code = memory_->get_int_reg(10);
        return *(int*)(&exit_code);
    }

    void process_argv(int argc, char** argv)
    {
        memory_->store32(memory_->get_int_reg(2), argc - 2);
        uint32_t addr = memory::ARGV_ADDR;
        for (int i = 0; i < argc - 2; i++)
        {
            char* str = argv[i + 2];
            uint32_t len = strlen(str);

            memory_->write_bytes(addr, (const uint8_t*) str, len);
            memory_->store32(memory_->get_int_reg(2) + (i + 2) * 4, addr);

            addr += len;
            addr = padding(addr);
        }
    }

private:
    uint32_t padding(uint32_t addr)
    {
        while (addr % 4 != 0)
        {
            addr++;
        }
        return addr;
    }

    std::shared_ptr<decoder::Decoder> decoder_;
    std::shared_ptr<memory::Memory> memory_;

    std::vector<std::shared_ptr<isa::Instruction>> instrs_;

}; // class Core

} // namespace core