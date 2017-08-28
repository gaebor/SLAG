#include "..\OS_dependent.h"

#include <windows.h>

#include "aq/Clock.h"

static bool* run;
static double startTime;
static double timeout;
static aq::Clock timer;

BOOL WINAPI consoleHandler(DWORD signal) {
	switch (signal)
	{
	case CTRL_C_EVENT:
	case CTRL_BREAK_EVENT:
	case CTRL_CLOSE_EVENT:
		*run = false; //TODO end the Compute for each module
		break;
	default:
		break;
	}

	return TRUE;
}

void init_termination_signal(bool* do_run, double hardResetTimeout)
{
	startTime = timer.Tock();
	run = do_run;
	*run = true;

	//! register console termination handler
	if (!SetConsoleCtrlHandler(consoleHandler, TRUE)) {
		fprintf(stderr, "\nCould not set control handler!\n"); 
	}
	
	timeout = hardResetTimeout;
}

void wait_termination_signal()
{
	while (*run)
	{
		if ( timeout > 0 && startTime + timeout <= timer.Tock())
			*run = false;
		Sleep(1);
	}
}
