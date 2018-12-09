#ifdef _MSC_VER
#include <windows.h>
#endif

#include <iostream>
#include <sstream>
#include <fstream>
#include <thread>

#include "ConfigReader.h"
#include "Graph.h"

#include "aq/Clock.h"

typedef std::lock_guard<std::mutex> AutoLock;

static volatile bool run;

std::vector<std::string> split_to_argv(const std::string& line);

struct ModuleTextualData
{
    ModuleTextualData()
        : cycle_time(0.0), compute_time(0.0), wait_time(0.0), n(0)
    {
    }
    std::string strout;
    const char* strout_end;
    double cycle_time, compute_time, wait_time;
    std::vector<std::pair<PortNumber, size_t>> bufferSizes;
    size_t n;
};

static double _speed = 0.5;
static std::map<std::string, ModuleTextualData> _texts;
static std::mutex _mutex;
static int nameOffset;
static char wait_marker = '~', overhead_marker = '-', load_marker = '#';
static std::thread _textThread;

static inline int round_int(double x)
{
    return int(x + 0.5);
}

void start_text()
{
    if (!_textThread.joinable())
    {
        _textThread = std::thread([]()
        {
            std::string nameTag = "  module  ";
            {
                AutoLock lock(_mutex);
                nameOffset = std::max((int)nameTag.size(), nameOffset);
            }
            
            while (run)
            {
                {
                    AutoLock lock(_mutex);

                    printf("%-*s", nameOffset, nameTag.c_str());
                    printf("| speed | overhead | text output\n");
                    for (int i = 0; i < nameOffset; ++i)
                        putchar('-');
                    printf("+-------+----------+-\n");

                    for (auto& m : _texts)
                    {
                        auto& t = m.second;
                        const int load = round_int(10 * t.compute_time / t.cycle_time);
                        const int wait = round_int(10 * t.wait_time / t.cycle_time);
                        printf("%-*s|%7e|", nameOffset, m.first.c_str(), t.cycle_time);
                        for (int i = 0; i < wait; ++i)
                            putchar(wait_marker);
                        for (int i = 0; i < load; ++i)
                            putchar(load_marker);
                        for (int i = load + wait; i < 10; ++i)
                            putchar(overhead_marker);

                        putchar('|');

                        printf("%.40s\n", m.second.strout.c_str());

                        m.second.n = 0;
                        m.second.compute_time = 0;
                        m.second.cycle_time = 0;
                        m.second.wait_time = 0;

                        //for (auto& q : m.second.bufferSizes)
                        //{
                        //    hr_giga(line.data(), width + 1, (float)q.second, "");
                        //    printf("%*s|> %*d|\n", nameOffset, line.data(), -16, q.first);
                        //}

                        for (int i = 0; i < nameOffset; ++i)
                            putchar('-');
                        printf("+-------+----------+-\n");
                    }
                    printf("\n");
                }
                std::this_thread::sleep_for(std::chrono::nanoseconds((std::intmax_t)(_speed * std::nano::den / std::nano::num)));
            }
        });
    }
}

void handle_output_text(const std::string& module_name_and_instance, const char* text, int length)
{
    if (text)
    {
        AutoLock lock(_mutex);
        auto& data = _texts[module_name_and_instance];
        data.strout.assign(text);
        data.strout_end = text + length;
        nameOffset = std::max((int)module_name_and_instance.size(), nameOffset);
    }
}

static aq::LimitBehavior GetBehavior(const std::string& value)
{
    if (value == "Wait")
        return aq::Wait;
    else if (value == "Drop")
        return aq::Drop;
    else if (value == "Refuse")
        return aq::Refuse;
    return aq::None;
}

void handle_statistics(const std::string& module_name_and_instance, double cycle, double load, double wait, const std::map<PortNumber, size_t>& buffer_sizes)
{
    AutoLock lock(_mutex);
    auto& data = _texts[module_name_and_instance];
    const double denum = 1.0 / (data.n + 1);
    const double ratio = denum * data.n;

    data.cycle_time *= ratio; data.cycle_time += denum * cycle;
    data.compute_time *= ratio; data.compute_time += denum * load;
    data.wait_time *= ratio; data.wait_time += denum * wait;
    nameOffset = std::max((int)module_name_and_instance.size(), nameOffset);
    data.bufferSizes.assign(buffer_sizes.begin(), buffer_sizes.end());
    ++data.n;
}

