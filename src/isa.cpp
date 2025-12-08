#include "isa.hpp"
#include "helpers.hpp"

namespace isa
{

void Instruction::process_branch()
{
    auto it = instructions_branch_map.find(funct3_.value());
    if (it == instructions_branch_map.end())
    {
        fprintf(stderr, "Unknown base math instruction\n"
                        "Opcode: 0x%08x; funct3: 0x%08x\n",
                        BRANCH_OPCODE, funct3_.value());
        assert(0);
    }
    std::tie(branch_instr_type_, additional_name_) = it->second;
}

void Instruction::process_mul_math()
{
    auto it = instructions_mul_math_map.find(funct3_.value());
    if (it == instructions_mul_math_map.end())
    {
        fprintf(stderr, "Unknown mul math instruction\n"
                        "Opcode: 0x%08x; funct3: 0x%08x\n",
                        BASE_MATH_R_OPCODE, funct3_.value());
        assert(0);
    }
    std::tie(mul_math_instr_type_, additional_name_) = it->second;
}

void Instruction::process_base_math_r()
{
    auto it = instructions_base_math_map.find(funct3_.value());
    if (it == instructions_base_math_map.end())
    {
        fprintf(stderr, "Unknown base math instruction\n"
                        "Opcode: 0x%08x; funct3: 0x%08x\n",
                        BASE_MATH_R_OPCODE, funct3_.value());
        assert(0);
    }
    std::tie(base_math_instr_type_, additional_name_) = it->second;
}

void Instruction::process_f_base_math()
{
    auto it = instructions_f_base_math_map.find(funct7_.value());
    if (it == instructions_f_base_math_map.end())
    {
        fprintf(stderr, "Unknown f base math instruction\n"
                        "Opcode: 0x%08x; funct7: 0x%08x\n",
                        F_BASE_MATH_OPCODE, funct7_.value());
        assert(0);
    }
    std::tie(f_base_math_instr_type_, additional_name_) = it->second;
}

void Instruction::process_base_math_i()
{
    auto it = instructions_base_math_map.find(funct3_.value());
    if (it == instructions_base_math_map.end())
    {
        fprintf(stderr, "Unknown base math instruction\n"
                        "Opcode: 0x%08x; funct3: 0x%08x\n",
                        BASE_MATH_I_OPCODE, funct3_.value());
        assert(0);
    }
    std::tie(base_math_instr_type_, additional_name_) = it->second;
}

void Instruction::process_load()
{
    auto it = instructions_load_map.find(funct3_.value());
    if (it == instructions_load_map.end())
    {
        fprintf(stderr, "Unknown load instruction\n"
                        "Opcode: 0x%08x; funct3: 0x%08x\n",
                        LOAD_OPCODE, funct3_.value());
        assert(0);
    }
    std::tie(load_instr_type_, additional_name_) = it->second;
}

void Instruction::process_store()
{
    auto it = instructions_store_map.find(funct3_.value());
    if (it == instructions_store_map.end())
    {
        fprintf(stderr, "Unknown load instruction\n"
                        "Opcode: 0x%08x; funct3: 0x%08x\n",
                        STORE_OPCODE, funct3_.value());
        assert(0);
    }
    std::tie(store_instr_type_, additional_name_) = it->second;
}

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
                if (funct7_.value() == MUL_MATH_FUNCT7)
                {
                    process_mul_math();
                }
                else
                {
                    process_base_math_r();
                }
            }
            else if (helpers::get_opcode(raw_instr.code) == F_BASE_MATH_OPCODE)
            {
                process_f_base_math();
            }
            break;
        case InstructionType::I:
            rd_ = helpers::get_rd(code);
            rs1_ = helpers::get_rs1(code);
            funct3_ = helpers::get_funct3(code);
            imm_ = helpers::get_imm_i(code);
            if (helpers::get_opcode(raw_instr.code) == BASE_MATH_I_OPCODE)
            {
                process_base_math_i();
            }
            else if (helpers::get_opcode(raw_instr.code) == LOAD_OPCODE)
            {
                process_load();
            }
            break;
        case InstructionType::S:
            rs1_ = helpers::get_rs1(code);
            rs2_ = helpers::get_rs2(code);
            funct3_ = helpers::get_funct3(code);
            imm_ = helpers::get_imm_s(code);

            if (helpers::get_opcode(raw_instr.code) == STORE_OPCODE)
            {
                process_store();
            }
            break;
        case InstructionType::B:
            rs1_ = helpers::get_rs1(code);
            rs2_ = helpers::get_rs2(code);
            funct3_ = helpers::get_funct3(code);
            imm_ = helpers::get_imm_b(code);
            if (helpers::get_opcode(raw_instr.code) == BRANCH_OPCODE)
            {
                process_branch();
            }
            break;
        case InstructionType::J:
            rd_ = helpers::get_rd(code);
            imm_ = helpers::get_imm_j(code);
            break;
        case InstructionType::R4:
            rd_ = helpers::get_rd(code);
            rs1_ = helpers::get_rs1(code);
            rs2_ = helpers::get_rs2(code);
            rs3_ = helpers::get_rs3(code);
            funct3_ = helpers::get_funct3(code);
            funct7_ = helpers::get_funct7(code);
            break;
        default:
            fprintf(stderr, "Unknown instruction type\n");
            assert(0);
            break;
    }
}

