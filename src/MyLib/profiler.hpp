#include <unistd.h>
#include <cstdint>
#include <string>
#include <chrono>

class FlameGraph {};

class Profiler
{
	const std::chrono::milliseconds interval;
	uint64_t prev_count;
	uint64_t curr_count;
	
	pid_t pid;
	
	struct CallStack {};	
	std::string raw_stack;
	
	void capture_and_freeze();
	uint64_t sample_uJ();
	CallStack parse_raw_stack();
	void update_flamegraph();
	void resume();
public:
	FlameGraph graph;
	Profiler(pid_t pid);
	Profiler(std::string cmd);
	void start();
	void report();
};
