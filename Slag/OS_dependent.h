#ifndef INCLUDE_OS_DEPENDENT_H
#define INCLUDE_OS_DEPENDENT_H

#include <windows.h>
#include <string>
#include <map>

#include "ModuleIdentifier.h"
#include "slag\slag_interface.h"

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
	static LARGE_INTEGER freq;
	LARGE_INTEGER tick;
	LARGE_INTEGER tock;
};

//!initializes termination events
/*!
	termination signal can be:
	 * timeout expired (measured from the time point of this very call)
	 * modules ran out of job, this is what do_run is used to
	 * console CTRL+C event, or other kind of halt, kill, logout, terminate signals by the OS
	@param do_run pointer to a unified terminating signal, it can be written by others and this function can write it as well
	@param hardResetTimeout timeout in seconds, it is not the most accurate. If 0 or negative value is set, then timeout is infinity
*/
void init_termination_signal(bool* do_run, double hardResetTimeout = 0.0);

//!returns when one of the termination events occur
/*!
	termination event can be:
	 * timeout expired
	 * modules ran out of job
	 * console CTRL+C event
*/
void wait_termination_signal();

//!implement the textual output here
/*!
	Don't block this call too much, make it fast. If this is time-consuming, make a background thread.
	Store the data for yourself.
	These calls will be asynchronous.
*/
void handle_output_text(const std::string& module_name_and_instance, const char* text);

//!SLAG calls this if the process graph has ended.
/*!
	You have the opportunity to close what you are doing!
*/
void terminate_output_text();

//! textual visualization speed can be set
/*!
	Textual output should be readable for a human, so you don't want to update it at 60fps or so.
	You can neglect this if you think otherwise, although additional overhead is expected due to thread locks.
*/
void set_output_text_speed(int milisec_to_wait);


void handle_statistics(const std::string& module_name_and_instance, double speed, double computeSpeed, const std::map<PortNumber, size_t>& buffer_sizes);
void handle_output_image(const std::string& module_name_and_instance, int w, int h, ImageType type, const unsigned char* data);
void terminate_output_image();

#endif //INCLUDE_OS_DEPENDENT_H