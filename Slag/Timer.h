#ifndef INCLUDE_TIMER_H
#define INCLUDE_TIMER_H

#include "Poco\Clock.h"

//!basic time functions
class Timer
{
public:
	//!initializes timer, also calls a Tick
	Timer(void);
	~Timer(void);
	//!marks and returns time interval between now and "tick"
	double Tock();
	//!marks start time
	void Tick();
	//!returns a pointer to a C string, containing the readable, unique start time of the process
	static const char* GetStartTime();
private:
	Poco::Clock clock;
};

#endif //INCLUDE_TIMER_H