uint32_t exec_lui(memory::Memory& memory, const isa::Instruction& instr)
{
    auto rd = instr.get_rd();
    auto imm = instr.get_imm();

    memory.set_int_reg(rd, imm);
    return memory.get_pc() + INSTR_SIZE;
}

uint32_t exec_auipc(memory::Memory& memory, const isa::Instruction& instr)
{
    auto rd = instr.get_rd();
    auto imm = instr.get_imm();
    auto pc = memory.get_pc();

    memory.set_int_reg(rd, imm + pc);
    return pc + INSTR_SIZE;
}

uint32_t exec_base_math_i(memory::Memory& memory, const isa::Instruction& instr)
{
    auto rd = instr.get_rd();
    uint32_t imm_unsigned = instr.get_imm();
    int imm_signed = helpers::bitcast<int>(imm_unsigned << 20) >> 20;
    auto rs1 = instr.get_rs1();
    auto type = instr.get_base_math_instr_type();
    int res = 0;
    uint32_t rs1_val_unsigned = memory.get_int_reg(rs1);
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
            uint8_t tmp_type = imm_unsigned >> 5;
            uint32_t shamt = imm_unsigned & 0x1F;

            if (tmp_type == 0b0000000) // SLL
            {
                res = helpers::bitcast<int>(rs1_val_unsigned << shamt);
            }
            else if (tmp_type == 0b0110000) // SEXT
            {
                if (shamt == 0b00100) // SEXT.B
                {
                    uint32_t tmp = rs1_val_unsigned & 0xFF;
                    res = helpers::bitcast<int>(tmp << 24) >> 24;
                }
                else if (shamt == 0b00101) // SEXT.H
                {
                    uint32_t tmp = rs1_val_unsigned & 0xFFFF;
                    res = helpers::bitcast<int>(tmp << 16) >> 16;
                }
                else
                {
                    fprintf(stderr, "Unknown sext instruction.\nshamt 0x%02x, addr 0x%08x\n",
                            shamt, instr.get_address());
                    assert(0);
                }
            }
            else
            {
                fprintf(stderr, "Unknown base math instruction.\nfunct7 0x%02x, addr 0x%08x\n",
                        tmp_type, instr.get_address());
                assert(0);
            }
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
            fprintf(stderr, "Unknown base math i instruction\n");
            assert(0);
            break;
    }

    memory.set_int_reg(rd, helpers::bitcast<uint32_t>(res));
    return memory.get_pc() + INSTR_SIZE;
}

