#include "../OS_dependent.h"

#include <vector>
#include <string>
#include <dlfcn.h>
#include <glob.h>
#include <libgen.h>

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

std::vector<std::string> enlist_libraries()
{
	std::vector<std::string> result;
	glob_t glob_result;
	glob("*.so",GLOB_TILDE,NULL,&glob_result);
	for(unsigned int i=0;i<glob_result.gl_pathc;++i)
		result.push_back(std::string(glob_result.gl_pathv[i]));
	
	globfree(&glob_result);
	return result;
}

std::string get_file_name(const std::string& file_name)
{
	std::string temp(file_name);
	return std::string(basename(&(temp[0])));
}

