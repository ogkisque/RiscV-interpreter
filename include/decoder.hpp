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

    uint32_t readU32(const unsigned char* data)
    {
        return data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
    }

    uint16_t readU16(const unsigned char* data)
    {
        return data[0] | (data[1] << 8);
    }

    void parse_elf(const std::string& filename)
    {
        std::ifstream file(filename, std::ios::binary);
        assert(file.is_open());

        file.seekg(0, std::ios::end);
        std::streamsize fileSize = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<unsigned char> fileData(fileSize);
        file.read(reinterpret_cast<char*>(fileData.data()), fileSize);
        file.close();

        assert(fileSize >= sizeof(Elf32Ehdr) && 
            fileData[0] == 0x7F && fileData[1] == 'E' &&
            fileData[2] == 'L' && fileData[3] == 'F');

        const Elf32Ehdr* ehdr = (const Elf32Ehdr*)(fileData.data());
        
        auto check = readU16(ehdr->e_ident + 18);
        assert(check == 0xF3);

        const Elf32Shdr* shdrTable = (const Elf32Shdr*)(fileData.data() + readU32((const unsigned char*)&ehdr->e_shoff));
        const Elf32Shdr* shstrtab = &shdrTable[readU16((const unsigned char*)&ehdr->e_shstrndx)];
        const char* shstrtabData = (const char*)(fileData.data() + readU32((const unsigned char*)&shstrtab->sh_offset));

        const Elf32Shdr* textSection = nullptr;
        auto size = readU16((const unsigned char*)&ehdr->e_shnum);
        for (int i = 0; i < size; i++)
        {
            const Elf32Shdr* shdr = &shdrTable[i];
            const char* sectionName = shstrtabData + readU32((const unsigned char*)&shdr->sh_name);
            
            if (std::string(sectionName) == ".text")
            {
                textSection = shdr;
                break;
            }
        }
        assert(textSection);

        const unsigned char* textData = fileData.data() + readU32((const unsigned char*)&textSection->sh_offset);
        uint32_t textSize = readU32((const unsigned char*)&textSection->sh_size);
        uint32_t textAddr = readU32((const unsigned char*)&textSection->sh_addr);

        raw_instrs_.reserve(textSize / 4);
        for (uint32_t i = 0; i < textSize; i += 4)
        {
            if (i + 4 > textSize)
                break;

            uint32_t instruction = readU32(textData + i);
            uint32_t address = textAddr + i;
            raw_instrs_.emplace_back(instruction, address);
        }
    }

    void parse_raw()
    {
        instrs_.reserve(raw_instrs_.size());

        int num_to_parse = 1;
        int i = 0;

        for (auto& raw_instr : raw_instrs_)
        {
            uint8_t opcode = raw_instr.code & 0x7F;
            auto it = isa::instructions_map.find(opcode);
            assert(it != isa::instructions_map.end());
            auto instr_info = it->second;
            instrs_.emplace_back(raw_instr, instr_info);

            i++;
            if (i >= num_to_parse)
                break;
        }
    }

public:
    void decode(const std::string& filename)
    {
        parse_elf(filename);
        parse_raw();
    }

    void dump_raw_instrs() const
    {
        for (auto& instr : raw_instrs_)
        {
            std::cerr << std::hex << std::setw(8) << std::setfill('0') << instr.address << ": "
                      << std::hex << std::setw(8) << std::setfill('0') << instr.code << "    "
                      << std::bitset<32>(instr.code) << std::endl;
        }
    }

    void dump_instr() const
    {
        for (auto& instr : instrs_)
        {
            fprintf(stderr, "address: 0x%08x; code: 0x%08x; name: %s\n",
                            instr.get_address(), instr.get_raw_code(), instr.get_name().c_str());
        }
    }

private:
    std::vector<isa::Instruction::RawInstruction> raw_instrs_;
    std::vector<isa::Instruction> instrs_;

}; // class Decoder

} // namespace decoder