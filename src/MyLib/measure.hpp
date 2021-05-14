#include <cstdint>
#include <chrono>

uint64_t cpu_uJ();
double cpu_avg_baseline_mW(std::chrono::milliseconds ms);
uint32_t gpu_direct_mW();
uint64_t gpu_uJ();

