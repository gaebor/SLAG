#include "../OS_dependent.h"
#include "../Timer.h"
#include "Poco/Thread.h"

static bool* run;
static double startTime;
static double timeout;
static Timer timer;

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
		Poco::Thread::sleep(1);
	}
}
