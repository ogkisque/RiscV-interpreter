#pragma once

#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <string>
#include <cstdint>
#include <bitset>
#include <cassert>

#include "isa.hpp"

namespace decoder
{

class Decoder
{
private:
    typedef uint32_t Elf32Addr;
    typedef uint16_t Elf32Half;
    typedef uint32_t Elf32Off;
    typedef uint32_t Elf32Word;

    struct Elf32Ehdr
    {
        unsigned char  e_ident[16];
        Elf32Half      e_type;
        Elf32Half      e_machine;
        Elf32Word      e_version;
        Elf32Addr      e_entry;
        Elf32Off       e_phoff;
        Elf32Off       e_shoff;
        Elf32Word      e_flags;
        Elf32Half      e_ehsize;
        Elf32Half      e_phentsize;
        Elf32Half      e_phnum;
        Elf32Half      e_shentsize;
        Elf32Half      e_shnum;
        Elf32Half      e_shstrndx;
    };

    struct Elf32Shdr
    {
        Elf32Word sh_name;
        Elf32Word sh_type;
        Elf32Word sh_flags;
        Elf32Addr sh_addr;
        Elf32Off  sh_offset;
        Elf32Word sh_size;
        Elf32Word sh_link;
        Elf32Word sh_info;
        Elf32Word sh_addralign;
        Elf32Word sh_entsize;
    };

    struct Elf32Phdr
    {
        Elf32Word p_type;
        Elf32Off  p_offset;
        Elf32Addr p_vaddr;
        Elf32Addr p_paddr;
        Elf32Word p_filesz;
        Elf32Word p_memsz;
        Elf32Word p_flags;
        Elf32Word p_align;
    };

    uint32_t readU32(const unsigned char* data)
    {
        return data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
    }

    uint16_t readU16(const unsigned char* data)
    {
        return data[0] | (data[1] << 8);
    }