int main(int argc, char* argv[])
{
	aq::LimitBehavior queueBehavior = aq::None;
	size_t queueLimit = std::numeric_limits<size_t>::max();
	double hardResetTime = 0.0;
    int loglevel = 1;
    Graph graph;

	if (argc < 2)
	{
		std::cerr << "USAGE: " << argv[0] << " >>config.cfg<<" << std::endl;
		goto halt;
	}
	try{
		std::ifstream f(argv[1]);
		if (!f.good())
		{
			std::cerr << "Cannot read \"" << argv[1] << "\"!" << std::endl;
			goto halt;
		}
		ConfigReader cfg(f);
		
		//graph settings
		for (auto l : cfg.GetSection("graph"))
		{
			const auto key = l.substr(0, l.find('='));
			const auto value = l.find('=') != std::string::npos ? l.substr(l.find('=') + 1) : "";

			if (key == "QueueBehavior")
				queueBehavior = GetBehavior(value);
			else if (key == "QueueLimit")
                std::istringstream(value) >> queueLimit;
			else if (key == "HardResetTime")
				std::istringstream(value) >> hardResetTime;
		}

		//instantiate modules
		for (auto moduleStr : cfg.GetSection("modules"))
		{
			const auto arguments = split_to_argv(moduleStr);
            if (arguments.empty())
                continue;
            const std::string moduleName = arguments[0];
            std::cout << "Module \"" << moduleName << "\" ... "; std::cout.flush();
            const FullModuleIdentifier moduleId(moduleName.c_str());

            switch (graph.AddModule(arguments, handle_statistics, handle_output_text))
            {
			case ErrorCode::Duplicate:
				std::cout << "found more than once!";
                break;
            case ErrorCode::AlreadyExists:
                std::cout << "already in the graph!";
                break;
            case ErrorCode::WrongArguments:
                std::cout << "wrong arguments!";
                break;
            case ErrorCode::CannotInitialize:
                std::cout << "instantiated by \"" << graph.GetModuleId(moduleName)->library << "\" but cannot initialize!";
                break;
			case ErrorCode::CannotOpen:
			{
				std::cout << "cannot be instantiated because couldn't open \"" << moduleId.library << "\"!" << std::endl;
				// goto halt;
			}break;
			case ErrorCode::CannotInstantiateByLibrary:
			{
				std::cout << "cannot be instantiated by the library \"" << moduleId.library << "\"!" << std::endl;
				// goto halt;
			}break;
			case ErrorCode::CannotInstantiate:
			{
				std::cout << "cannot be instantiated!" << std::endl;
				// goto halt;
			}break;
            case ErrorCode::NotALibrary:
            {
                std::cout << "cannot be instantiated because \"" << moduleId.library << "\" is not a Library!" << std::endl;
                // goto halt;
            }break;
			}
		}
        std::cout << std::endl;
		
        //read port connections topology

		//the key is the input port (destination), because a destination can receive only from one source!
		//the mapped value is the output port (source), one source can send to many destinations
		std::map<PortIdentifier, PortIdentifier> connections;
		for (const auto& c : cfg.GetSection("connections"))
		{
            const auto parts = split_to_argv(c);
            if (parts.size() < 2)
            {
                std::cout << "Invalid connection:\"" << c << "\" !" << std::endl;
                continue;
            }

			const auto fromModuleId = PortIdentifier(parts[0]);
			const auto toModuleId = PortIdentifier(parts[1]);
            std::string limitstr;
            size_t limit;
            aq::LimitBehavior behavior = queueBehavior;

            if (parts.size() >= 3)
            {
                behavior = GetBehavior(parts[2].substr(0, parts[2].find('@')));
                if (parts[2].find('@') != std::string::npos)
                    limitstr = parts[2].substr(parts[2].find('@') + 1);
                if (parts.size() >= 4)
                    limitstr = parts[3];
            }
            std::istringstream iss(limitstr);
            iss >> limit;
            if (ErrorCode::Success != graph.AddConnection(fromModuleId, toModuleId, behavior, limit))
            {
                std::cout << "Couldn't connect \"" << c << "\" !";
            }
		}
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		goto halt;
	}

	goto no_halt;

halt:
    run = false;
    if (_textThread.joinable())
        _textThread.join();

	return 1;

no_halt:
    run = true;
    aq::Clock timer;
    double startTime = timer.Tock();

    start_text();

    graph.Start();

    while (run && graph.IsRunning()) // Ctrl break can halt it too
    {
        if (hardResetTime > 0 && startTime + hardResetTime <= timer.Tock())
        {
            run = false;
            graph.Stop();
            break;
        }
        std::this_thread::yield();
    }
    run = false;
    graph.Stop();

    if (_textThread.joinable())
        _textThread.join();
    
    return 0;
}

#ifdef _MSC_VER
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

#else
#include <wordexp.h>

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
#endif