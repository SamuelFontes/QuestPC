#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <elf.h>
#include <sys/mman.h>
#include <unistd.h>

const char *GAME_EXECUTABLE_RELATIVE_PATH = "/home/sfontes/Documents/game/extracted/lib/arm64-v8a/libUE4.so";

size_t align_down(size_t addr, size_t align) {
    return addr & ~(align - 1);
}

size_t align_up(size_t addr, size_t align) {
    return (addr + align - 1) & ~(align - 1);
}

int main() {
    std::ifstream elfFile(GAME_EXECUTABLE_RELATIVE_PATH, std::ios::binary);
    if (!elfFile) {
        std::cerr << "Failed to open ELF file." << std::endl;
        return 1;
    }

    Elf64_Ehdr elfHeader;
    elfFile.read(reinterpret_cast<char*>(&elfHeader), sizeof(elfHeader));

    if (memcmp(elfHeader.e_ident, ELFMAG, SELFMAG) != 0) {
        std::cerr << "Not a valid ELF file." << std::endl;
        return 1;
    }

    if (elfHeader.e_machine != EM_AARCH64) {
        std::cerr << "Not an ARM64 ELF binary." << std::endl;
        return 1;
    }

    std::cout << "Valid ARM64 ELF detected." << std::endl;

    // Read Program Headers
    elfFile.seekg(elfHeader.e_phoff, std::ios::beg);
    std::vector<Elf64_Phdr> programHeaders(elfHeader.e_phnum);

    for (uint16_t i = 0; i < elfHeader.e_phnum; ++i) {
        elfFile.read(reinterpret_cast<char*>(&programHeaders[i]), sizeof(Elf64_Phdr));
    }

    size_t pageSize = sysconf(_SC_PAGESIZE);

    // Iterate LOAD segments and map them into memory
    for (uint16_t i = 0; i < elfHeader.e_phnum; ++i) {
        const Elf64_Phdr& ph = programHeaders[i];
        if (ph.p_type != PT_LOAD) continue;

        size_t seg_offset = align_down(ph.p_offset, pageSize);
        size_t seg_vaddr  = align_down(ph.p_vaddr, pageSize);
        size_t seg_filesz = ph.p_filesz + (ph.p_offset - seg_offset);
        size_t seg_memsz  = ph.p_memsz  + (ph.p_vaddr  - seg_vaddr);
        size_t seg_size   = align_up(seg_memsz, pageSize);

        int prot = 0;
        if (ph.p_flags & PF_R) prot |= PROT_READ;
        if (ph.p_flags & PF_W) prot |= PROT_WRITE;
        if (ph.p_flags & PF_X) prot |= PROT_EXEC;

        void* seg_addr = mmap((void*)seg_vaddr, seg_size, prot, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (seg_addr == MAP_FAILED) {
            std::cerr << "Failed to mmap segment." << std::endl;
            return 1;
        }

        // Load segment data from file
        elfFile.seekg(seg_offset, std::ios::beg);
        elfFile.read(reinterpret_cast<char*>(seg_addr), seg_filesz);

        std::cout << "Mapped segment at virtual 0x" << std::hex << (uintptr_t)seg_addr
                  << " with size 0x" << seg_size 
                  << " (prot: " << ((prot & PROT_READ) ? "R" : "")
                                << ((prot & PROT_WRITE) ? "W" : "")
                                << ((prot & PROT_EXEC) ? "X" : "")
                  << ")" << std::endl;
    }

    elfFile.close();
    std::cout << "All LOAD segments mapped." << std::endl;

    return 0;
}
