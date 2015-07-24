#pragma once
#include <windows.h>

class Timer
{
public:
	Timer(void);
	~Timer(void);
	double Tock();
	void Tick();
private:
	static LARGE_INTEGER freq;
	LARGE_INTEGER tick;
	LARGE_INTEGER tock;
};


