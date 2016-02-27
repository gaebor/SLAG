#include "Timer.h"
#include <stdio.h>
#include "Poco\DateTime.h"

Timer::Timer(void)
{
	Tick();
}

Timer::~Timer(void)
{
}

double Timer::Tock()
{
	return clock.elapsed() / 1000000.0;
}

void Timer::Tick()
{
	clock.update();
}

static int init();

static SYSTEMTIME startUpTime;
static const int init_val = init();

static char timeStr[30];

static int init()
{
	Poco::DateTime dateTime;
	sprintf(timeStr, "%04d_%02d_%02d_%02dh%02dm%02ds_%03d%03dus",
		dateTime.year(), dateTime.month(), dateTime.day(),
		dateTime.hour(), dateTime.minute(), dateTime.second(),
		dateTime.millisecond(), dateTime.microsecond());

	return 0;
}

const char* Timer::GetStartTime()
{
	return timeStr;
}
