#include "OS_dependent.h"

#include <chrono>

Timer::Timer(void)
{
	static BOOL f = QueryPerformanceFrequency(&freq);
	Tick();
}

Timer::~Timer(void)
{
}

double Timer::Tock()
{
	QueryPerformanceCounter(&tock);
	return (double)(tock.QuadPart-tick.QuadPart)/freq.QuadPart;
}

void Timer::Tick()
{
	QueryPerformanceCounter(&tick);
}

LARGE_INTEGER Timer::freq;

static int init();

static SYSTEMTIME startUpTime;
static const int init_val = init();

static char timeStr[1024];

static int init()
{
	GetLocalTime(&startUpTime);

	Timer now;
	double fraqtime = now.Tock();
	fraqtime = fraqtime-(size_t)fraqtime;
	sprintf_s(timeStr, 1024, "%d_%d_%d_%02d%02d%02d_%f", startUpTime.wYear, startUpTime.wMonth, startUpTime.wDay, startUpTime.wHour, startUpTime.wMinute, startUpTime.wSecond, fraqtime );

	return 0;
}

const char* Timer::GetStartTime()
{
	return timeStr;
}
