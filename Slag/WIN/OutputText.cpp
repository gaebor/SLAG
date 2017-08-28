#include <windows.h>

#include "../OS_dependent.h"

#include <map>
#include <thread>
#include <memory>
#include <mutex>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <chrono>

#include "../HumanReadable.h"

#undef max

struct ModuleTextualData
{
	ModuleTextualData()
		: output(), cycle_time(0.0), compute_time(0.0), wait_time(0.0)
	{
	}
	std::string output;
	double cycle_time, compute_time, wait_time;
	std::vector<std::pair<PortNumber, size_t>> bufferSizes;
};

static bool run = true;
static bool started = false; //it is set to true at the first handle_output_text call
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

static std::thread _textThread([]()
{
	std::string nameTag = "  module  ";
	nameOffset = (int)nameTag.size();
	short cursor = 0;
	short cursor_end;

	static auto const hCon = GetStdHandle(STD_OUTPUT_HANDLE);

	auto internal_func = [&]()
	{
		CONSOLE_SCREEN_BUFFER_INFO cinfo;
		if (GetConsoleScreenBufferInfo(hCon, &cinfo))
		{
			cursor = cinfo.dwCursorPosition.Y;
			cursor_end = cursor;
		}
		
		printf("%-*s", nameOffset, nameTag.c_str());
		printf("|  speed   | overhead | text output\n");
		for (int i = 0; i < nameOffset; ++i)
			putchar('-');
		printf("+----------+----------+-\n");

		for (const auto& m : _texts)
		{
			char line[1024];
			print_humanreadable_time(line, 1024, m.second.cycle_time);
			const int load = round_int(10 * m.second.compute_time / m.second.cycle_time);
			const int wait = round_int(10 * m.second.wait_time / m.second.cycle_time);
			printf("%-*s|%s|", nameOffset, m.first.c_str(), line);
			for (int i = 0; i < wait; ++i)
				putchar(wait_marker);
			for (int i = 0; i < load; ++i)
				putchar(load_marker);
			for (int i = load + wait; i < 10; ++i)
				putchar(overhead_marker);
			
			putchar('|');
			std::cout << m.second.output << "\n";

			for (auto& q : m.second.bufferSizes)
			{
				print_humanreadable_giga(line, 1024, (double)q.second);
				printf("%*s|> %d\n", nameOffset, line, q.first);
			}
			
			for (int i = 0; i < nameOffset; ++i)
				putchar('-');
			printf("+----------+----------+-\n");
		}
		printf("\n");

		if (GetConsoleScreenBufferInfo(hCon, &cinfo))
			cursor_end = cinfo.dwCursorPosition.Y;
		
		COORD coord;
		coord.X = 0;
		coord.Y = cursor;
		SetConsoleCursorPosition(hCon, coord);
	};

	// waits until anything happens, or the run has completed even before printing anything out
	while (!started && run)
		std::this_thread::sleep_for(std::chrono::nanoseconds((std::int64_t)(_speed * std::nano::den / std::nano::num)));

	while (run)
	{
		{
		AutoLock lock(_mutex);
		internal_func();
		}
		std::this_thread::sleep_for(std::chrono::nanoseconds((std::int64_t)(_speed * std::nano::den / std::nano::num)));
	}
	if (started)
		internal_func();

	COORD coord;
	coord.X = 0;
	coord.Y = cursor_end;
	SetConsoleCursorPosition(hCon, coord);
});

void terminate_output_text()
{
	run = false;
	_textThread.join();
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
}

void handle_output_text( const std::string& module_name_and_instance, const char* text )
{
	static const bool start = (started = true);
	if (text)
	{
		AutoLock lock(_mutex);
		auto& data = _texts[module_name_and_instance];
		data.output = text;
		nameOffset = std::max((int)module_name_and_instance.size(), nameOffset);
	}
}

void handle_statistics( const std::string& module_name_and_instance, double cycle, double load, double wait, const std::map<PortNumber, size_t>& buffer_sizes)
{
	static const bool start = (started = true);
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
