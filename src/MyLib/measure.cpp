#include <cstdint>
#include <fstream>

uint64_t cpu_uJ()
{
	std::ifstream istrm("/sys/class/powercap/intel-rapl/intel-rapl:0/energy_uj", std::ios::binary);
	if (!istrm.is_open()) {
		throw std::runtime_error("Failed to open powercap interface - does your CPU support RAPL? Are you running with sudo?");		
	}
	uint64_t energy;
	istrm >> energy;
	return energy;
}

uint64_t cpu_baseline_uJ()
{	
}

