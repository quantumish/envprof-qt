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
#include <optional>
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

const Func* Profiler::attempt_update(std::vector<Func*>& funcs, const std::string& name, uint64_t energy)
{
    for (Func* f : funcs) {
		if (f->name.compare(name) == 0) {
			f->energy += energy;
			f->duration = samples - f->start;
			return f;
		}
    }
    return nullptr;
}

void Profiler::capture_and_freeze()
{
	prev_count = curr_count;
	curr_count = cpu_uJ();	
	ptrace(PTRACE_ATTACH, pid, 0, 0);
	void* ui = _UPT_create(pid);
	if (!ui) throw std::runtime_error("_UPT_create() failed.");
	struct timespec t = { .tv_sec = 0, t.tv_nsec = 1000 };
	nanosleep(&t, NULL);
	unw_cursor_t c;
	unw_addr_space_t as = unw_create_addr_space(&_UPT_accessors, 0);
	int rc = unw_init_remote(&c, as, ui);
	if (rc != 0) {
        if (rc == UNW_EINVAL) {
            throw std::runtime_error("Failed to initialize cursor for remote unwinding: UNW_EINVAL.");
        } else if (rc == UNW_EUNSPEC) {
			throw std::runtime_error("Failed to initialize cursor for remote unwinding: UNW_EUNSPEC.");
        } else if (rc == UNW_EBADREG) {
			throw std::runtime_error("Failed to initialize cursor for remote unwinding: UNW_EBADREG.");
        } else {
			std::cout << rc << "\n";
			throw std::runtime_error("Failed to initialize cursor for remote unwinding;");
        }
    }
    int a = UNW_EUNSPEC;
	std::vector<std::string> names;
	do {
	    unw_word_t offset, pc;
	    char fname[64];
	    unw_get_proc_name(&c, fname, sizeof(fname), &offset);
	    names.insert(names.begin(), std::string(fname));
	} while (unw_step(&c) > 0);	
	std::vector<Func*>* level = &funcs;
	for (std::string name : names) {
	    const Func* existing = attempt_update(*level, name, curr_count-prev_count);
	    if (existing != nullptr) level = const_cast<std::vector<Func*>*>(&existing->callees);
	    else {
			level->push_back(new Func(name, samples, curr_count - prev_count));
			level = &level->back()->callees;
		}
	}	
	_UPT_destroy(ui);
	ptrace(PTRACE_DETACH, pid, 0, 0);
	ptrace(PTRACE_CONT, pid);
		
	// kill(pid, SIGCONT); // HACK I shouldn't have to do this for it to work...
}

void Profiler::dump(std::vector<Func*>& level, int indent)
{
    for (Func* f : level) {
		for (int i = 0; i < indent; i++) std::cout << " ";
		std::cout << f->name << " - " << f->energy << "\n";
		dump(f->callees, indent+1);
    }
}

void Profiler::start()
{
    curr_count = cpu_uJ();
    std::cout << curr_count << '\n';
    // TODO Add exit condition
	for (samples = 0; samples < 1000; samples++) {
		capture_and_freeze();
		usleep(interval.count()*1000);		
	}
}
