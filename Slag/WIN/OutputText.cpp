#include <windows.h>
#include <io.h>

#include "../OS_dependent.h"

#include <map>
#include <thread>
#include <memory>
#include <mutex>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <chrono>

#include "hr.h"

#undef max

struct ModuleTextualData
{
	ModuleTextualData()
		: strout(nullptr), strout_old(nullptr),
        cycle_time(0.0), compute_time(0.0), wait_time(0.0)
	{
	}
	const char* strout;
    const char* strout_old;
    int strout_length;
    // PHANDLE read_end;
	double cycle_time, compute_time, wait_time;
	std::vector<std::pair<PortNumber, size_t>> bufferSizes;
};

static bool run = true;
static double _speed = 0.5;
static std::map<std::string, ModuleTextualData> _texts;
static std::mutex _mutex;
static int nameOffset;
static char wait_marker = '-', overhead_marker = '+', load_marker = ' ';

typedef std::lock_guard<std::mutex> AutoLock;

static inline int round_int( double x )
{
	return int(x+0.5);
}

static std::shared_ptr<std::thread> _textThread;

void terminate_output_text()
{
	run = false;
	if (_textThread.get() && _textThread->joinable())
		_textThread->join();
}

void* get_txtout(const std::string& module_name_and_instance)
{
    return nullptr;
    //auto& textual_data = _texts[module_name_and_instance];
    //HANDLE write_end;

    //if (textual_data.output == nullptr)
    //{
    //    if (CreatePipe(textual_data.read_end, &write_end, NULL,0))
    //        textual_data.output = (FILE*)write_end;
    //}

    //return _texts[module_name_and_instance].output;
}

void* get_txtin(const std::string& module_name_and_instance)
{
    return nullptr;
}
void configure_output_text(const std::vector<std::string>& params)
{
	for (size_t i = 0; i < params.size(); ++i)
	{
		if (params[i] == "-s" || params[i] == "--speed" && i + 1 < params.size())
		{
			double speed = atof(params[i + 1].c_str());
			if (speed >= 0)
				_speed = speed;
		}
		else if (params[i] == "-w" || params[i] == "--wait" && i + 1 < params.size())
			wait_marker = params[i + 1][0];
		else if (params[i] == "-o" || params[i] == "--overhead" && i + 1 < params.size())
			overhead_marker = params[i + 1][0];
		else if (params[i] == "-l" || params[i] == "--load" && i + 1 < params.size())
			load_marker = params[i + 1][0];
	}

	_textThread.reset(new std::thread([]()
	{
		static auto const hCon = GetStdHandle(STD_OUTPUT_HANDLE);

		std::string nameTag = "  module  ";
		nameOffset = (int)nameTag.size();
        SHORT cursor = 0;
        SHORT cursor_end;
		CONSOLE_SCREEN_BUFFER_INFO cinfo;
		COORD coord;
        SHORT width = 80;

		if (GetConsoleScreenBufferInfo(hCon, &cinfo))
			cursor_end = cinfo.dwCursorPosition.Y;
		else
			cursor_end = 0;

		while (run)
		{
			{
				AutoLock lock(_mutex);

				if (GetConsoleScreenBufferInfo(hCon, &cinfo))
				{
					cursor = cinfo.dwCursorPosition.Y;
					cursor_end = std::max(cursor, cursor_end);
                    width = cinfo.dwSize.X;
				}

				printf("%-*s", nameOffset, nameTag.c_str());
				printf("|  speed   | overhead | text output\n");
				for (int i = 0; i < nameOffset; ++i)
					putchar('-');
				printf("+----------+----------+-\n");

				for (auto& m : _texts)
				{
                    std::vector<char> line(width + 1, '\0');
					hr_time(line.data(), width + 1, (float)m.second.cycle_time, "");
					const int load = round_int(10 * m.second.compute_time / m.second.cycle_time);
					const int wait = round_int(10 * m.second.wait_time / m.second.cycle_time);
					printf("%-*s|%s|", nameOffset, m.first.c_str(), line.data() + line.size());
					for (int i = 0; i < wait; ++i)
						putchar(wait_marker);
					for (int i = 0; i < load; ++i)
						putchar(load_marker);
					for (int i = load + wait; i < 10; ++i)
						putchar(overhead_marker);

					putchar('|');

                    for (int i = nameOffset + 20;
                         *m.second.strout && *m.second.strout != '\n' && i < width;
                         ++(m.second.strout), ++i)
                    {
                        putchar(*m.second.strout);
                    }
					putchar('\n');

					for (auto& q : m.second.bufferSizes)
					{
						hr_giga(line.data(), width + 1, (float)q.second, "");
						printf("%*s|> %*d|\n", nameOffset, line.data(), -19, q.first);
					}

					for (int i = 0; i < nameOffset; ++i)
						putchar('-');
					printf("+----------+----------+-\n");
				}
				printf("\n");

				if (GetConsoleScreenBufferInfo(hCon, &cinfo))
					cursor_end = std::max(cursor_end, cinfo.dwCursorPosition.Y);
				
				coord.X = 0;
				coord.Y = cursor;
				SetConsoleCursorPosition(hCon, coord);
				
				//_texts.clear();
			}
			std::this_thread::sleep_for(std::chrono::nanoseconds((std::int64_t)(_speed * std::nano::den / std::nano::num)));
		}
		//if (started)
		//	internal_func();

		coord.X = 0;
		coord.Y = cursor_end;
		SetConsoleCursorPosition(hCon, coord);
	}));
}

void handle_output_text( const std::string& module_name_and_instance, const char* text, int length)
{
	if (text)
	{
		AutoLock lock(_mutex);
		auto& data = _texts[module_name_and_instance];
        data.strout = text;
        data.strout_length = length;
		nameOffset = std::max((int)module_name_and_instance.size(), nameOffset);
	}
}

void handle_statistics( const std::string& module_name_and_instance, double cycle, double load, double wait, const std::map<PortNumber, size_t>& buffer_sizes)
{
	AutoLock lock(_mutex);
	auto& data = _texts[module_name_and_instance];
	data.cycle_time = cycle;
	data.compute_time = load;
	data.wait_time = wait;
	nameOffset =  std::max((int)module_name_and_instance.size(), nameOffset);
	data.bufferSizes.assign(buffer_sizes.begin(), buffer_sizes.end());
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
	}
	return argv;
}
