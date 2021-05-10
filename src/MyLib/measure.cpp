#include <cstdint>
#include <fstream>
#include <thread>
#include <chrono>

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

double cpu_avg_baseline_mW(std::chrono::milliseconds ms)
{
	uint64_t before = cpu_uJ();
	std::this_thread::sleep_for(ms);
	uint64_t after = cpu_uJ();
	return (after-before)/static_cast<double>(ms.count());
}


