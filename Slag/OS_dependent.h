#ifndef INCLUDE_OS_DEPENDENT_H
#define INCLUDE_OS_DEPENDENT_H

#include <string>
#include <map>
#include <vector>

#include "ModuleIdentifier.h"
#include "slag\slag_interface.h"

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

//! textual visualization speed can be set
/*!
Textual output should be readable for a human, so you don't want to update it at 60fps or so.
You can neglect this if you think otherwise, although additional overhead is expected due to thread locks.
*/
void handle_statistics(const std::string& module_name_and_instance, double speed, double computeSpeed, const std::map<PortNumber, size_t>& buffer_sizes);

ImageType get_image_type(void);
void handle_output_image(const std::string& module_name_and_instance, int w, int h, ImageType type, const unsigned char* data);
void terminate_output_image();

//!loads a library, .so or .dll
void* load_library(const char* file_name);

//!closes a library
bool close_library(void* library);

//!extracts symbol from library
void* get_symbol_from_library(void* library, const char* symbol_name);

//!list files like *.dll or *.so
/*!
	/todo give a search path
*/
std::vector<std::string> enlist_libraries();

//! takes off the extension and directory, leaves filename only!
std::string get_file_name(const std::string& file_name);

#endif //INCLUDE_OS_DEPENDENT_H