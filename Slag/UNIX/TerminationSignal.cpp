#include "../OS_dependent.h"
#include "aq/Clock.h"
#include <thread>

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
