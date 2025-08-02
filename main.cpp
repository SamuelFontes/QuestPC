#include <iostream>
#include <fstream>
#include <cstdint>
#include <vector>
#include <cstring>
#include <elf.h>  // Linux standard ELF types

const char *GAME_EXECUTABLE_RELATIVE_PATH = "/home/sfontes/Documents/game/extracted/lib/arm64-v8a/libUE4.so";

int main() {
    std::ifstream elfFile(GAME_EXECUTABLE_RELATIVE_PATH, std::ios::binary);
    if (!elfFile) {
        std::cerr << "Failed to open ELF file." << std::endl;
        return 1;
    }

    // Read ELF header
    Elf64_Ehdr elfHeader;
    elfFile.read(reinterpret_cast<char*>(&elfHeader), sizeof(elfHeader));

    // Verify ELF Magic
    if (memcmp(elfHeader.e_ident, ELFMAG, SELFMAG) != 0) {
        std::cerr << "Not a valid ELF file." << std::endl;
        return 1;
    }

    std::cout << "Valid ELF file detected." << std::endl;

    // Verify it's for ARM64
    if (elfHeader.e_machine != EM_AARCH64) {
        std::cerr << "Not an ARM64 ELF binary." << std::endl;
        return 1;
    }

    std::cout << "ELF is ARM64 architecture." << std::endl;

    // Read Program Headers (Segments)
    elfFile.seekg(elfHeader.e_phoff, std::ios::beg);
    std::vector<Elf64_Phdr> programHeaders(elfHeader.e_phnum);

    for (uint16_t i = 0; i < elfHeader.e_phnum; ++i) {
        elfFile.read(reinterpret_cast<char*>(&programHeaders[i]), sizeof(Elf64_Phdr));
    }

    std::cout << "Program Headers:" << std::endl;
    for (uint16_t i = 0; i < elfHeader.e_phnum; ++i) {
        const Elf64_Phdr& ph = programHeaders[i];
        if (ph.p_type == PT_LOAD) {
            std::cout << "  Segment " << i 
                      << " LOAD at offset 0x" << std::hex << ph.p_offset 
                      << " to virtual address 0x" << ph.p_vaddr
                      << ", size: 0x" << ph.p_memsz << std::endl;
        }
    }

    elfFile.close();
    return 0;
}
