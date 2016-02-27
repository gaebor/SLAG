#include "../OS_dependent.h"

#include <vector>
#include <string>
#include <dlfcn.h>

void* load_library(const char* file_name)
{
	return dlopen(file_name, RTLD_LAZY);
}

bool close_library( void* library )
{
	return dlclose(library) == 0;
}

void* get_symbol_from_library(void* library, const char* symbol_name)
{
	char* error;
	auto result = dlsym(library, symbol_name);
	if ((error = dlerror()) != NULL)
	{
		fputs(error, stderr);
		return nullptr;
	}else
		return result;
}
