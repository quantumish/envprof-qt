#include <unistd.h>
#include <cstdint>
#include <string>
#include <chrono>
#include <vector>
#include <optional>
#include <fstream>

struct Func {
    std::string name;
    const uint64_t start;
    uint64_t energy;
    uint64_t duration;
    std::vector<Func*> callees;
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
	std::ofstream log;
  
    const Func* attempt_update(std::vector<Func*>& funcs, const std::string& name, uint64_t energy);
public:
	uint64_t total;
    Profiler(pid_t pid, std::string logs = "./latest.log");
    Profiler(std::string cmd);
	~Profiler();
	std::vector<Func*> funcs;
    void start();
	void capture_and_freeze();
	void dump(std::vector<Func*>& level, int indent = 0);
	const std::vector<Func*> expensive_funcs(std::vector<std::string> exclude_namespaces = {});
};