uint32_t exec_base_math_r(memory::Memory& memory, const isa::Instruction& instr)
{
    auto rd = instr.get_rd();
    auto rs1 = instr.get_rs1();
    auto rs2 = instr.get_rs2();
    auto funct7 = instr.get_funct7();
    int res = 0;
    uint32_t rs1_val_unsigned = memory.get_int_reg(rs1);
    int rs1_val_signed = helpers::bitcast<int>(rs1_val_unsigned);
    uint32_t rs2_val_unsigned = memory.get_int_reg(rs2);
    int rs2_val_signed = helpers::bitcast<int>(rs2_val_unsigned);

    if (funct7 == MUL_MATH_FUNCT7)
    {
        auto type = instr.get_mul_math_instr_type();
        switch (type)
        {
            case MulMathInstructionType::MUL:
            {
                res = helpers::bitcast<int>((uint32_t)((uint64_t)rs1_val_unsigned *
                                                       (uint64_t)rs2_val_unsigned));
                break;
            }
            case MulMathInstructionType::MULH:
            {
                int64_t a = (int64_t) rs1_val_signed;
                int64_t b = (int64_t) rs2_val_signed;
                int64_t tmp = a * b;
                res = helpers::bitcast<int>((uint32_t)((uint64_t) tmp >> 32));
                break;
            }
            case MulMathInstructionType::MULHSU:
            {
                int64_t a = (int64_t) rs1_val_signed;
                int64_t b = (int64_t) rs2_val_unsigned;
                int64_t tmp = a * b;
                res = helpers::bitcast<int>((uint32_t)((uint64_t) tmp >> 32));
                break;
            }
            case MulMathInstructionType::MULHU:
            {
                uint64_t a = (uint64_t) rs1_val_unsigned;
                uint64_t b = (uint64_t) rs2_val_unsigned;
                uint64_t tmp = a * b;
                res = helpers::bitcast<int>((uint32_t)(tmp >> 32));
                break;
            }
            case MulMathInstructionType::DIV:
            {
                if (rs2_val_signed == 0)
                {
                    res = -1;
                }
                else if (rs1_val_signed == INT32_MIN && rs2_val_signed == -1) {
                    res = INT32_MIN;
                }
                else
                {
                    res = rs1_val_signed / rs2_val_signed;
                }
                break;
            }
            case MulMathInstructionType::DIVU:
            {
                if (rs2_val_unsigned == 0U)
                {
                    res = helpers::bitcast<int>(UINT32_MAX);
                }
                else
                {
                    res = helpers::bitcast<int>(rs1_val_unsigned / rs2_val_unsigned);
                }
                break;
            }
            case MulMathInstructionType::REM:
            {
                if (rs2_val_signed == 0)
                {
                    res = rs1_val_signed;
                }
                else if (rs1_val_signed == INT32_MIN && rs2_val_signed == -1)
                {
                    res = 0;
                }
                else
                {
                    res = rs1_val_signed % rs2_val_signed;
                }
                break;
            }
            case MulMathInstructionType::REMU:
            {
                if (rs2_val_unsigned == 0U)
                {
                    res = rs1_val_signed;
                }
                res = helpers::bitcast<int>(rs1_val_unsigned % rs2_val_unsigned);
                break;
            }
            default:
            {
                fprintf(stderr, "Unknown mul math instruction\n"
                                "Opcode: 0x%08x; funct7 0x%08x\n",
                                BASE_MATH_R_OPCODE, funct7);
                assert(0);
                break;
            }
        }
    }
    else
    {
        auto type = instr.get_base_math_instr_type();
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
                if (funct7 == 0b0100000) // ANDN
                {
                    res = rs1_val_signed & (~rs2_val_signed);
                }
                else if (funct7 == 0b0000000) // AND
                {
                    res = rs1_val_signed & rs2_val_signed;
                }
                else
                {
                    fprintf(stderr, "Unknown base math instruction\n");
                    assert(0);
                }
                break;
            case BaseMathInstructionType::OR:
                if (funct7 == 0b0100000) // ORN
                {
                    res = rs1_val_signed | (~rs2_val_signed);
                }
                else if (funct7 == 0b0000000) // OR
                {
                    res = rs1_val_signed | rs2_val_signed;
                }
                else
                {
                    fprintf(stderr, "Unknown base math instruction\n");
                    assert(0);
                }
                break;
            case BaseMathInstructionType::XOR:
                if (funct7 == 0b0100000) // XORN
                {
                    res = ~(rs1_val_signed ^ rs2_val_signed);
                }
                else if (funct7 == 0b0000000) // XOR
                {
                    res = rs1_val_signed ^ rs2_val_signed;
                }
                else if (funct7 == 0b0000100) // ZEXT.H
                {
                    res = rs1_val_signed & 0xFFFF;
                }
                else
                {
                    fprintf(stderr, "Unknown base math instruction.\nfunct7 0x%02x, addr 0x%08x\n",
                            funct7, instr.get_address());
                    assert(0);
                }
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
                fprintf(stderr, "Unknown base math r instruction\n"
                                "Opcode: 0x%08x; funct7 0x%08x\n",
                                BASE_MATH_R_OPCODE, funct7);
                assert(0);
                break;
        }
    }

    memory.set_int_reg(rd, helpers::bitcast<uint32_t>(res));
    return memory.get_pc() + INSTR_SIZE;
}

