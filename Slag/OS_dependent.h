#ifndef INCLUDE_OS_DEPENDENT_H
#define INCLUDE_OS_DEPENDENT_H

#include <string>
#include <vector>

#include "slag_interface.h"

ImageType get_image_type(void);

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

#endif //INCLUDE_OS_DEPENDENT_H
