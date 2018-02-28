#ifndef INCLUDE_OS_DEPENDENT_H
#define INCLUDE_OS_DEPENDENT_H

#include <string>
#include <vector>
#include <map>

#include "slag/slag_interface.h"
#include "ModuleIdentifier.h"

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
	 * do_run set to false
	 * terminate signal from OS
*/
void wait_termination_signal();

//!implement the textual output here
/*!
	Don't block this call too much, make it fast. If this is time-consuming, make a background thread.
	Store the data for yourself.
	These calls will be asynchronous.
*/
// void handle_output_text(const std::string& module_name_and_instance, const char* text, int length);
void* get_output_text_handle(const std::string& module_name_and_instance);

//!SLAG calls this if the process graph has ended.
/*!
	You have the opportunity to close what you are doing!
*/
void terminate_output_text();

//! textual visualization settings
/*!
	Textual output should be readable for a human, so you don't want to update it at 60fps or so.
	You can neglect this if you think otherwise, although additional overhead is expected due to thread locks.
*/
void configure_output_text(const std::vector<std::string>& params);

//! textual visualization speed can be set
/*!
Textual output should be readable for a human, so you don't want to update it at 60fps or so.
You can neglect this if you think otherwise, although additional overhead is expected due to thread locks.
@param cycle the time of one evaluation cycle
@param load the time of the module compute function
@param wait the time needed to wait its inputs
*/
void handle_statistics(const std::string& module_name_and_instance, double cycle, double load, double wait, const std::map<PortNumber, size_t>& buffer_sizes);

//! return your OS's favourable image type
ImageType get_image_type(void);
void handle_output_image(const std::string& module_name_and_instance, int w, int h, ImageType type, const unsigned char* data);
void terminate_output_image();

//! image visualization settings
void configure_output_image(const std::vector<std::string>&);

//!loads a library, .so or .dll
void* load_library(const char* file_name);

//!closes a library
bool close_library(void* library);

//!extracts symbol from library
void* get_symbol_from_library(void* library, const char* symbol_name);

//!list files like *.dll or *.so
/*!
	it searches the directory of the executable
*/
std::vector<std::string> enlist_libraries();

//! takes off the extension and directory, leaves filename only!
std::string get_file_name(const std::string& file_name);

//! splits a string like the command line do
std::vector<std::string> split_to_argv(const std::string& line);

#endif //INCLUDE_OS_DEPENDENT_H