uint32_t exec_jalr(memory::Memory& memory, const isa::Instruction& instr)
{
    auto rd = instr.get_rd();
    auto rs1 = instr.get_rs1();
    assert(instr.get_funct3() == 0x0);

    uint32_t rs1_val_unsigned = memory.get_int_reg(rs1);

    uint32_t offs_unsigned = instr.get_imm();
    int offs_signed = (helpers::bitcast<int>(offs_unsigned << 20)) >> 20;

    uint32_t pc = memory.get_pc();
    uint32_t new_pc = (rs1_val_unsigned + helpers::bitcast<uint32_t>(offs_signed)) & (~1U);

    if (rd != 0)
    {
        memory.set_int_reg(rd, pc + INSTR_SIZE);
    }
    return new_pc;
}

uint32_t exec_jal(memory::Memory& memory, const isa::Instruction& instr)
{
    auto rd = instr.get_rd();
    uint32_t imm_unsigned = instr.get_imm();
    int imm_signed = (helpers::bitcast<int>(imm_unsigned << 11)) >> 11;

    uint32_t pc = memory.get_pc();
    int pc_signed = helpers::bitcast<int>(pc);
    uint32_t new_pc = helpers::bitcast<uint32_t>(pc_signed + imm_signed);

    if (rd != 0)
    {
        memory.set_int_reg(rd, pc + INSTR_SIZE);
    }

    return new_pc;
}

uint32_t exec_branch(memory::Memory& memory, const isa::Instruction& instr)
{
    auto type = instr.get_branch_instr_type();
    auto rs1 = instr.get_rs1();
    auto rs2 = instr.get_rs2();

    uint32_t imm_unsigned = instr.get_imm();
    int imm_signed = (helpers::bitcast<int>(imm_unsigned << 19)) >> 19;

    uint32_t rs1_unsigned = memory.get_int_reg(rs1);
    uint32_t rs2_unsigned = memory.get_int_reg(rs2);
    int rs1_signed = helpers::bitcast<int>(rs1_unsigned);
    int rs2_signed = helpers::bitcast<int>(rs2_unsigned);

    bool need_jump = false;
    auto pc_unsigned = memory.get_pc();
    int pc_signed = helpers::bitcast<int>(pc_unsigned);

    switch (type)
    {
        case BranchInstructionType::BEQ:
            need_jump = (rs1_unsigned == rs2_unsigned);
            break;
        case BranchInstructionType::BNE:
            need_jump = (rs1_unsigned != rs2_unsigned);
            break;
        case BranchInstructionType::BLT:
            need_jump = (rs1_signed < rs2_signed);
            break;
        case BranchInstructionType::BGE:
            need_jump = (rs1_signed >= rs2_signed);
            break;
        case BranchInstructionType::BLTU:
            need_jump = (rs1_unsigned < rs2_unsigned);
            break;
        case BranchInstructionType::BGEU:
            need_jump = (rs1_unsigned >= rs2_unsigned);
            break;
        default:
            break;
    }

    if (need_jump)
    {
        return pc_signed + imm_signed;
    }
    else
    {
        return pc_unsigned + INSTR_SIZE;
    }
}

uint32_t exec_load(memory::Memory& memory, const isa::Instruction& instr)
{
    auto rd = instr.get_rd();
    uint32_t imm_unsigned = instr.get_imm();
    int imm_signed = helpers::bitcast<int>(imm_unsigned << 20) >> 20;
    auto rs1 = instr.get_rs1();
    auto type = instr.get_load_instr_type();
    int res = 0;
    int rs1_val_unsigned = memory.get_int_reg(rs1);
    uint32_t addr = rs1_val_unsigned + helpers::bitcast<uint32_t>(imm_signed);

    switch (type)
    {
        case LoadInstructionType::LB:
        {
            uint8_t data = memory.load8(addr);
            res = (helpers::bitcast<int>((uint32_t) data << 24)) >> 24;
            break;
        }
        case LoadInstructionType::LH:
        {
            uint16_t data = memory.load16(addr);
            res = (helpers::bitcast<int>((uint32_t) data << 16)) >> 16;
            break;
        }
        case LoadInstructionType::LW:
        {
            uint32_t data = memory.load32(addr);
            res = helpers::bitcast<int>(data);
            break;
        }
        case LoadInstructionType::LBU:
        {
            uint8_t data = memory.load8(addr);
            res = helpers::bitcast<int>((uint32_t) data);
            break;
        }
        case LoadInstructionType::LHU:
        {
            uint16_t data = memory.load16(addr);
            res = helpers::bitcast<int>((uint32_t) data);
            break;
        }
        default:
        {
            fprintf(stderr, "Unknown load instruction\n");
            assert(0);
            break;
        }
    }

    if (rd != 0)
    {
        memory.set_int_reg(rd, helpers::bitcast<uint32_t>(res));
    }

    return memory.get_pc() + INSTR_SIZE;
}

