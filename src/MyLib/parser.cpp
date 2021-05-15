#include <stdexcept>
#include <vector>
#include <stdio.h>
#include <string.h>
#include <elf.h>
#include <cxxabi.h>
#include <array>

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
	std::vector<std::array<Elf64_Shdr,2>> symtabs;
	Elf64_Shdr syment[2] = {0};
    for (int i = 0; i < header.e_shnum-1; i++) {
		fread(&syment, sizeof(Elf64_Shdr), 2, fptr);
		// printf("%d %d\n", syment[0].sh_type, syment[1].sh_type);
		if (syment[0].sh_type == SHT_SYMTAB && syment[1].sh_type == SHT_STRTAB) {
			symtabs.push_back({syment[0], syment[1]});
		}
		fseek(fptr, -sizeof(Elf64_Shdr), SEEK_CUR);
	}
	for (std::array<Elf64_Shdr,2> symtab : symtabs) {
		fseek(fptr, symtab[1].sh_offset, SEEK_SET);
		char* strtbl = new char[symtab[1].sh_size];
		fread(strtbl, symtab[1].sh_size, 1, fptr);
		fseek(fptr, symtab[0].sh_offset, SEEK_SET);
		printf("Seeked to symbol table.\n");
		Elf64_Sym symbol = {0};
		for (size_t i = 0; i < symtab[0].sh_size; i+=sizeof(Elf64_Sym)) {
			fread(&symbol, sizeof(Elf64_Sym), 1, fptr);
			int status;
			if (symbol.st_name != 0) {
				char* mangled = reinterpret_cast<char*>(&strtbl[symbol.st_name]);
				char* demangled = (char*)malloc(sizeof(char)*MAX_SYMLEN);
				size_t max = MAX_SYMLEN;
				abi::__cxa_demangle(mangled, demangled, &max, &status);
				printf("%zu\n",max);
			    demangled[max] = '\0';
				printf("%s\n", demangled);
			}
		}
	}
}

int main()
{
	get_symbols("/home/quantumish/projects/fold/fold");
}
