#include "..\OS_dependent.h"

#include <vector>
#include <string>
#include <stdio.h>

#include <windows.h>
#include <Shlwapi.h>

#if _MSC_VER < 1900
//	without any better
#	define snprintf sprintf_s
#endif 

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

	char exec_name[256];
	GetModuleFileNameA(NULL, exec_name, 249);
	if (GetLastError() == ERROR_SUCCESS)
		snprintf(strrchr(exec_name, '\\'), 7, "\\*.dll");
	else
		snprintf(exec_name, 255, "*.dll");

	hFind = FindFirstFileA(exec_name, &FindFileData);

	if (hFind != INVALID_HANDLE_VALUE)
	{
		do{
			result.emplace_back(FindFileData.cFileName);
		}while (FindNextFileA(hFind, &FindFileData));
		FindClose(hFind);
	}

	return result;
}

std::string get_file_name(const std::string& file_name)
{
	return std::string(PathFindFileNameA(file_name.c_str()), PathFindExtensionA(file_name.c_str()));
}
