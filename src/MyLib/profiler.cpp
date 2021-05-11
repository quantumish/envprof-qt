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
#include <time.h>
#include <wait.h>
#include <libunwind.h>
#include <libunwind-x86.h>
#include <libunwind-ptrace.h>

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
	prev_count = curr_count;
	curr_count = cpu_uJ();	
	ptrace(PTRACE_ATTACH, pid);
	void* ui = _UPT_create(pid);
	if (!ui) throw std::runtime_error("_UPT_create() failed.");
	struct timespec t = { .tv_sec = 0, t.tv_nsec = 1000 };
    nanosleep(&t, NULL);
	unw_cursor_t c;
	unw_addr_space_t as = unw_create_addr_space(&_UPT_accessors, 0);
	int rc = unw_init_remote(&c, as, ui);
	if (rc != 0) throw std::runtime_error("Failed to initialize cursor for remote unwinding.");
	std::vector<Func>* level = &funcs;
	do {
		unw_word_t offset, pc;
		char fname[64];
		unw_get_proc_name(&c, fname, sizeof(fname), &offset);
		bool TEMP_DO_NOT_LET_ME_STAY_IN_CODE = false;
		for (Func f : *level) {
			if (f.name == fname) {
				f.energy += curr_count-prev_count;
				f.duration = samples-f.start;
				level = &f.callees;
				TEMP_DO_NOT_LET_ME_STAY_IN_CODE = true;
				break;
			}
		}
		if (!TEMP_DO_NOT_LET_ME_STAY_IN_CODE) level->emplace_back(std::string(fname), samples, curr_count-prev_count);			
		// printf("\n%p : (%s+0x%x) [%p]", (void*)pc, fname, (int)offset, (void*)pc);
	} while (unw_step(&c) > 0);
	_UPT_destroy(ui);
	ptrace(PTRACE_DETACH, pid);
	kill(pid, SIGCONT); // HACK I shouldn't have to do this for it to work...
}

void Profiler::dump(std::vector<Func>& level, int indent)
{
	for (Func f : level) {
		for (int i = 0; i < indent; i++) std::cout << '\t';
		std::cout << f.name << " - " << f.energy << "\n";
		dump(f.callees, indent+1);
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
    curr_count = cpu_uJ();
    // TODO Add exit condition
    //    while (true) {
    capture_and_freeze();
	//	parse_raw_stack();
	//	resume();
	//	usleep(interval.count());
	//    }
}
