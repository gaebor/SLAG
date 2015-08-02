#ifndef INCLUDE_OS_DEPENDENT_H
#define INCLUDE_OS_DEPENDENT_H

#include <windows.h>
#include <string>
#include <map>

#include "ModuleIdentifier.h"

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

void handle_output_text(const std::string& module_name_and_instance, const char* text);
void terminate_output_text();
void set_output_text_speed(int milisec_to_wait);
void handle_statistics(const std::string& module_name_and_instance, double speed, double computeSpeed, const std::map<PortNumber, size_t>& buffer_sizes);

#endif //INCLUDE_OS_DEPENDENT_H