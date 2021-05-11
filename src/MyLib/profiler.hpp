#include <unistd.h>
#include <cstdint>
#include <string>
#include <chrono>
#include <vector>

struct Func {
    std::string name;
    const uint64_t start;
    uint64_t energy;
    uint64_t duration;
    std::vector<Func> callees;
    Func(std::string id, uint64_t now, uint64_t init_energy);    
};

class Profiler
{
    uint64_t samples;
    const std::chrono::milliseconds interval;
    uint64_t prev_count;
    uint64_t curr_count;
    pid_t pid;
	uint64_t baseline;
   
    uint64_t sample_uJ();    
    void resume();
public:
    Profiler(pid_t pid);
    Profiler(std::string cmd);
	std::vector<Func> funcs;
    void start();
    void report();
	void capture_and_freeze();
	void dump(std::vector<Func>& level, int indent = 0);
};

