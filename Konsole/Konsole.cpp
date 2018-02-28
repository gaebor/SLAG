#include <windows.h>
#include <io.h>
#include <fcntl.h>

#include<stdio.h>

#include<string>

#include<iostream>
#include<fstream>
#include<sstream>

class KonsoleProcess
{
public:
	KonsoleProcess(const char* name, const char* dir = NULL)
		: input(NULL)
	{
		saAttr.nLength = sizeof(saAttr);
		saAttr.bInheritHandle = TRUE;
		saAttr.lpSecurityDescriptor = NULL;

		HANDLE g_hChildStd_IN_Rd = NULL;
		HANDLE g_hChildStd_IN_Wr = NULL;

		if (!CreatePipe(&g_hChildStd_IN_Rd, &g_hChildStd_IN_Wr, &saAttr, 0))
			return;

		if (!SetHandleInformation(g_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0))
			return;

		// Set up members of the PROCESS_INFORMATION structure. 

		ZeroMemory(&piProcInfo, sizeof(piProcInfo));

		// Set up members of the STARTUPINFO structure. 
		// This structure specifies the STDIN and STDOUT handles for redirection.

		ZeroMemory(&siStartInfo, sizeof(siStartInfo));
		siStartInfo.cb = sizeof(siStartInfo);
		siStartInfo.hStdInput = g_hChildStd_IN_Rd;
		siStartInfo.lpTitle = (char*)name;
		siStartInfo.dwFlags |= STARTF_USESTDHANDLES;
		
		siStartInfo.dwFlags |= STARTF_USESHOWWINDOW;
		siStartInfo.wShowWindow = SW_SHOWNOACTIVATE;

		int cols = 80;
		int rows = 30;
		int posx = 0;
		int posy = 0;

		{
			std::string line(name);
			line += ".txt.pos";
			std::ifstream ifs(line);
			if (std::getline(ifs, line) && std::getline(ifs, line))
			{
				std::istringstream iss(line);
				iss >> cols >> rows >> posx >> posy;
				if (cols <= 0)
					cols = 80;
				if (rows <= 0)
					rows = 30;
				if (posx < 0)
					posx = 0;
				if (posy < 0)
					posy = 0;
			}
		}
		siStartInfo.dwFlags |= STARTF_USEPOSITION;
		siStartInfo.dwFlags |= STARTF_USECOUNTCHARS;

		siStartInfo.dwX = posx;
		siStartInfo.dwY = posy;
		siStartInfo.dwXCountChars = cols;
		siStartInfo.dwXCountChars = rows;

		char exec_name[256];
		GetModuleFileNameA(NULL, exec_name, 246);
		snprintf(strrchr(exec_name, '\\'), 9, "\\cat.exe");

		//TODO maybe process naming affects cmd window font settings
		//what is I set the font, close app, then I create a new cmd window
		// I want to keep font settings (position can be explicitly told)

		if (!CreateProcessA(NULL,
			exec_name,         // command line 
			NULL,          // process security attributes 
			NULL,          // primary thread security attributes 
			TRUE,          // handles are inherited 
			CREATE_NEW_CONSOLE, // creation flags 
			NULL,          // use parent's environment 
			dir,           // directory 
			&siStartInfo,  // STARTUPINFO pointer 
			&piProcInfo))  // receives PROCESS_INFORMATION 
		{
			return;
		}
		input = _fdopen(_open_osfhandle((intptr_t)g_hChildStd_IN_Wr, _O_BINARY), "wb");
	}
	~KonsoleProcess()
	{
		fflush(input);
		system("pause");
		
		fclose(input);
		WaitForSingleObject(piProcInfo.hProcess, INFINITE);
		CloseHandle(piProcInfo.hProcess);
		CloseHandle(piProcInfo.hThread);
	}
	operator FILE* ()const
	{
		return input;
	}
private:
	SECURITY_ATTRIBUTES saAttr;
	FILE* input;
	STARTUPINFOA siStartInfo;
	PROCESS_INFORMATION piProcInfo;
};

int main()
{
	KonsoleProcess process("name");
	
	fprintf(process, "%s\n", "hell\xc5\x91 w\xc5\xb1rld");
	fprintf(process, "%s\n", "\xc3\xa1\x72\x76\xc3\xad\x7a\x74\xc5\xb1\x72\xc5\x91");
    return 0;
}

