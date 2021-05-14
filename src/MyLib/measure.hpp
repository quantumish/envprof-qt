#include <cstdint>
#include <functional>
#include <chrono>

uint64_t cpu_uJ();
double cpu_avg_baseline_mW(std::chrono::milliseconds ms);
uint32_t gpu_direct_mW();
uint64_t gpu_uJ();
double collect_baseline_mW(std::function<uint64_t(void)> collector, std::chrono::milliseconds ms);
