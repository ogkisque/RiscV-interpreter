#pragma once

#include <unordered_map>
#include <cstdint>
#include <string>
#include <utility>
#include <optional>

#include "core.hpp"

namespace isa
{

enum class InstructionType
{
    R, I, S, B, U, J
};

class Instruction;

using ExecuteFunction = uint32_t(*)(core::Core& core, const Instruction& instr);

class Instruction
{
public:
    struct RawInstruction
    {
        uint32_t code;
        uint32_t address;
    };

    struct InstructionInfo
    {
        InstructionType type;
        std::string name;
        ExecuteFunction func;
    };

    Instruction(RawInstruction raw_instr, InstructionInfo info) : 
        raw_(raw_instr), info_(info)
    {
        auto type = info_.type;
        auto code = raw_instr.code;

        switch (type)
        {
            case InstructionType::U:
                rd_ = code >> 7 & 0x1F;
                imm_ = code >> 12;
                break;
            default:
                break;
        }
    }

    int get_imm() const
    {
        assert(imm_.has_value());
        return imm_.value();
    }

    uint8_t get_rd() const
    {
        assert(rd_.has_value());
        return rd_.value();
    }

    uint8_t get_rs1() const
    {
        assert(rs1_.has_value());
        return rs1_.value();
    }

    uint8_t get_rs2() const
    {
        assert(rs2_.has_value());
        return rs2_.value();
    }

    uint8_t get_funct3() const
    {
        assert(funct3_.has_value());
        return funct3_.value();
    }

    uint8_t get_funct7() const
    {
        assert(funct7_.has_value());
        return funct7_.value();
    }

    std::string get_name() const
    {
        return info_.name;
    }

    uint32_t get_address() const
    {
        return raw_.address;
    }

    uint32_t get_raw_code() const
    {
        return raw_.code;
    }

private:
    RawInstruction raw_;
    InstructionInfo info_;

    std::optional<uint8_t> funct3_ = std::nullopt;
    std::optional<uint8_t> funct7_ = std::nullopt;
    std::optional<int> imm_ = std::nullopt;
    std::optional<uint8_t> rd_ = std::nullopt;
    std::optional<uint8_t> rs1_ = std::nullopt;
    std::optional<uint8_t> rs2_= std::nullopt;
};

uint32_t exec_lui(core::Core& core, const isa::Instruction& instr)
{
    auto rd = instr.get_rd();
    auto imm = instr.get_imm();

    core.set_int_reg(rd, imm);
}

std::unordered_map<uint8_t, Instruction::InstructionInfo> instructions_map = 
    {
        {0b0110111, {InstructionType::U, "lui", exec_lui}}
    };

} // namespace isa