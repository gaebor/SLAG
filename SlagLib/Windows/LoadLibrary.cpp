#include "OS_dependent.h"

#include <vector>
#include <string>
#include <stdio.h>

#include <windows.h>
#include <Shlwapi.h>

#if _MSC_VER < 1900
//	without any better
#	define snprintf sprintf_s
#endif 

SlagImageType get_image_type(void)
{
    return SlagImageType::RGBA;
}

void* load_library(const char* file_name)
{
    void* result = nullptr;
    const int wchars_num = MultiByteToWideChar(CP_UTF8, 0, file_name, -1, NULL, 0);
    if (wchars_num > 0)
    {
        wchar_t* wstr = new (std::nothrow) wchar_t[wchars_num];
        if (wstr && MultiByteToWideChar(CP_UTF8, 0, file_name, -1, wstr, wchars_num) > 0)
        {
            result = ::LoadLibraryW(wstr);
        }
        if (wstr)
            delete[] wstr;
    }
    return result;
}

bool close_library( void* library )
{
	return ::FreeLibrary((HMODULE)library) == TRUE;
}

void* get_symbol_from_library(void* library, const char* symbol_name)
{
	return ::GetProcAddress((HMODULE)library, symbol_name);
}

std::vector<std::string> enlist_libraries()
{
	std::vector<std::string> result;

	WIN32_FIND_DATAW FindFileData;
	HANDLE hFind;

	wchar_t exec_name[1024];
    wchar_t* filename_pos = nullptr;
	GetModuleFileNameW(NULL, exec_name, 1024-7);
    if (GetLastError() == ERROR_SUCCESS)
        filename_pos = PathFindFileNameW(exec_name);
	if (filename_pos)
        swprintf(filename_pos, 7, L"*.dll");
    else
        swprintf(exec_name, 7, L"*.dll");

	hFind = FindFirstFileW(exec_name, &FindFileData);

	if (hFind != INVALID_HANDLE_VALUE)
	{
		do{
            result.emplace_back();
            std::string& current = result.back();
            const int size = (int)wcslen(FindFileData.cFileName);
            const int size_needed = WideCharToMultiByte(CP_UTF8, 0, FindFileData.cFileName, size, NULL, 0, NULL, NULL);
            if (size_needed > 0)
            {
                current.resize(size_needed);
                if (WideCharToMultiByte(CP_UTF8, 0, FindFileData.cFileName, size, &current[0], size_needed, NULL, NULL) > 0)
                    continue;
            }
            result.pop_back();
        }while (FindNextFileW(hFind, &FindFileData));
		FindClose(hFind);
	}

	return result;
}

std::string get_file_name(const std::string& file_name)
{
	return std::string(PathFindFileNameA(file_name.c_str()), PathFindExtensionA(file_name.c_str()));
}