uint32_t exec_store(memory::Memory& memory, const isa::Instruction& instr)
{
    auto type = instr.get_store_instr_type();
    uint32_t imm_unsigned = instr.get_imm();
    int imm_signed = helpers::bitcast<int>(imm_unsigned << 20) >> 20;
    auto rs1 = instr.get_rs1();
    auto rs1_val_unsigned = memory.get_int_reg(rs1);
    auto rs2 = instr.get_rs2();
    auto rs2_val_unsigned = memory.get_int_reg(rs2);
    uint32_t addr = rs1_val_unsigned + helpers::bitcast<uint32_t>(imm_signed);

    switch (type)
    {
        case StoreInstructionType::SB:
        {
            uint8_t data = (uint8_t) (rs2_val_unsigned & helpers::bitmask(8));
            memory.store8(addr, data);
            break;
        }
        case StoreInstructionType::SH:
        {
            uint16_t data = (uint16_t) (rs2_val_unsigned & helpers::bitmask(16));
            memory.store16(addr, data);
            break;
        }
        case StoreInstructionType::SW:
        {
            uint32_t data = rs2_val_unsigned;
            memory.store32(addr, data);
            break;
        }
        default:
        {
            fprintf(stderr, "Unknown store instruction\n");
            assert(0);
            break;
        }
    }

    return memory.get_pc() + INSTR_SIZE;
}

uint32_t exec_ecall(memory::Memory& memory, const isa::Instruction& instr)
{
    uint32_t type = memory.get_int_reg(17);
    int fd = helpers::bitcast<int>(memory.get_int_reg(10));
    uint32_t buf_addr = memory.get_int_reg(11);
    uint32_t count = memory.get_int_reg(12);

    switch (type)
    {
        case ECALL_READ_CODE:
        {
            std::vector<uint8_t> tmp(count);
            auto res = read(fd, tmp.data(), count);

            for (ssize_t i = 0; i < res; i++)
            {
                memory.store8(buf_addr + i, tmp[i]);
            }
            memory.set_int_reg(10, (uint32_t) res);
            break;
        }
        case ECALL_WRITE_CODE:
        {
            std::vector<uint8_t> tmp(count);
            for (size_t i = 0; i < count; i++)
            {
                tmp[i] = memory.load8(buf_addr + i);
            }

            auto res = write(fd, tmp.data(), count);
            assert(res == count);
            memory.set_int_reg(10, (uint32_t) res);
            break;
        }
        case ECALL_EXIT_CODE:
        {
            memory.set_exit();
            break;
        }
        default:
        {
            fprintf(stderr, "Unknown ecall instruction %u\n", type);
            assert(0);
            break;
        }
    }
    return memory.get_pc() + INSTR_SIZE;
}

uint32_t exec_f_load(memory::Memory& memory, const isa::Instruction& instr)
{
    auto rd = instr.get_rd();
    uint32_t imm_unsigned = instr.get_imm();
    int imm_signed = helpers::bitcast<int>(imm_unsigned << 20) >> 20;
    auto rs1 = instr.get_rs1();
    auto funct3 = instr.get_funct3();
    assert(funct3 == 0b010);
    int rs1_val_unsigned = memory.get_int_reg(rs1);
    uint32_t addr = rs1_val_unsigned + helpers::bitcast<uint32_t>(imm_signed);

    uint32_t data = memory.load32(addr);
    memory.set_float_reg(rd, helpers::bitcast<float>(data));

    return memory.get_pc() + INSTR_SIZE;
}

uint32_t exec_f_store(memory::Memory& memory, const isa::Instruction& instr)
{
    uint32_t imm_unsigned = instr.get_imm();
    int imm_signed = helpers::bitcast<int>(imm_unsigned << 20) >> 20;
    auto rs1 = instr.get_rs1();
    auto rs1_val_unsigned = memory.get_int_reg(rs1);
    auto rs2 = instr.get_rs2();
    auto rs2_val = memory.get_float_reg(rs2);
    uint32_t addr = rs1_val_unsigned + helpers::bitcast<uint32_t>(imm_signed);
    auto funct3 = instr.get_funct3();
    assert(funct3 == 0b010);

    memory.store32(addr, helpers::bitcast<uint32_t>(rs2_val));
    return memory.get_pc() + INSTR_SIZE;
}

