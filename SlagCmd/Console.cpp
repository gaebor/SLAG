#include <windows.h>
#include <io.h>

#include "Additionals.h"

int GetConsoleWidth()
{
    CONSOLE_SCREEN_BUFFER_INFO cinfo;

    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cinfo))
        return cinfo.dwSize.X;
    else
        return 80;
}

static int cursor_pos;

void RememberCursorPosition()
{
    CONSOLE_SCREEN_BUFFER_INFO cinfo;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cinfo))
        cursor_pos = cursor_pos > cinfo.dwCursorPosition.Y ? cursor_pos : cinfo.dwCursorPosition.Y;
}
void RestoreCursorPosition()
{
    COORD coord;
    coord.X = 0;
    coord.Y = cursor_pos;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

std::vector<std::string> split_to_argv(const std::string& line)
{
    std::vector<std::string> argv;
    std::wstring wstr(line.begin(), line.end());
    int size;
    auto result = CommandLineToArgvW(wstr.c_str(), &size);
    if (result)
    {
        for (int i = 0; i < size; ++i)
            argv.emplace_back(result[i], result[i] + wcslen(result[i]));
        LocalFree(result);
    }
    return argv;
}
