#include "..\OS_dependent.h"

#include <vector>
#include <string>

#if defined _MSC_VER
#include <windows.h>
#include <Shlwapi.h>

void* load_library(const char* file_name)
{
	return ::LoadLibraryA(file_name);
}

bool close_library( void* library )
{
	return FreeLibrary((HMODULE)library) == TRUE;
}

void* get_symbol_from_library(void* library, const char* symbol_name)
{
	return GetProcAddress((HMODULE)library, symbol_name);
}

std::vector<std::string> enlist_libraries()
{
	std::vector<std::string> result;

	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;

	hFind = FindFirstFile("*.dll", &FindFileData);

	if (hFind != INVALID_HANDLE_VALUE)
	{
		do{
			result.emplace_back(FindFileData.cFileName);
		}while (FindNextFile(hFind, &FindFileData));
		FindClose(hFind);
	}

	return result;
}

std::string get_file_name(const std::string& file_name)
{
	return std::string(PathFindFileName(file_name.c_str()), PathFindExtension(file_name.c_str()));
}

#elif defined __GNUC__

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

#endif