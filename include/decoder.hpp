#pragma once

#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <string>
#include <cstdint>
#include <bitset>
#include <cassert>

namespace decoder
{

class ElfParser
{
private:
    typedef uint32_t Elf32_Addr;
    typedef uint16_t Elf32_Half;
    typedef uint32_t Elf32_Off;
    typedef uint32_t Elf32_Word;

    struct Elf32_Ehdr
    {
        unsigned char   e_ident[16];
        Elf32_Half      e_type;
        Elf32_Half      e_machine;
        Elf32_Word      e_version;
        Elf32_Addr      e_entry;
        Elf32_Off       e_phoff;
        Elf32_Off       e_shoff;
        Elf32_Word      e_flags;
        Elf32_Half      e_ehsize;
        Elf32_Half      e_phentsize;
        Elf32_Half      e_phnum;
        Elf32_Half      e_shentsize;
        Elf32_Half      e_shnum;
        Elf32_Half      e_shstrndx;
    };

    struct Elf32_Shdr
    {
        Elf32_Word sh_name;
        Elf32_Word sh_type;
        Elf32_Word sh_flags;
        Elf32_Addr sh_addr;
        Elf32_Off  sh_offset;
        Elf32_Word sh_size;
        Elf32_Word sh_link;
        Elf32_Word sh_info;
        Elf32_Word sh_addralign;
        Elf32_Word sh_entsize;
    };

    uint32_t readU32(const unsigned char* data)
    {
        return data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
    }

    uint16_t readU16(const unsigned char* data) {
        return data[0] | (data[1] << 8);
    }

public:
    struct RawInstruction
    {
        uint32_t code;
        uint32_t address;
    };

    std::vector<RawInstruction> get_instructions(const std::string& filename)
    {
        std::ifstream file(filename, std::ios::binary);
        assert(file.is_open());

        file.seekg(0, std::ios::end);
        std::streamsize fileSize = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<unsigned char> fileData(fileSize);
        file.read(reinterpret_cast<char*>(fileData.data()), fileSize);
        file.close();

        assert(fileSize >= sizeof(Elf32_Ehdr) && 
            fileData[0] == 0x7F && fileData[1] == 'E' &&
            fileData[2] == 'L' && fileData[3] == 'F');

        const Elf32_Ehdr* ehdr = (const Elf32_Ehdr*)(fileData.data());
        
        auto check = readU16(ehdr->e_ident + 18);
        assert(check == 0xF3);

        const Elf32_Shdr* shdrTable = (const Elf32_Shdr*)(fileData.data() + readU32((const unsigned char*)&ehdr->e_shoff));
        const Elf32_Shdr* shstrtab = &shdrTable[readU16((const unsigned char*)&ehdr->e_shstrndx)];
        const char* shstrtabData = (const char*)(fileData.data() + readU32((const unsigned char*)&shstrtab->sh_offset));

        const Elf32_Shdr* textSection = nullptr;
        auto size = readU16((const unsigned char*)&ehdr->e_shnum);
        for (int i = 0; i < size; i++)
        {
            const Elf32_Shdr* shdr = &shdrTable[i];
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

        std::vector<RawInstruction> instrs;
        for (uint32_t i = 0; i < textSize; i += 4)
        {
            if (i + 4 > textSize)
                break;

            uint32_t instruction = readU32(textData + i);
            uint32_t address = textAddr + i;
            instrs.emplace_back(instruction, address);
        }
        return instrs;
    }

    void dump_instrs(const std::vector<RawInstruction>& instrs)
    {
        for (auto& instr : instrs)
        {
            std::cerr << std::hex << std::setw(8) << std::setfill('0') << instr.address << ": "
                    << std::hex << std::setw(8) << std::setfill('0') << instr.code << "    "
                    << std::bitset<32>(instr.code) << std::endl;
        }
    }
}; // class ElfParser

} // namespace decoder