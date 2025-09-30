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
            break;
        case InstructionType::I:
            rd_ = helpers::get_rd(code);
            rs1_ = helpers::get_rs1(code);
            funct3_ = helpers::get_funct3(code);
            imm_ = helpers::get_imm_i(code);
            if (helpers::get_opcode(raw_instr.code) == BASE_MATH_OPCODE)
            {
                auto it = instructions_base_math_map.find(funct3_.value());
                if (it == instructions_base_math_map.end())
                {
                    fprintf(stderr, "Unknown base math instruction."
                                    "Opcode: 0x%08x; funct3: 0x%08x Address: 0x%08x\n",
                                    helpers::get_opcode(raw_instr.code), funct3_.value(), raw_instr.address);
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
}

uint32_t exec_base_math(memory::Memory& memory, const isa::Instruction& instr)
{
    auto rd = instr.get_rd();
    auto imm = instr.get_imm();
    auto rs1 = instr.get_rs1();
    auto type = instr.get_base_math_instr_type();
    uint32_t res = 0;
    uint32_t rs1_val = memory.get_int_reg(rs1);
    
    switch (type)
    {
        case BaseMathInstructionType::ADDI:
            res = imm + rs1_val;
            break;
        default:
            assert(0);
    }

    memory.set_int_reg(rd, res);
}

} // namespace isa