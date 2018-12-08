#ifndef INCLUDE_OS_DEPENDENT_H
#define INCLUDE_OS_DEPENDENT_H

#include <string>
#include <vector>
#include <map>

#include "slag_interface.h"
#include "ModuleIdentifier.h"

//!implement the textual output here
/*!
	Don't block this call too much, make it fast. If this is time-consuming, make a background thread.
	Store the data for yourself.
	These calls will be asynchronous.
*/
void handle_output_text(const std::string& module_name_and_instance, const char* text, int length);

//! returns FILE*
void* get_txtout(const std::string& module_name_and_instance);

//! returns FILE*
void* get_txtin(const std::string& module_name_and_instance);

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

//! splits a string like the command line do
std::vector<std::string> split_to_argv(const std::string& line);

#endif //INCLUDE_OS_DEPENDENT_H
