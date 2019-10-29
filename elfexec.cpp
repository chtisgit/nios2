#include "elfexec.h"

#include <cstdint>
#include <string>
#include <fstream>


#include "nios2.h"

struct ElfHeader {
	uint8_t e_ident[16];
	uint16_t e_type, e_machine;
	uint32_t e_version;
	uint32_t e_entry, e_phoff, e_shoff, e_flags;
	uint16_t e_ehsize;
	uint16_t e_phentsize, e_phnum;
	uint16_t e_shentsize, e_shnum;
	uint16_t e_shstrndx;
};

struct ProgHeader {
	uint32_t p_type;
	uint32_t p_offset;
	uint32_t p_vaddr;
	uint32_t p_paddr;
	uint32_t p_filesz;
	uint32_t p_memsz;
	uint32_t p_flags;
	uint32_t p_align;
};

struct LoaderError {};

template<typename T>
static void get(std::ifstream& file, T& out) {
    T u{};
    char b[8];
    file.read(b, sizeof(T));
    for(int i = 0; i != sizeof(T); i++) {
        u = (u << 8) | b[i];
    }

    out = u;
}

static std::vector<ProgHeader> getPHs(std::ifstream& file, const ElfHeader& ehdr) {
    std::vector<ProgHeader> phs(ehdr.e_phnum);

    file.seekg(ehdr.e_phoff, std::ios_base::beg);
    for(int i = 0; i != ehdr.e_phnum; i++) {
        ProgHeader &ph = phs[i];
        get(file, ph.p_type);
        get(file, ph.p_offset);
        get(file, ph.p_vaddr);
        get(file, ph.p_paddr);
        get(file, ph.p_filesz);
        get(file, ph.p_memsz);
        get(file, ph.p_flags);
        get(file, ph.p_align);
    }

    return phs;
}

ElfHeader loadElf(const std::string &path, uint8_t *base, uint8_t *end)
{
    std::ifstream file(path, std::ios_base::binary);

    ElfHeader ehdr;

    for(int i = 0; i != 16; i++){
        get(file, ehdr.e_ident[i]);
    }

    get(file, ehdr.e_type);
    get(file, ehdr.e_machine);
    get(file, ehdr.e_version);
    get(file, ehdr.e_entry);
    get(file, ehdr.e_phoff);
    get(file, ehdr.e_shoff);
    get(file, ehdr.e_flags);
    get(file, ehdr.e_ehsize);
    get(file, ehdr.e_phentsize);
    get(file, ehdr.e_phnum);
    get(file, ehdr.e_shentsize);
    get(file, ehdr.e_shnum);
    get(file, ehdr.e_shstrndx);

    if(ehdr.e_phoff == 0)
        throw LoaderError{};

    auto phs = getPHs(file, ehdr);

    printf("ELF: %zu program headers found (offset = %X)\n", phs.size(), ehdr.e_phoff);
    for(auto ph : phs) {
        printf("ELF: header type %d vaddr: %X\n", ph.p_type, ph.p_vaddr);
        if(ph.p_type == 1){
            for(int i = 0; i != ph.p_filesz; i++) {
                file.seekg(ph.p_offset, std::ios_base::beg);
                printf("ELF: load %d bytes from offset %X to address %X\n", ph.p_filesz, ph.p_offset, ph.p_vaddr);
                base[ph.p_vaddr + i] = file.peek();
            }
        }
    }

    return ehdr;
}

void runElf(const std::string &path)
{
	nios2::CPU cpu;
	cpu.setMemSize(512 * 1024);

	auto ehdr = loadElf(path, cpu.Memory.data(), cpu.Memory.data() + cpu.Memory.size());
    
    printf("ELF: entry %X\n", ehdr.e_entry);
    cpu.run(ehdr.e_entry);
}
