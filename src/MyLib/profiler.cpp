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
    void* ui = _UPT_create(pid);
	if (!ui) throw std::runtime_error("_UPT_create() failed.");
	ptrace(PTRACE_ATTACH, pid);
	unw_cursor_t c;
	unw_addr_space_t as = unw_create_addr_space(&_UPT_accessors, 0);
	int rc = unw_init_remote(&c, as, ui);
	if (rc != 0) throw std::runtime_error("Failed to initialize cursor for remote unwinding.");
	do {
		unw_word_t offset, pc;
		char fname[64];
		(void) unw_get_proc_name(&c, fname, sizeof(fname), &offset);
		printf("\n%p : (%s+0x%x) [%p]", (void*)pc, fname, (int)offset, (void*)pc);
	} while (unw_step(&c) > 0);
	ptrace(PTRACE_DETACH, pid);
	_UPT_destroy(ui);
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
