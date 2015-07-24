#include "Timer.h"

Timer::Timer(void)
{
	static BOOL f = QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&tick);
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
