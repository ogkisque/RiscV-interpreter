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
            else if (helpers::get_opcode(raw_instr.code) == LOAD_OPCODE)
            {
                auto it = instructions_load_map.find(funct3_.value());
                if (it == instructions_load_map.end())
                {
                    fprintf(stderr, "Unknown load instruction\n"
                                    "Opcode: 0x%08x; funct3: 0x%08x Address: 0x%08x\n",
                                    LOAD_OPCODE, funct3_.value(), raw_instr.address);
                    assert(0);
                }
                std::tie(load_instr_type_, additional_name_) = it->second;
            }
            break;
        case InstructionType::S:
            rs1_ = helpers::get_rs1(code);
            rs2_ = helpers::get_rs2(code);
            funct3_ = helpers::get_funct3(code);
            imm_ = helpers::get_imm_s(code);

            if (helpers::get_opcode(raw_instr.code) == STORE_OPCODE)
            {
                auto it = instructions_store_map.find(funct3_.value());
                if (it == instructions_store_map.end())
                {
                    fprintf(stderr, "Unknown load instruction\n"
                                    "Opcode: 0x%08x; funct3: 0x%08x Address: 0x%08x\n",
                                    STORE_OPCODE, funct3_.value(), raw_instr.address);
                    assert(0);
                }
                std::tie(store_instr_type_, additional_name_) = it->second;
            }
            break;
        case InstructionType::B:
            rs1_ = helpers::get_rs1(code);
            rs2_ = helpers::get_rs2(code);
            funct3_ = helpers::get_funct3(code);
            imm_ = helpers::get_imm_b(code);
            if (helpers::get_opcode(raw_instr.code) == BRANCH_OPCODE)
            {
                auto it = instructions_branch_map.find(funct3_.value());
                if (it == instructions_branch_map.end())
                {
                    fprintf(stderr, "Unknown base math instruction\n"
                                    "Opcode: 0x%08x; funct3: 0x%08x Address: 0x%08x\n",
                                    BRANCH_OPCODE, funct3_.value(), raw_instr.address);
                    assert(0);
                }
                std::tie(branch_instr_type_, additional_name_) = it->second;
            }
            break;
        case InstructionType::J:
            rd_ = helpers::get_rd(code);
            imm_ = helpers::get_imm_j(code);
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
            fprintf(stderr, "Unknown base math r instruction\n"
                            "Opcode: 0x%08x; funct7 0x%08x\n",
                            BASE_MATH_R_OPCODE, funct7);
            assert(0);
            break;
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
    int rs1_val_signed = helpers::bitcast<int>(rs1_val_unsigned);

    uint32_t offs_unsigned = instr.get_imm();
    int offs_signed = (helpers::bitcast<int>(offs_unsigned << 20)) >> 20;

    uint32_t pc = memory.get_pc();
    uint32_t new_pc = (rs1_val_unsigned + helpers::bitcast<uint32_t>(offs_signed)) & (~1U);
    //fprintf(stderr, "JALR INSTR; rs1 0x%08x (r%u); imm 0x%08x; new pc 0x%08x\n",
            //rs1_val_unsigned, rs1, helpers::bitcast<uint32_t>(offs_signed), new_pc);
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
    //fprintf(stderr, "LOAD INSTR; rs1 0x%08x; imm 0x%08x; addr 0x%08x\n",
            //rs1_val_unsigned, helpers::bitcast<uint32_t>(imm_signed), addr);

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
    //fprintf(stderr, "STORE INSTR; rs1 0x%08x; imm 0x%08x; addr 0x%08x; val 0x%08x\n",
            //rs1_val_unsigned, helpers::bitcast<uint32_t>(imm_signed), addr, rs2_val_unsigned);

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
            //fprintf(stderr, "DATA 0x%08x\n", data);
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
            fprintf(stderr, "Unknown ecall instruction\n");
            assert(0);
            break;
        }
    }
    return memory.get_pc() + INSTR_SIZE;
}

} // namespace isa