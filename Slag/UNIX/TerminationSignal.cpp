#include "../OS_dependent.h"

#include <signal.h>
#include <thread>

#include "aq/Clock.h"

static bool* run;
static double startTime;
static double timeout;
static aq::Clock timer;

void new_handler(int signum)
{
	*run = false;
}

void init_termination_signal(bool* do_run, double hardResetTimeout)
{
	startTime = timer.Tock();
	run = do_run;
	*run = true;

	for (auto sgn : { SIGINT , SIGHUP , SIGTERM })
	{
		if (signal(sgn, new_handler) == SIG_IGN)
			signal(sgn, SIG_IGN);
	}

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