    void parse_elf(const std::string& filename, uint32_t& start_pc, std::shared_ptr<memory::Memory> mem)
    {
        std::ifstream file(filename, std::ios::binary);
        assert(file.is_open());
        file.seekg(0, std::ios::end);
        std::streamsize fileSize = file.tellg();
        file.seekg(0, std::ios::beg);
        assert(fileSize > 0);

        std::vector<unsigned char> fileData(fileSize);
        file.read(reinterpret_cast<char*>(fileData.data()), fileSize);
        file.close();

        assert(fileSize >= (std::streamsize)sizeof(Elf32Ehdr));
        const Elf32Ehdr* ehdr = reinterpret_cast<const Elf32Ehdr*>(fileData.data());

        assert(ehdr->e_ident[0] == 0x7F && ehdr->e_ident[1] == 'E' && ehdr->e_ident[2] == 'L' && ehdr->e_ident[3] == 'F');

        const unsigned char EI_CLASS = 4;
        assert(ehdr->e_ident[EI_CLASS] == 1);

        const unsigned char EI_DATA = 5;
        assert(ehdr->e_ident[EI_DATA] == 1);

        start_pc = ehdr->e_entry;

        uint32_t phoff = ehdr->e_phoff;
        uint16_t phentsize = ehdr->e_phentsize;
        uint16_t phnum = ehdr->e_phnum;
        const uint32_t PT_LOAD = 1;
        const uint32_t PF_X = 1;

        uint32_t text_vaddr = 0;
        const unsigned char* text_file_ptr = nullptr;
        uint32_t text_filesz = 0;

        assert(phoff + uint32_t(phnum) * uint32_t(std::max<uint16_t>(phentsize, sizeof(Elf32Phdr))) <= fileSize);

        for (uint16_t i = 0; i < phnum; ++i)
        {
            size_t offset = phoff + i * phentsize;
            assert(offset + sizeof(Elf32Phdr) <= fileData.size());
            const Elf32Phdr* ph = reinterpret_cast<const Elf32Phdr*>(fileData.data() + offset);

            uint32_t p_type   = ph->p_type;
            uint32_t p_offset = ph->p_offset;
            uint32_t p_vaddr  = ph->p_vaddr;
            uint32_t p_filesz = ph->p_filesz;
            uint32_t p_memsz  = ph->p_memsz;
            uint32_t p_flags  = ph->p_flags;

            if (p_type == PT_LOAD)
            {
                if (mem)
                {
                    if (p_filesz)
                    {
                        assert(uint64_t(p_offset) + p_filesz <= uint64_t(fileData.size()));
                        mem->write_bytes(p_vaddr, fileData.data() + p_offset, p_filesz);
                    }

                    if (p_memsz > p_filesz)
                    {
                        uint32_t zero_addr = p_vaddr + p_filesz;
                        uint32_t zero_len = p_memsz - p_filesz;
                        std::vector<uint8_t> zeros(zero_len, 0);
                        mem->write_bytes(zero_addr, zeros.data(), zeros.size());
                    }
                }
            }
        }

        uint32_t shoff = ehdr->e_shoff;
        uint16_t shentsize = ehdr->e_shentsize;
        uint16_t shnum = ehdr->e_shnum;
        uint16_t shstrndx = ehdr->e_shstrndx;
        if (shoff && shnum && shstrndx < shnum)
        {
            const Elf32Shdr* shTable = reinterpret_cast<const Elf32Shdr*>(fileData.data() + shoff);
            const Elf32Shdr* shstr = &shTable[shstrndx];
            const char* shstrtabData = reinterpret_cast<const char*>(fileData.data() + shstr->sh_offset);
            for (int i = 0; i < shnum; ++i)
            {
                const Elf32Shdr* sh = &shTable[i];
                const char* name = shstrtabData + sh->sh_name;
                if (std::strcmp(name, ".text") == 0)
                {
                    text_vaddr = sh->sh_addr;
                    text_filesz = sh->sh_size;
                    text_file_ptr = fileData.data() + sh->sh_offset;
                    break;
                }
            }
        }

        if (text_file_ptr && text_filesz >= 4)
        {
            mem->set_zero_pc(text_vaddr);
            mem->ensure_capacity(text_vaddr, text_filesz);

            for (uint32_t i = 0; i + 4 <= text_filesz; i += 4)
            {
                uint32_t instr = uint32_t(text_file_ptr[i])
                            | (uint32_t(text_file_ptr[i+1]) << 8)
                            | (uint32_t(text_file_ptr[i+2]) << 16)
                            | (uint32_t(text_file_ptr[i+3]) << 24);
                uint32_t addr = text_vaddr + i;
                raw_instrs_.emplace_back(instr, addr);
            }
        }
    }

    std::vector<std::shared_ptr<isa::Instruction>> parse_raw()
    {
        std::vector<std::shared_ptr<isa::Instruction>> instrs;
        instrs.reserve(raw_instrs_.size());

        int num_to_parse = 100;
        int i = 0;

        for (auto& raw_instr : raw_instrs_)
        {
            uint8_t opcode = raw_instr.code & 0x7F;
            auto it = isa::instructions_map.find(opcode);
            if (it == isa::instructions_map.end())
            {
                fprintf(stderr, "Unknown instruction. Opcode: 0x%08x; Address: 0x%08x\n",
                                opcode, raw_instr.address);
                assert(0);
            }
            auto instr_info = it->second;
            instrs.push_back(std::make_shared<isa::Instruction>(raw_instr, instr_info));

            i++;
            if (i >= num_to_parse)
                break;
        }

        return instrs;
    }

public:
    std::vector<std::shared_ptr<isa::Instruction>>
    decode(const std::string& filename, uint32_t& start_pc, std::shared_ptr<memory::Memory> mem)
    {
        parse_elf(filename, start_pc, mem);
        return parse_raw();
    }

private:
    std::vector<isa::Instruction::RawInstruction> raw_instrs_;

}; // class Decoder

} // namespace decoder