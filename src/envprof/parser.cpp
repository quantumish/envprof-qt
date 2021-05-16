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
	std::vector<std::array<Elf64_Shdr,2>> symtabs;
	Elf64_Shdr syment[2] = {0};
    for (int i = 0; i < header.e_shnum-1; i++) {
		fread(&syment, sizeof(Elf64_Shdr), 2, fptr); // NOTE: questionable
		if (syment[0].sh_type == SHT_SYMTAB && syment[1].sh_type == SHT_STRTAB) {
			symtabs.push_back({syment[0], syment[1]});
		}
		fseek(fptr, -sizeof(Elf64_Shdr), SEEK_CUR); // HACK 
	}
	for (std::array<Elf64_Shdr,2> symtab : symtabs) {
		fseek(fptr, symtab[1].sh_offset, SEEK_SET);
		char* strtbl = new char[symtab[1].sh_size];
		fread(strtbl, symtab[1].sh_size, 1, fptr);
		fseek(fptr, symtab[0].sh_offset, SEEK_SET);
		FILE* out = fopen("./symbols.txt", "w");
		Elf64_Sym symbol = {0};
		for (size_t i = 0; i < symtab[0].sh_size; i+=sizeof(Elf64_Sym)) {
			fread(&symbol, sizeof(Elf64_Sym), 1, fptr);
			if (symbol.st_name != 0) {
				char* mangled = reinterpret_cast<char*>(&strtbl[symbol.st_name]);				
				int status;
				size_t sz;
				char* demangled = abi::__cxa_demangle(mangled, NULL, &sz, &status);
				if (status == 0) fprintf(out, "%s\n", demangled);
			}
		}
		fclose(out);
	}    
	fclose(fptr);
}

// int main()
// {
// 	get_symbols("/home/quantumish/projects/fold/fold");
// }
