#include "isa.hpp"
#include "helpers.hpp"

namespace isa
{

Instruction::Instruction(RawInstruction raw_instr, InstructionInfo info)
    : raw_(raw_instr), info_(info)
{
    auto type = info_.type;
    auto code = raw_instr.code;

    switch (type)
    {
        case InstructionType::U:
            rd_ = helpers::get_rd(code);
            imm_ = helpers::get_imm_u(code);
            break;
        case InstructionType::R:
            rd_ = helpers::get_rd(code);
            rs1_ = helpers::get_rs1(code);
            rs2_ = helpers::get_rs2(code);
            funct3_ = helpers::get_funct3(code);
            funct7_ = helpers::get_funct7(code);
            if (helpers::get_opcode(raw_instr.code) == BASE_MATH_R_OPCODE)
            {
                auto it = instructions_base_math_map.find(funct3_.value());
                if (it == instructions_base_math_map.end())
                {
                    fprintf(stderr, "Unknown base math instruction\n"
                                    "Opcode: 0x%08x; funct3: 0x%08x Address: 0x%08x\n",
                                    BASE_MATH_R_OPCODE, funct3_.value(), raw_instr.address);
                    assert(0);
                }
                std::tie(base_math_instr_type_, additional_name_) = it->second;
            }
            break;
        case InstructionType::I:
            rd_ = helpers::get_rd(code);
            rs1_ = helpers::get_rs1(code);
            funct3_ = helpers::get_funct3(code);
            imm_ = helpers::get_imm_i(code);
            if (helpers::get_opcode(raw_instr.code) == BASE_MATH_I_OPCODE)
            {
                auto it = instructions_base_math_map.find(funct3_.value());
                if (it == instructions_base_math_map.end())
                {
                    fprintf(stderr, "Unknown base math instruction\n"
                                    "Opcode: 0x%08x; funct3: 0x%08x Address: 0x%08x\n",
                                    BASE_MATH_I_OPCODE, funct3_.value(), raw_instr.address);
                    assert(0);
                }
                std::tie(base_math_instr_type_, additional_name_) = it->second;
            }
            break;
        case InstructionType::S:
            rs1_ = helpers::get_rs1(code);
            rs2_ = helpers::get_rs2(code);
            funct3_ = helpers::get_funct3(code);
            imm_ = helpers::get_imm_s(code);
            break;
        case InstructionType::B:
            rs1_ = helpers::get_rs1(code);
            rs2_ = helpers::get_rs2(code);
            funct3_ = helpers::get_funct3(code);
            imm_ = helpers::get_imm_b(code);
            break;
        case InstructionType::J:
            rd_ = helpers::get_rd(code);
            imm_ = helpers::get_imm_j(code);
            break;
        default:

            break;
    }
}

uint32_t exec_lui(memory::Memory& memory, const isa::Instruction& instr)
{
    auto rd = instr.get_rd();
    auto imm = instr.get_imm();

    memory.set_int_reg(rd, imm);
    return memory.get_pc() + 1;
}

uint32_t exec_auipc(memory::Memory& memory, const isa::Instruction& instr)
{
    auto rd = instr.get_rd();
    auto imm = instr.get_imm();
    auto pc = memory.get_pc();

    memory.set_int_reg(rd, imm + pc);
    return pc + 1;
}

uint32_t exec_base_math_i(memory::Memory& memory, const isa::Instruction& instr)
{
    auto rd = instr.get_rd();
    uint32_t imm_unsigned = instr.get_imm();
    int imm_signed = helpers::bitcast<int>(imm_unsigned);
    auto rs1 = instr.get_rs1();
    auto type = instr.get_base_math_instr_type();
    int res = 0;
    int rs1_val_unsigned = memory.get_int_reg(rs1);
    int rs1_val_signed = helpers::bitcast<int>(rs1_val_unsigned);
    
    switch (type)
    {
        case BaseMathInstructionType::ADD_SUB:
            res = imm_signed + rs1_val_signed;
            break;
        case BaseMathInstructionType::SLT:
            res = rs1_val_signed < imm_signed ? 1 : 0;
            break;
        case BaseMathInstructionType::SLTU:
            res = rs1_val_unsigned < imm_unsigned ? 1 : 0;
            break;
        case BaseMathInstructionType::AND:
            res = rs1_val_signed & imm_signed;
            break;
        case BaseMathInstructionType::OR:
            res = rs1_val_signed | imm_signed;
            break;
        case BaseMathInstructionType::XOR:
            res = rs1_val_signed ^ imm_signed;
            break;
        case BaseMathInstructionType::SLL:
        {
            uint32_t shamt = imm_unsigned & 0x1F;
            res = helpers::bitcast<int>(rs1_val_unsigned << shamt);
            break;
        }
        case BaseMathInstructionType::SRL_SRA:
        {
            uint32_t shamt = imm_unsigned & 0x1F;
            if ((imm_unsigned & 0xFE0U) == 0x400) // SRAI
            {
                res = helpers::bitcast<int>(rs1_val_signed >> shamt);
            }
            else if ((imm_unsigned & 0xFE0U) == 0x0) // SRLI
            {
                res = helpers::bitcast<int>(rs1_val_unsigned >> shamt);
            }
            else
            {
                fprintf(stderr, "Unknown base math i instruction\n"
                                "Opcode: 0x%08x; Imm 0x%08x\n",
                                BASE_MATH_I_OPCODE, imm_unsigned);
                assert(0);
            }
            break;
        }
        default:
            assert(0);
    }

    memory.set_int_reg(rd, helpers::bitcast<uint32_t>(res));
    return memory.get_pc() + 1;
}

uint32_t exec_base_math_r(memory::Memory& memory, const isa::Instruction& instr)
{
    auto rd = instr.get_rd();
    auto rs1 = instr.get_rs1();
    auto rs2 = instr.get_rs2();
    auto type = instr.get_base_math_instr_type();
    auto funct7 = instr.get_funct7();
    int res = 0;
    int rs1_val_unsigned = memory.get_int_reg(rs1);
    int rs1_val_signed = helpers::bitcast<int>(rs1_val_unsigned);
    int rs2_val_unsigned = memory.get_int_reg(rs2);
    int rs2_val_signed = helpers::bitcast<int>(rs2_val_unsigned);
    
    switch (type)
    {
        case BaseMathInstructionType::ADD_SUB:
            if (funct7 == 0x20) // SUB
            {
                res = rs1_val_signed - rs2_val_signed;
            }
            else if (funct7 == 0x0) // ADD
            {
                res = rs1_val_signed + rs2_val_signed;
            }
            else
            {
                fprintf(stderr, "Unknown base math r instruction\n"
                                "Opcode: 0x%08x; funct7 0x%08x\n",
                                BASE_MATH_R_OPCODE, funct7);
                assert(0);
            }
            break;
        case BaseMathInstructionType::SLT:
            res = rs1_val_signed < rs2_val_signed ? 1 : 0;
            break;
        case BaseMathInstructionType::SLTU:
            res = rs1_val_unsigned < rs2_val_unsigned ? 1 : 0;
            break;
        case BaseMathInstructionType::AND:
            res = rs1_val_signed & rs2_val_signed;
            break;
        case BaseMathInstructionType::OR:
            res = rs1_val_signed | rs2_val_signed;
            break;
        case BaseMathInstructionType::XOR:
            res = rs1_val_signed ^ rs2_val_signed;
            break;
        case BaseMathInstructionType::SLL:
        {
            uint32_t shamt = rs2_val_unsigned & 0x1F;
            res = helpers::bitcast<int>(rs1_val_unsigned << shamt);
            break;
        }
        case BaseMathInstructionType::SRL_SRA:
        {
            uint32_t shamt = rs2_val_unsigned & 0x1F;
            if (funct7 == 0x20) // SRA
            {
                res = helpers::bitcast<int>(rs1_val_signed >> shamt);
            }
            else if (funct7 == 0x0) // SRL
            {
                res = helpers::bitcast<int>(rs1_val_unsigned >> shamt);
            }
            else
            {
                fprintf(stderr, "Unknown base math r instruction\n"
                                "Opcode: 0x%08x; funct7 0x%08x\n",
                                BASE_MATH_R_OPCODE, funct7);
                assert(0);
            }
            break;
        }
        default:
            assert(0);
    }

    memory.set_int_reg(rd, helpers::bitcast<uint32_t>(res));
    return memory.get_pc() + 1;
}

} // namespace isa