uint32_t exec_fmadd(memory::Memory& memory, const isa::Instruction& instr)
{
    auto rd = instr.get_rd();
    auto rs1 = instr.get_rs1();
    auto rs2 = instr.get_rs2();
    auto rs3 = instr.get_rs3();
    float rs1_val = memory.get_float_reg(rs1);
    float rs2_val = memory.get_float_reg(rs2);
    float rs3_val = memory.get_float_reg(rs3);
    float res = 0.0;

    if (helpers::is_f_snan(rs1_val) || helpers::is_f_snan(rs2_val) || helpers::is_f_snan(rs3_val))
    {
        res = helpers::bitcast<float>(CANONICAL_NAN);
    }
    else
    {
        helpers::set_round(instr.get_funct3());
        res = std::fma(rs1_val, rs2_val, rs3_val);
    }

    memory.set_float_reg(rd, res);
    return memory.get_pc() + INSTR_SIZE;
}

uint32_t exec_fmsub(memory::Memory& memory, const isa::Instruction& instr)
{
    auto rd = instr.get_rd();
    auto rs1 = instr.get_rs1();
    auto rs2 = instr.get_rs2();
    auto rs3 = instr.get_rs3();
    float rs1_val = memory.get_float_reg(rs1);
    float rs2_val = memory.get_float_reg(rs2);
    float rs3_val = memory.get_float_reg(rs3);
    float res = 0.0;

    if (helpers::is_f_snan(rs1_val) || helpers::is_f_snan(rs2_val) || helpers::is_f_snan(rs3_val))
    {
        res = helpers::bitcast<float>(CANONICAL_NAN);
    }
    else
    {
        helpers::set_round(instr.get_funct3());
        res = std::fma(rs1_val, rs2_val, -rs3_val);
    }

    memory.set_float_reg(rd, res);
    return memory.get_pc() + INSTR_SIZE;
}

uint32_t exec_fnmadd(memory::Memory& memory, const isa::Instruction& instr)
{
    auto rd = instr.get_rd();
    auto rs1 = instr.get_rs1();
    auto rs2 = instr.get_rs2();
    auto rs3 = instr.get_rs3();
    float rs1_val = memory.get_float_reg(rs1);
    float rs2_val = memory.get_float_reg(rs2);
    float rs3_val = memory.get_float_reg(rs3);
    float res = 0.0;

    if (helpers::is_f_snan(rs1_val) || helpers::is_f_snan(rs2_val) || helpers::is_f_snan(rs3_val))
    {
        res = helpers::bitcast<float>(CANONICAL_NAN);
    }
    else
    {
        helpers::set_round(instr.get_funct3());
        res = std::fma(-rs1_val, rs2_val, -rs3_val);
    }

    memory.set_float_reg(rd, res);
    return memory.get_pc() + INSTR_SIZE;
}

uint32_t exec_fnmsub(memory::Memory& memory, const isa::Instruction& instr)
{
    auto rd = instr.get_rd();
    auto rs1 = instr.get_rs1();
    auto rs2 = instr.get_rs2();
    auto rs3 = instr.get_rs3();
    float rs1_val = memory.get_float_reg(rs1);
    float rs2_val = memory.get_float_reg(rs2);
    float rs3_val = memory.get_float_reg(rs3);
    float res = 0.0;

    if (helpers::is_f_snan(rs1_val) || helpers::is_f_snan(rs2_val) || helpers::is_f_snan(rs3_val))
    {
        res = helpers::bitcast<float>(CANONICAL_NAN);
    }
    else
    {
        helpers::set_round(instr.get_funct3());
        res = std::fma(-rs1_val, rs2_val, rs3_val);
    }

    memory.set_float_reg(rd, res);
    return memory.get_pc() + INSTR_SIZE;
}

