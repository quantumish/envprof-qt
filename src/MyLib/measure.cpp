#include <cstdint>
#include <fstream>
#include <thread>
#include <chrono>
#include <iostream>
#include <functional>
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

double collect_baseline_mW(std::function<uint64_t(void)> collector, std::chrono::milliseconds ms)
{
	uint64_t before = collector();
	std::this_thread::sleep_for(ms);
	uint64_t after = collector();
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
