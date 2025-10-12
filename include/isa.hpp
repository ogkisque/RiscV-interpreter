#pragma once

#include <unordered_map>
#include <cstdint>
#include <string>
#include <utility>
#include <optional>
#include <unistd.h>
#include <cmath>

#include "memory.hpp"

namespace isa
{

const uint32_t INSTR_SIZE =         0x4;
const int ECALL_READ_CODE =         3;
const int ECALL_WRITE_CODE =        64;
const int ECALL_EXIT_CODE =         93;
const uint32_t MUL_MATH_FUNCT7 =    0b1;

enum class InstructionType
{
    R, I, S, B, U, J, R4
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

enum class MulMathInstructionType
{
    MUL, MULH, MULHSU, MULHU, DIV, DIVU, REM, REMU
};

const std::unordered_map<uint8_t, std::pair<MulMathInstructionType, std::string>>
instructions_mul_math_map = {
    {0b000, {MulMathInstructionType::MUL, "mul"}},
    {0b001, {MulMathInstructionType::MULH, "mulh"}},
    {0b010, {MulMathInstructionType::MULHSU, "mulhsu"}},
    {0b011, {MulMathInstructionType::MULHU, "mulhu"}},
    {0b100, {MulMathInstructionType::DIV, "div"}},
    {0b101, {MulMathInstructionType::DIVU, "divu"}},
    {0b110, {MulMathInstructionType::REM, "rem"}},
    {0b111, {MulMathInstructionType::REMU, "remu"}}
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

const uint8_t F_BASE_MATH_OPCODE = 0b1010011;
enum class FBaseMathInstructionType
{
    FADD, FSUB, FMUL, FDIV, FSQRT, FSGNJ, FMIN_FMAX,
    FCVT_W_S, FMV_X_W_FCLASS, FEQ_FLT_FLE, FCVT_S_W, FMV_W_X
};

const std::unordered_map<uint8_t, std::pair<FBaseMathInstructionType, std::string>>
instructions_f_base_math_map = {
    {0b0000000, {FBaseMathInstructionType::FADD, "fadd"}},
    {0b0000100, {FBaseMathInstructionType::FSUB, "fsub"}},
    {0b0001000, {FBaseMathInstructionType::FMUL, "fmul"}},
    {0b0001100, {FBaseMathInstructionType::FDIV, "fdiv"}},
    {0b0101100, {FBaseMathInstructionType::FSQRT, "fsqrt"}},
    {0b0010000, {FBaseMathInstructionType::FSGNJ, "fsgnj"}},
    {0b0010100, {FBaseMathInstructionType::FMIN_FMAX, "fmin/fmax"}},
    {0b1100000, {FBaseMathInstructionType::FCVT_W_S, "fcvt_w_s"}},
    {0b1110000, {FBaseMathInstructionType::FMV_X_W_FCLASS, "fmv_x_w/fclass"}},
    {0b1010000, {FBaseMathInstructionType::FEQ_FLT_FLE, "feq/flt/fle"}},
    {0b1101000, {FBaseMathInstructionType::FCVT_S_W, "fcvt_s_w"}},
    {0b1111000, {FBaseMathInstructionType::FMV_W_X, "fmv_w_x"}}
};

const uint32_t CANONICAL_NAN = 0x7FC00000;

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

    uint8_t get_rs3() const
    {
        assert(rs3_.has_value());
        return rs3_.value();
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

    FBaseMathInstructionType get_f_base_math_instr_type() const
    {
        assert(f_base_math_instr_type_.has_value());
        return f_base_math_instr_type_.value();
    }

    MulMathInstructionType get_mul_math_instr_type() const
    {
        assert(mul_math_instr_type_.has_value());
        return mul_math_instr_type_.value();
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
    std::optional<uint8_t> rs3_= std::nullopt;

    std::optional<BaseMathInstructionType> base_math_instr_type_ = std::nullopt;
    std::optional<BranchInstructionType> branch_instr_type_ = std::nullopt;
    std::optional<LoadInstructionType> load_instr_type_ = std::nullopt;
    std::optional<StoreInstructionType> store_instr_type_ = std::nullopt;
    std::optional<MulMathInstructionType> mul_math_instr_type_ = std::nullopt;
    std::optional<FBaseMathInstructionType> f_base_math_instr_type_ = std::nullopt;
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
uint32_t exec_f_load(memory::Memory& memory, const isa::Instruction& instr);
uint32_t exec_f_store(memory::Memory& memory, const isa::Instruction& instr);
uint32_t exec_fmadd(memory::Memory& memory, const isa::Instruction& instr);
uint32_t exec_fmsub(memory::Memory& memory, const isa::Instruction& instr);
uint32_t exec_fnmadd(memory::Memory& memory, const isa::Instruction& instr);
uint32_t exec_fnmsub(memory::Memory& memory, const isa::Instruction& instr);
uint32_t exec_f_base_math(memory::Memory& memory, const isa::Instruction& instr);

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
    {0b1110011,             {InstructionType::I, "ecall", exec_ecall}},
    {0b0000111,             {InstructionType::I, "f_load", exec_f_load}},
    {0b0100111,             {InstructionType::S, "f_store", exec_f_store}},
    {0b1000011,             {InstructionType::R4, "fmadd_s", exec_fmadd}},
    {0b1000111,             {InstructionType::R4, "fmsub_s", exec_fmsub}},
    {0b1001111,             {InstructionType::R4, "fnmadd_s", exec_fnmadd}},
    {0b1001011,             {InstructionType::R4, "fnmsub_s", exec_fnmsub}},
    {F_BASE_MATH_OPCODE,    {InstructionType::R, "f_base_math", exec_f_base_math}}
};

} // namespace isa