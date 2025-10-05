#pragma once

#include <unordered_map>
#include <cstdint>
#include <string>
#include <utility>
#include <optional>

#include "memory.hpp"

namespace isa
{

const uint32_t INSTR_SIZE = 0x4;

enum class InstructionType
{
    R, I, S, B, U, J
};

const uint8_t BASE_MATH_I_OPCODE = 0b0010011;
const uint8_t BASE_MATH_R_OPCODE = 0b0110011;
enum class BaseMathInstructionType
{
    ADD_SUB, SLT, SLTU, XOR, OR, AND, SLL, SRL_SRA
};

const std::unordered_map<uint8_t, std::pair<BaseMathInstructionType, std::string>>
instructions_base_math_map = {
    {0b000, {BaseMathInstructionType::ADD_SUB, "add/sub"}},
    {0b010, {BaseMathInstructionType::SLT, "slt"}},
    {0b011, {BaseMathInstructionType::SLTU, "sltu"}},
    {0b100, {BaseMathInstructionType::XOR, "xor"}},
    {0b110, {BaseMathInstructionType::OR, "or"}},
    {0b111, {BaseMathInstructionType::AND, "and"}},
    {0b001, {BaseMathInstructionType::SLL, "sll"}},
    {0b101, {BaseMathInstructionType::SRL_SRA, "srl/sra"}}
};

const uint8_t BRANCH_OPCODE = 0b1100011;
enum class BranchInstructionType
{
    BEQ, BNE, BLT, BGE, BLTU, BGEU
};

const std::unordered_map<uint8_t, std::pair<BranchInstructionType, std::string>>
instructions_branch_map = {
    {0b000, {BranchInstructionType::BEQ, "beq"}},
    {0b001, {BranchInstructionType::BNE, "bne"}},
    {0b100, {BranchInstructionType::BLT, "blt"}},
    {0b101, {BranchInstructionType::BGE, "bge"}},
    {0b110, {BranchInstructionType::BLTU, "bltu"}},
    {0b111, {BranchInstructionType::BGEU, "bgeu"}}
};

const uint8_t LOAD_OPCODE = 0b0000011;
enum class LoadInstructionType
{
    LB, LH, LW, LBU, LHU
};

const std::unordered_map<uint8_t, std::pair<LoadInstructionType, std::string>>
instructions_load_map = {
    {0b000, {LoadInstructionType::LB, "lb"}},
    {0b001, {LoadInstructionType::LH, "lh"}},
    {0b010, {LoadInstructionType::LW, "lw"}},
    {0b100, {LoadInstructionType::LBU, "lbu"}},
    {0b101, {LoadInstructionType::LHU, "lhu"}}
};

const uint8_t STORE_OPCODE = 0b0100011;
enum class StoreInstructionType
{
    SB, SH, SW
};

const std::unordered_map<uint8_t, std::pair<StoreInstructionType, std::string>>
instructions_store_map = {
    {0b000, {StoreInstructionType::SB, "sb"}},
    {0b001, {StoreInstructionType::SH, "sh"}},
    {0b010, {StoreInstructionType::SW, "sw"}}
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

    BranchInstructionType get_branch_instr_type() const
    {
        assert(branch_instr_type_.has_value());
        return branch_instr_type_.value();
    }

    LoadInstructionType get_load_instr_type() const
    {
        assert(load_instr_type_.has_value());
        return load_instr_type_.value();
    }

    StoreInstructionType get_store_instr_type() const
    {
        assert(store_instr_type_.has_value());
        return store_instr_type_.value();
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
    std::optional<BranchInstructionType> branch_instr_type_ = std::nullopt;
    std::optional<LoadInstructionType> load_instr_type_ = std::nullopt;
    std::optional<StoreInstructionType> store_instr_type_ = std::nullopt;
    std::optional<std::string> additional_name_ = std::nullopt;
};

uint32_t exec_lui(memory::Memory& memory, const isa::Instruction& instr);
uint32_t exec_auipc(memory::Memory& memory, const isa::Instruction& instr);
uint32_t exec_base_math_i(memory::Memory& memory, const isa::Instruction& instr);
uint32_t exec_base_math_r(memory::Memory& memory, const isa::Instruction& instr);
uint32_t exec_jalr(memory::Memory& memory, const isa::Instruction& instr);
uint32_t exec_jal(memory::Memory& memory, const isa::Instruction& instr);
uint32_t exec_branch(memory::Memory& memory, const isa::Instruction& instr);
uint32_t exec_load(memory::Memory& memory, const isa::Instruction& instr);
uint32_t exec_store(memory::Memory& memory, const isa::Instruction& instr);
uint32_t exec_ecall(memory::Memory& memory, const isa::Instruction& instr);

const std::unordered_map<uint8_t, Instruction::InstructionInfo>
instructions_map = {
    {0b0110111,             {InstructionType::U, "lui", exec_lui}},
    {0b0010111,             {InstructionType::U, "auipc", exec_auipc}},
    {BASE_MATH_I_OPCODE,    {InstructionType::I, "base_math_i", exec_base_math_i}},
    {BASE_MATH_R_OPCODE,    {InstructionType::R, "base_math_r", exec_base_math_r}},
    {0b1100111,             {InstructionType::I, "jalr", exec_jalr}},
    {0b1101111,             {InstructionType::J, "jal", exec_jal}},
    {BRANCH_OPCODE,         {InstructionType::B, "branch", exec_branch}},
    {LOAD_OPCODE,           {InstructionType::I, "load", exec_load}},
    {STORE_OPCODE,          {InstructionType::S, "store", exec_store}},
    {0b1110011,             {InstructionType::I, "ecall", exec_ecall}}
};

} // namespace isa