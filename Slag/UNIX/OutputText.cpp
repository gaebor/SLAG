#include "../OS_dependent.h"

#include <map>
#include <thread>
#include <memory>
#include <mutex>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <wordexp.h>

#include "hr.h"

#undef max

struct ModuleTextualData
{
	ModuleTextualData()
		: output(), cycle_time(0.0), compute_time(0.0), wait_time(0.0)
	{
	}
	std::vector<char> output;
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

static inline int round_int(double x)
{
	return int(x + 0.5);
}

static std::shared_ptr<std::thread> _textThread;

void terminate_output_text()
{
	run = false;
	if (_textThread.get() && _textThread->joinable())
		_textThread->join();
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
		std::string nameTag = "  module  ";
		nameOffset = (int)nameTag.size();

		while (run)
		{
			{
				AutoLock lock(_mutex);

				printf("\033[%d;%dH", 1, 1);

				printf("%-*s", nameOffset, nameTag.c_str());
				printf("|  speed   | overhead | text output\n");
				for (int i = 0; i < nameOffset; ++i)
					putchar('-');
				printf("+----------+----------+-\n");

				for (const auto& m : _texts)
				{
					char line[1024];
					hr_time(line, 1024, m.second.cycle_time, "");
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
					fwrite(m.second.output.data(), 1, m.second.output.size(), stdout);
					putchar('\n');

					for (auto& q : m.second.bufferSizes)
					{
						hr_giga(line, 1024, (double)q.second, "");
						printf("%*s|> %*d|\n", nameOffset, line, -19, q.first);
					}

					for (int i = 0; i < nameOffset; ++i)
						putchar('-');
					printf("+----------+----------+-\n");
				}
				printf("\n");
			}
			std::this_thread::sleep_for(std::chrono::nanoseconds((std::int64_t)(_speed * std::nano::den / std::nano::num)));
		}
	}));
}

void handle_output_text(const std::string& module_name_and_instance, const char* text, int length)
{
	if (text)
	{
		AutoLock lock(_mutex);
		auto& data = _texts[module_name_and_instance];
		data.output.assign(text, text + length);
		nameOffset = std::max((int)module_name_and_instance.size(), nameOffset);
	}
}

void handle_statistics(const std::string& module_name_and_instance, double cycle, double load, double wait, const std::map<PortNumber, size_t>& buffer_sizes)
{
	AutoLock lock(_mutex);
	auto& data = _texts[module_name_and_instance];
	data.cycle_time = cycle;
	data.compute_time = load;
	data.wait_time = wait;
	nameOffset = std::max((int)module_name_and_instance.size(), nameOffset);
	data.bufferSizes.assign(buffer_sizes.begin(), buffer_sizes.end());
}

std::vector<std::string> split_to_argv(const std::string& line)
{
	wordexp_t wordexp_result;
	std::vector<std::string> argv;

	if (wordexp(line.c_str(), &wordexp_result, 0) == 0)
	{
		for (size_t i = 0; i < wordexp_result.we_wordc; ++i)
			argv.emplace_back(wordexp_result.we_wordv[i]);

		wordfree(&wordexp_result);
	}
	return argv;
}

//TODO pipe write to console
void* get_txtout(const std::string&)
{
    return nullptr;
}

void* get_txtin(const std::string&)
{
    return nullptr;
}

