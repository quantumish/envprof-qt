#include <stdexcept>
#include <fstream>
#include <streambuf>
#include <csignal>
#include <unistd.h>
#include "profiler.hpp"
#include "measure.hpp"

Profiler::Profiler(pid_t p)
	:pid{p}, interval{1}
{
}

void Profiler::capture_and_freeze()
{
	std::string spid = std::to_string(pid);
	// TODO Iterate over *all* threads.
	std::ifstream istrm("/proc/"+spid+"/task/"+spid+"/stack");
	if (!istrm.is_open()) {
		throw std::runtime_error("Failed to view call stack - are you running with sudo? Is this the right PID?");		
	}
	// NOTE Look into details of how this works
	raw_stack = std::string((std::istreambuf_iterator<char>(istrm)), std::istreambuf_iterator<char>());
	prev_count = curr_count;
	curr_count = cpu_uJ();
	kill(pid, SIGSTOP);
}

uint64_t Profiler::sample_uJ()
{
	return curr_count - prev_count;
}

void Profiler::resume()
{
	kill(pid, SIGCONT);
}

void Profiler::start()
{
	prev_count = cpu_uJ();
	// TODO Add exit condition
	while (true) {
		capture_and_freeze();
		parse_raw_stack();
		update_flamegraph();
		resume();
		usleep(interval.count());
	}
}
