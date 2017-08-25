#include "OS_dependent.h"

#include <map>
#include <thread>
#include <memory>
#include <mutex>
#include <iostream>
#include <sstream>
#include <algorithm>

#include "HumanReadable.h"
#include "ModuleIdentifier.h"

std::vector<std::string> split_to_argv(const std::string& line)
{
	std::istringstream iss(line);
	std::vector<std::string> result;
	std::string arg;
	while (iss >> arg)
	{
		result.push_back(arg);
	}
	return result;
}

struct ModuleTextualData
{
	ModuleTextualData()
		: output(), speed(0.0), computeSpeed(0.0)
	{
	}
	std::string output;
	double speed, computeSpeed;
	std::vector<std::pair<PortNumber, size_t>> bufferSizes;
};

static bool run = true;
static bool started = false; //it is set to true at the first handle_output_text call
static int _speed = 500; //default refresh rate is half second
static std::map<std::string, ModuleTextualData> _texts;
static std::mutex _mutex;
static int nameOffset;

typedef std::lock_guard<std::mutex> AutoLock;

static inline int round_int( double x )
{
	return int(x+0.5);
}

static std::thread _textThread([]()
{
	std::string nameTag = "  module  ";
	nameOffset = nameTag.size();

	auto internal_func = [&]()
	{
#if defined _MSC_VER
		system("cls");
#elif defined __GNUC__
		system("clear");
#endif
		
		printf("%-*s", nameOffset, nameTag.c_str());
		printf("|  speed   | overhead | text output\n");
		for (int i = 0; i < nameOffset; ++i)
			putchar('-');
		printf("+----------+----------+-\n");

		for (const auto& m : _texts)
		{
			char line[1024];
			print_humanreadable_time(line, 1024, m.second.speed);
			const int load = round_int(10*(m.second.computeSpeed)/(m.second.speed));

			printf("%-*s|%s|", nameOffset, m.first.c_str(), line);
			for (int i = 0; i < 10-load; ++i)
				putchar('=');
			for (int i = 10-load; i < 10; ++i)
				putchar(' ');
			putchar('|');
			std::cout << m.second.output << "\n";

			for (auto& q : m.second.bufferSizes)
			{
				print_humanreadable_giga(line, 1024, q.second);
				printf("%*s|> %d\n", nameOffset, line, q.first);
			}
			
			for (int i = 0; i < nameOffset; ++i)
				putchar('-');
			printf("+----------+----------+-\n");
		}
		//FeedImshow();
	};

	while (!started && run)
		std::this_thread::sleep_for(std::chrono::milliseconds(_speed));

	while (run)
	{
		{
		AutoLock lock(_mutex);
		internal_func();
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(_speed));
	}
	if (started)
		internal_func();
});

void terminate_output_text()
{
	run = false;
	_textThread.join();
}

void set_output_text_speed( int milisec_to_wait )
{
	if (milisec_to_wait >= 0)
		_speed = milisec_to_wait;
}

void handle_output_text( const std::string& module_name_and_instance, const char* text )
{
	static const bool start = (started = true);
	AutoLock lock(_mutex);
	auto& data = _texts[module_name_and_instance];
	data.output = text;
	nameOffset =  std::max<int>(module_name_and_instance.size(), nameOffset);
}

void handle_statistics( const std::string& module_name_and_instance, double speed, double computeSpeed, const std::map<PortNumber, size_t>& buffer_sizes)
{
	static const bool start = (started = true);
	AutoLock lock(_mutex);
	auto& data = _texts[module_name_and_instance];
	data.speed = speed;
	data.computeSpeed = computeSpeed;
	nameOffset =  std::max<int>(module_name_and_instance.size(), nameOffset);
	data.bufferSizes.assign(buffer_sizes.begin(), buffer_sizes.end());
}
