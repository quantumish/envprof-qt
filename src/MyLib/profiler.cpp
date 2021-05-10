#include <stdexcept>
#include <fstream>
#include <streambuf>
#include <csignal>
#include <unistd.h>
#include <sys/ptrace.h>
#include "profiler.hpp"
#include "measure.hpp"
#include <iostream>
#include <algorithm>

Func::Func(std::string id, uint64_t now, uint64_t init_energy)
    :name(id), start(now), duration(0), energy(init_energy)
{
}

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
    prev_count = curr_count;
    curr_count = cpu_uJ();
    kill(pid, SIGSTOP);
    std::vector<std::string> raw_stack;
    std::string line;
    while (std::getline(istrm, line)) {
	lines_in_reverse.insert(lines_in_reverse.begin(), line);
    }
    std::vector<Func>& level = funcs;
    for (std::string call : raw_stack) {
        for (Func f : level) {
	  if (f.name == call) {
	    f.duration = samples - f.start;
	    f.energy += curr_count - prev_count;
	    level = level.callees;
	    continue;
	  }
	}
	level.emplace_back(call, samples, curr_count - prev_count);
	level = level.callees;
    }
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
    //    while (true) {
	capture_and_freeze();
	//	parse_raw_stack();
	//	resume();
	//	usleep(interval.count());
	//    }
}
