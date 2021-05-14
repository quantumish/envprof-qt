#include <cstdint>
#include <fstream>
#include <thread>
#include <chrono>
#include <iostream>
#include <nvml.h>

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

uint32_t gpu_direct_mW()
{
	nvmlInit_v2();
	nvmlDevice_t dev;
	nvmlDeviceGetHandleByIndex(0, &dev);
	uint32_t power;
	nvmlReturn_t ret = nvmlDeviceGetPowerUsage(dev, &power);
	return power;   
}

uint64_t gpu_uJ()
{
	nvmlInit_v2();
	nvmlDevice_t dev;
	nvmlDeviceGetHandleByIndex(0, &dev);
	uint64_t energy;
	nvmlReturn_t ret = nvmlDeviceGetTotalEnergyConsumption(dev, reinterpret_cast<unsigned long long int*>(&energy));
	return energy*1000;
}
