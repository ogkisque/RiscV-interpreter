#pragma once

#include <unordered_map>
#include <cstdint>
#include <string>
#include <utility>
#include <optional>

#include "memory.hpp"

namespace isa
{

enum class InstructionType
{
    R, I, S, B, U, J
};

const uint8_t BASE_MATH_OPCODE = 0b0010011;
enum class BaseMathInstructionType
{
    ADDI, SLTI, SLTIU, XORI, ORI, ANDI, SLLI, SRLI_SRAI
};

const std::unordered_map<uint8_t, std::pair<BaseMathInstructionType, std::string>>
instructions_base_math_map = {
    {0b000, {BaseMathInstructionType::ADDI, "addi"}},
    {0b010, {BaseMathInstructionType::SLTI, "slti"}},
    {0b011, {BaseMathInstructionType::SLTIU, "sltiu"}},
    {0b100, {BaseMathInstructionType::XORI, "xori"}},
    {0b110, {BaseMathInstructionType::ORI, "ori"}},
    {0b111, {BaseMathInstructionType::ANDI, "andi"}},
    {0b001, {BaseMathInstructionType::SLLI, "slli"}},
    {0b101, {BaseMathInstructionType::SRLI_SRAI, "srli/srai"}}
};

class Instruction;

using ExecuteFunction = uint32_t(*)(memory::Memory& memory, const Instruction& instr);

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

    Instruction(RawInstruction raw_instr, InstructionInfo info);

    uint32_t get_imm() const
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
        if (additional_name_.has_value())
        {
            return info_.name + "::" + additional_name_.value();
        }
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

    BaseMathInstructionType get_base_math_instr_type() const
    {
        assert(base_math_instr_type_.has_value());
        return base_math_instr_type_.value();
    }

    uint32_t execute(memory::Memory& memory)
    {
        return info_.func(memory, *this);
    }

private:
    RawInstruction raw_;
    InstructionInfo info_;

    std::optional<uint8_t> funct3_ = std::nullopt;
    std::optional<uint8_t> funct7_ = std::nullopt;
    std::optional<uint32_t> imm_ = std::nullopt;
    std::optional<uint8_t> rd_ = std::nullopt;
    std::optional<uint8_t> rs1_ = std::nullopt;
    std::optional<uint8_t> rs2_= std::nullopt;

    std::optional<BaseMathInstructionType> base_math_instr_type_ = std::nullopt;
    std::optional<std::string> additional_name_ = std::nullopt;
};

uint32_t exec_lui(memory::Memory& memory, const isa::Instruction& instr);

uint32_t exec_base_math(memory::Memory& memory, const isa::Instruction& instr);

const std::unordered_map<uint8_t, Instruction::InstructionInfo>
instructions_map = {
    {0b0110111, {InstructionType::U, "lui", exec_lui}},
    {BASE_MATH_OPCODE, {InstructionType::I, "base_math", exec_base_math}}
};

} // namespace isa