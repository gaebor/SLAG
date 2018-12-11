#include <windows.h>

#include <thread>
#include <iostream>
#include <sstream>
#include <fstream>
#include <mutex>

#include "aq/AsyncQueue.h"
#include "ConfigReader.h"

#include "Graph.h"
#include "Identifiers.h"

typedef std::lock_guard<std::mutex> AutoLock;

static volatile bool run;

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

// https://stackoverflow.com/a/3999597/3583290
std::string utf8_encode(const wchar_t* wstr, int len = 0)
{
    std::string result;
    if (len == 0)
        len = (int)wcslen(wstr);
    if (len > 0)
    {
        const int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr, len, NULL, 0, NULL, NULL);
        result.resize(size_needed);
        if (WideCharToMultiByte(CP_UTF8, 0, wstr, len, &result[0], size_needed, NULL, NULL) == 0)
            result.clear();
    }
    return result;
}

std::string utf8_encode(const std::wstring& wstr)
{
    return utf8_encode(wstr.c_str(), (int)wstr.size());
}

std::wstring utf8_decode(const char* str, int len = 0)
{
    std::wstring result;
    if (len == 0)
        len = (int)strlen(str);
    if (len > 0)
    {
        int size_needed = MultiByteToWideChar(CP_UTF8, 0, str, len, NULL, 0);
        result.resize(size_needed);
        MultiByteToWideChar(CP_UTF8, 0, str, len, &result[0], size_needed);
    }
    return result;
}

std::wstring utf8_decode(const std::string& str)
{
    return utf8_decode(str.c_str(), (int)str.size());
}

std::vector<std::string> split_to_argv(const std::string& line)
{
    int size = 0;
    std::vector<std::string> result;
    const std::wstring wstr = utf8_decode(line);
    auto argv = CommandLineToArgvW(wstr.c_str(), &size);

    if (argv)
    {
        for (int i = 0; i < size; ++i)
        {
            result.emplace_back(utf8_encode(argv[i]));
        }
        LocalFree(argv);
    }
    return result;
}

using namespace slag;

//static double _speed = 0.5;
//static std::map<ModuleIdentifier, ModuleTextualData> _texts;
//static std::mutex _mutex;
//static int nameOffset, textWidth = 80;
//static char wait_marker = '~', overhead_marker = '-', load_marker = '#';
//static std::thread _textThread;

void handle_output_text(const ModuleIdentifier& module_id, const char* text, int length)
{

}

void handle_statistics(const ModuleIdentifier& module_id, double cycle, double load, double wait)
{
    
}

void handle_output_image(const ModuleIdentifier& module_id, int w, int h, SlagImageType type, const unsigned char* data);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow)
{
    int argc;
    const auto argv = CommandLineToArgvW(GetCommandLineW(), &argc);

	aq::LimitBehavior queueBehavior = aq::None;
	size_t queueLimit = std::numeric_limits<size_t>::max();
	double hardResetTime = 0.0;
    int loglevel = 1;
    Graph graph;

	if (argc < 2)
	{
		std::wcerr << L"USAGE: " << argv[0] << L" >>config.cfg<<" << std::endl;
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
            const FullModuleIdentifier fullModuleId(moduleName.c_str());

            const ModuleIdentifier moduleId(fullModuleId.module);
            std::cout << "Module \"" << moduleName << "\" ... "; std::cout.flush();

            switch (graph.AddModule(arguments, handle_statistics, statistics2_callback(), handle_output_text, handle_output_image))
            {
            case ErrorCode::Success:
                std::cout << "initialized";
                break;
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
                std::cout << "instantiated by \"" << graph.GetModuleId(moduleId)->library << "\" but cannot initialize!";
                break;
			case ErrorCode::CannotOpen:
			{
				std::cout << "cannot be instantiated because couldn't open \"" << fullModuleId.library << "\"!" << std::endl;
				// goto halt;
			}break;
			case ErrorCode::CannotInstantiateByLibrary:
			{
				std::cout << "cannot be instantiated by the library \"" << fullModuleId.library << "\"!" << std::endl;
				// goto halt;
			}break;
			case ErrorCode::CannotInstantiate:
			{
				std::cout << "cannot be instantiated!" << std::endl;
				// goto halt;
			}break;
            case ErrorCode::NotALibrary:
            {
                std::cout << "cannot be instantiated because \"" << fullModuleId.library << "\" is not a Library!" << std::endl;
                // goto halt;
            }break;
			}
            std::cout << std::endl;
		}
        std::cout << std::endl;
		
        //read port connections
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
                std::cout << "Couldn't connect \"" << c << "\" !" << std::endl;
            }
		}
        std::cout << std::endl;
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		goto halt;
	}

    graph.Start();

    graph.Wait();

    return 0;
halt:
    return 1;
}