uint32_t exec_f_base_math(memory::Memory& memory, const isa::Instruction& instr)
{
    auto type = instr.get_f_base_math_instr_type();
    auto rs1 = instr.get_rs1();
    float rs1_val = memory.get_float_reg(rs1);
    auto rs2 = instr.get_rs2();
    auto rd = instr.get_rd();
    float res = 0.0;
    auto funct3 = instr.get_funct3();
    bool int_rd = false;
    uint32_t int_res = 0;

    switch (type)
    {
        case FBaseMathInstructionType::FADD:
        {
            float rs2_val = memory.get_float_reg(rs2);
            if (helpers::is_f_snan(rs1_val) || helpers::is_f_snan(rs2_val))
            {
                res = helpers::bitcast<float>(CANONICAL_NAN);
                break;
            }
            helpers::set_round(funct3);
            res = rs1_val + rs2_val;
            break;
        }
        case FBaseMathInstructionType::FSUB:
        {
            float rs2_val = memory.get_float_reg(rs2);
            if (helpers::is_f_snan(rs1_val) || helpers::is_f_snan(rs2_val))
            {
                res = helpers::bitcast<float>(CANONICAL_NAN);
                break;
            }
            helpers::set_round(funct3);
            res = rs1_val - rs2_val;
            break;
        }
        case FBaseMathInstructionType::FMUL:
        {
            float rs2_val = memory.get_float_reg(rs2);
            if (helpers::is_f_snan(rs1_val) || helpers::is_f_snan(rs2_val))
            {
                res = helpers::bitcast<float>(CANONICAL_NAN);
                break;
            }
            helpers::set_round(funct3);
            res = rs1_val * rs2_val;
            break;
        }
        case FBaseMathInstructionType::FDIV:
        {
            float rs2_val = memory.get_float_reg(rs2);
            if (helpers::is_f_snan(rs1_val) || helpers::is_f_snan(rs2_val))
            {
                res = helpers::bitcast<float>(CANONICAL_NAN);
                break;
            }
            helpers::set_round(funct3);
            res = rs1_val / rs2_val;
            break;
        }
        case FBaseMathInstructionType::FSQRT:
        {
            if (helpers::is_f_snan(rs1_val))
            {
                res = helpers::bitcast<float>(CANONICAL_NAN);
                break;
            }
            helpers::set_round(funct3);
            res = std::sqrt(rs1_val);
            break;
        }
        case FBaseMathInstructionType::FSGNJ:
        {
            float rs2_val = memory.get_float_reg(rs2);
            uint32_t rs1_unsigned = helpers::bitcast<uint32_t>(rs1_val);
            uint32_t rs2_unsigned = helpers::bitcast<uint32_t>(rs2_val);
            uint32_t sign, mant_exp;
            switch (funct3)
            {
                case 0b000: // fsgnj.s
                    sign = (rs2_unsigned >> 31) & 1U;
                    mant_exp = rs1_unsigned & 0x7FFFFFFFU;
                    break;
                case 0b001: // fsgnjn.s
                    sign = (~(rs2_unsigned >> 31)) & 1U;
                    mant_exp = rs1_unsigned & 0x7FFFFFFFU;
                    break;
                case 0b010: // fsgnjx.s
                    sign = ((rs1_unsigned >> 31) ^ (rs2 >> 31)) & 1U;
                    mant_exp = rs1_unsigned & 0x7FFFFFFFU;
                    break;
                default:
                    fprintf(stderr, "Unknown fsgnj instruction\nCode: 0x%08x, addr: 0x%08x, funct3: 0x%02x\n",
                                    instr.get_raw_code(), instr.get_address(), funct3);
                    assert(0);
                    break;
            }
            res = helpers::bitcast<float>(mant_exp | (sign << 31));
            break;
        }
        case FBaseMathInstructionType::FMIN_FMAX:
        {
            float rs2_val = memory.get_float_reg(rs2);
            if (helpers::is_f_snan(rs1_val) && helpers::is_f_snan(rs2_val))
            {
                res = helpers::bitcast<float>(CANONICAL_NAN);
                break;
            }
            else if (helpers::is_f_snan(rs1_val))
            {
                res = rs2_val;
                break;
            }
            else if (helpers::is_f_snan(rs2_val))
            {
                res = rs1_val;
                break;
            }

            switch (funct3)
            {
                case 0b000: // fmin
                    res = std::fminf(rs1_val, rs2_val);
                    break;
                case 0b001: // fmax
                    res = std::fmaxf(rs1_val, rs2_val);
                    break;
                default:
                    fprintf(stderr, "Unknown fmax_fmin instruction\nCode: 0x%08x, addr: 0x%08x, funct3: 0x%02x\n",
                                    instr.get_raw_code(), instr.get_address(), funct3);
                    assert(0);
                    break;
            }
            break;
        }
        case FBaseMathInstructionType::FCVT_W_S:
        {
            int_rd = true;
            switch (rs2)
            {
                case 0b0: // fcvt_w_s
                    if (helpers::is_f_snan(rs1_val))
                    {
                        int_res = helpers::bitcast<uint32_t>(INT32_MAX);
                    }
                    helpers::set_round(funct3);
                    int_res = static_cast<uint32_t>(static_cast<int32_t>(std::rintf(rs1_val)));
                    break;
                case 0b1: // fcvt_wu_s
                    if (helpers::is_f_nan(rs1_val))
                    {
                        int_res = UINT32_MAX;
                    }
                    helpers::set_round(funct3);
                    int_res = static_cast<uint32_t>(std::rintf(rs1_val));
                    break;
                default:
                    fprintf(stderr, "Unknown fcvt_w_s instruction\nCode: 0x%08x, addr: 0x%08x, rs2: 0x%02x\n",
                                    instr.get_raw_code(), instr.get_address(), rs2);
                    assert(0);
                    break;
            }
            break;
        }
        case FBaseMathInstructionType::FMV_X_W_FCLASS:
        {
            int_rd = true;
            switch (funct3)
            {
                case 0b0: // fmv_x_w
                {
                    int_res = helpers::bitcast<uint32_t>(rs1_val);
                    break;
                }
                case 0b1: // fclass
                {
                    uint32_t u = helpers::bitcast<uint32_t>(rs1_val);
                    uint32_t exp = (u >> 23) & 0xFF;
                    uint32_t frac = u & 0x7FFFFFu;
                    uint32_t sign = (u >> 31) & 1u;
                    if (exp == 0xFF)
                    {
                        if (frac == 0)
                        {
                            if (sign)
                                int_res |= (1u << 0);
                            else
                                int_res |= (1u << 7);
                        }
                        else
                        {
                            if (helpers::is_f_snan(u))
                                int_res |= (1u << 8);
                            else
                                int_res |= (1u << 9);
                        }
                    }
                    else if (exp == 0)
                    {
                        if (frac == 0)
                        {
                            if (sign)
                                int_res |= (1u << 3);
                            else
                                int_res |= (1u << 4);
                        }
                        else
                        {
                            if (sign)
                                int_res |= (1u << 2);
                            else
                                int_res |= (1u << 5);
                        }
                    }
                    else
                    {
                        if (sign)
                            int_res |= (1u << 1);
                        else
                            int_res |= (1u << 6);
                    }
                    break;
                }
                default:
                    fprintf(stderr, "Unknown fcvt_s_w instruction\nCode: 0x%08x, addr: 0x%08x, funct3: 0x%02x\n",
                                    instr.get_raw_code(), instr.get_address(), funct3);
                    assert(0);
                    break;
            }
            break;
        }
        case FBaseMathInstructionType::FEQ_FLT_FLE:
        {
            float rs2_val = memory.get_float_reg(rs2);
            int_rd = true;
            if (helpers::is_f_snan(rs1_val) || helpers::is_f_snan(rs2_val))
            {
                int_res = 0;
                break;
            }

            switch (funct3)
            {
                case 0b000: // fle
                    int_res = (rs1_val <= rs2_val) ? 1 : 0;
                    break;
                case 0b001: // flt
                    int_res = (rs1_val < rs2_val) ? 1 : 0;
                    break;
                case 0b010: // feq
                    int_res = (rs1_val == rs2_val) ? 1 : 0;
                    break;
                default:
                    fprintf(stderr, "Unknown fcmp instruction\nCode: 0x%08x, addr: 0x%08x, funct3: 0x%02x\n",
                                    instr.get_raw_code(), instr.get_address(), funct3);
                    assert(0);
                    break;
            }
            break;
        }
        case FBaseMathInstructionType::FCVT_S_W:
        {
            uint32_t rs1_int = memory.get_int_reg(rs1);
            helpers::set_round(funct3);
            switch (rs2)
            {
                case 0b0: // fcvt_s_w
                    res = static_cast<float>(helpers::bitcast<int>(rs1_int));
                    break;
                case 0b1: // fcvt_s_wu
                    res = static_cast<float>(rs1_int);
                    break;
                default:
                    fprintf(stderr, "Unknown fcvt_s_w instruction\nCode: 0x%08x, addr: 0x%08x, rs2: 0x%02x\n",
                                    instr.get_raw_code(), instr.get_address(), rs2);
                    assert(0);
                    break;
            }
            break;
        }
        case FBaseMathInstructionType::FMV_W_X:
        {
            uint32_t rs1_int = memory.get_int_reg(rs1);
            res = helpers::bitcast<float>(rs1_int);
            break;
        }
        default:
        {
            fprintf(stderr, "Unknown f base math instruction\n"
                            "Code: 0x%08x; addr: 0x%08x; funct7 0x%02x\n",
                            instr.get_raw_code(), instr.get_address(), instr.get_funct7());
            assert(0);
            break;
        }
    }

    if (int_rd)
    {
        memory.set_int_reg(rd, int_res);
    }
    else
    {
        memory.set_float_reg(rd, res);
    }

    return memory.get_pc() + INSTR_SIZE;
}

} // namespace isa