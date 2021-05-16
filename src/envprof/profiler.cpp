#include <stdexcept>
#include <fstream>
#include <streambuf>
#include <csignal>
#include <unistd.h>
#include <sys/ptrace.h>
#include "profiler.hpp"
#include "measure.hpp"
#include <iostream>
#include <time.h>
#include <wait.h>
#include <stdio.h>
#include <optional>
#include <libunwind.h>
#include <libunwind-x86.h>
#include <libunwind-ptrace.h>
#include <stack>
#include <string.h>
#include <cxxabi.h>

#include "parser.hpp"

#define MAXLINE 1024
#define MAX_SYMLEN 256

extern char** environ;

static const bool str_in_file(std::string path, std::string target)
{    
	FILE* fptr = fopen(path.c_str(), "r");
	char line[MAXLINE];
	while (fgets(line, MAXLINE, fptr)) {		
		line[strlen(line)-1] = '\0';
		if (strcmp(target.c_str(), line) == 0) {
			fclose(fptr);
			return true;
		}
	}
	fclose(fptr);
	return false;
}

Func::Func(std::string id, uint64_t now, uint64_t init_energy)
    :name(id), start(now), duration(0), energy(init_energy)
{
}

Profiler::Profiler(pid_t p, std::string log_path)
	:pid{p}, interval{1}, log{}
{
	log.open(log_path);
}

Profiler::Profiler(std::string cmd)
	:interval{1}
{
	FILE* p = popen((cmd + " & echo $!" ).c_str(), "r");	
	fscanf(p, "%d", &pid);    
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
	ptrace(PTRACE_ATTACH, pid);
	void* ui = _UPT_create(pid);
	if (!ui) throw std::runtime_error("_UPT_create() failed.");
	unw_cursor_t c;
	unw_addr_space_t as = unw_create_addr_space(&_UPT_accessors, 0);
	int rc = unw_init_remote(&c, as, ui);
	std::vector<std::string> names;
	do {
	    unw_word_t offset, pc;
	    char fname[MAX_SYMLEN];
	    unw_get_proc_name(&c, fname, sizeof(fname), &offset);
		int status;
		size_t sz;
		char* demangled = abi::__cxa_demangle(fname, NULL, &sz, &status);
		if (status == 0) names.insert(names.begin(), std::string(demangled));
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
	_UPT_resume(as, &c, ui);
	_UPT_destroy(ui);
	kill(pid, SIGCONT); // HACK I shouldn't have to do this for it to work...
}

void Profiler::dump(std::vector<Func*>& level, int indent)
{
    for (Func* f : level) {
		for (int i = 0; i < indent; i++) std::cout << " ";
		std::cout << f->name << " - " << f->energy << " uJ\n";
		dump(f->callees, indent+1);
    }
}

static const bool valid_namespace(std::string symbol, std::vector<std::string> exclude)
{
	for (std::string namespc : exclude) {	
		std::string funcname = namespc.substr(namespc.find(" ")+1);		
		if (namespc.compare(symbol) == 0 || funcname.compare(symbol) == 0) {
			return false;
			break;
		}
	}
	return true;
}

const std::vector<Func*> Profiler::expensive_funcs(std::vector<std::string> exclude_namespaces)
{
	std::stack<std::pair<std::vector<Func*>, uint64_t>> stk;
	if (funcs[0]->name.compare("_start") == 0) { // HACK
		stk.push({funcs[0]->callees, funcs[0]->energy});
	}
	else stk.push({funcs, total});
	std::vector<Func*> expensive;
	bool not_entered = true;
	while (stk.size() > 0) {
		std::vector<Func*> level = stk.top().first;
		uint64_t energy = stk.top().second;
		stk.pop();
		for (Func* f : level) {
			if (f->energy/static_cast<float>(energy) < 0.4) continue;
			if (str_in_file("./symbols.txt", f->name)) {
				not_entered = false;
				if (valid_namespace(f->name, exclude_namespaces)) expensive.push_back(f);
				stk.push({f->callees, f->energy});
			}
			else if (not_entered) stk.push({f->callees, f->energy});
		}
	}
	return expensive;
}

void Profiler::start()
{
    curr_count = cpu_uJ();
	uint64_t start_count = curr_count;
    // TODO Add exit condition
	for (samples = 0; samples < 1000; samples++) {
		capture_and_freeze();
		usleep(interval.count()*1000);		
	}
	uint64_t end_count = cpu_uJ();
	total=funcs[0]->energy;
}

Profiler::~Profiler()
{
	log.close();
}
