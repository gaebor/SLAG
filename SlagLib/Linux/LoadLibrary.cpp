#include "OS_dependent.h"

#include <vector>
#include <string>
#include <dlfcn.h>
#include <glob.h>
#include <string.h>

#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <limits.h>

#include <dlfcn.h>

SlagImageType get_image_type(void)
{
    return SlagImageType::RGB;
}

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
	if (dlerror() != NULL)
	{
		return nullptr;
	}else
		return result;
}

std::vector<std::string> enlist_libraries()
{
	// https://stackoverflow.com/questions/4025370/can-an-executable-discover-its-own-path-linux
	char path[PATH_MAX + 7];
	char dest[PATH_MAX];
	memset(dest, 0, sizeof(dest));
	struct stat info;
	pid_t pid = getpid();
	sprintf(path, "/proc/%d/exe", pid);
	if (readlink(path, dest, PATH_MAX) == -1)
		strcpy(path, "*.so"); // fallback
	else {
		strcpy(path, dest);
		// cut off filename
		sprintf(strrchr(path, '/'), "/*.so");
	}

	std::vector<std::string> result;
	glob_t glob_result;
	glob(path, GLOB_TILDE, NULL, &glob_result);
	for(unsigned int i=0;i<glob_result.gl_pathc;++i)
		result.push_back(std::string(glob_result.gl_pathv[i]));
	
	globfree(&glob_result);
	return result;
}

std::string get_file_name(const std::string& file_name)
{
	const char *base = strrchr(file_name.c_str(), '/');
	base = base ? base + 1 : file_name.c_str();

	const char* extension = strrchr(base, '.');
	if (!extension)
		extension = strrchr(base, '\0');

	return std::string(base, extension);
}

