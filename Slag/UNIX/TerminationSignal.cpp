#include "../OS_dependent.h"

#include <thread>
#include <wordexp.h>

#include "aq/Clock.h"

static bool* run;
static double startTime;
static double timeout;
static aq::Clock timer;

void init_termination_signal(bool* do_run, double hardResetTimeout)
{
	startTime = timer.Tock();
	run = do_run;
	*run = true;
	
	timeout = hardResetTimeout;
}

void wait_termination_signal()
{
	while (*run)
	{
		if ( timeout > 0 && startTime + timeout <= timer.Tock())
			*run = false;
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

std::vector<std::string> split_to_argv(const std::string& line)
{
	wordexp_t wordexp_result;
	std::vector<std::string> argv;
	
	if (wordexp(line.c_str(), &wordexp_result, 0) == 0)
	{
		for (size_t i = 0; i < wordexp_result.we_wordc; ++i)
			argv.emplace_back(wordexp_result.we_wordv[i]);

		wordfree(&wordexp_result);
	}
	return argv;
}
