#include <stdexcept>
#include <stdio.h>
#include <string.h>
#include <elf.h>
#include <cxxabi.h>

#define MAX_SYMLEN 256

void get_symbols(std::string path)
{
	FILE* fptr = fopen(path.c_str(), "r");
	Elf64_Ehdr header = {0};
	fread(&header, sizeof(Elf64_Ehdr), 1, fptr);
	if (strncmp(reinterpret_cast<char*>(&header), ELFMAG, 3) != 0) {
		throw std::runtime_error("Attempted read of invalid ELF file.");
	}
    fseek(fptr, header.e_shoff, SEEK_SET);
	printf("Seeked to section header table.\n");
	Elf64_Shdr symtab[2] = {0};	
	while (symtab[0].sh_type != SHT_DYNSYM || symtab[1].sh_type != SHT_STRTAB) {
		fread(&symtab, sizeof(Elf64_Shdr), 2, fptr);
	}
	fseek(fptr, symtab[1].sh_offset, SEEK_SET);
	char* strtbl = new char[symtab[1].sh_size];
	fread(strtbl, symtab[1].sh_size, 1, fptr);
	fseek(fptr, symtab[0].sh_offset, SEEK_SET);
	printf("Seeked to symbol table.\n");
	Elf64_Sym symbol = {0};
    for (size_t i = 0; i < symtab[0].sh_size; i+=sizeof(Elf64_Sym)) {
		fread(&symbol, sizeof(Elf64_Sym), 1, fptr);
		int status;
		char* mangled = reinterpret_cast<char*>(&strtbl[symbol.st_name]);
		char demangled[MAX_SYMLEN];
		size_t max = MAX_SYMLEN;
		abi::__cxa_demangle(mangled, demangled, &max, &status);
		printf("%s\n", demangled);
	}
}

int main()
{
	get_symbols("/home/quantumish/projects/fold/fold");
}
