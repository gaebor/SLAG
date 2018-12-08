#include <iostream>
#include <sstream>
#include <fstream>

#include "ConfigReader.h"
#include "Graph.h"
#include "Additionals.h"

#include <signal.h>

#include "aq/Clock.h"

static volatile bool run;

void new_handler(int signum)
{
	run = false;
}

int main(int argc, char* argv[])
{
    for (auto sgn : { SIGINT , SIGHUP , SIGTERM , SIGQUIT })
	{
		if (signal(sgn, new_handler) == SIG_IGN)
			signal(sgn, SIG_IGN);
	}
    
	MessageQueue::LimitBehavior queueBehavior = MessageQueue::None;
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
			{
				if (value == "Wait")
					queueBehavior = MessageQueue::Wait;
				else if (value == "Drop")
					queueBehavior = MessageQueue::Drop;
                else if (value == "Refuse")
                    queueBehavior = MessageQueue::Refuse;
				else
					queueBehavior = MessageQueue::None;
			}
			else if (key == "QueueLimit")
				queueLimit = atoi(value.c_str());
			else if (key == "TextOutputSettings")
                configure_output_text(split_to_argv(value));
			else if (key == "ImageOutputSettings")
				configure_output_image(split_to_argv(value));
			else if (key == "HardResetTime")
				std::istringstream(value) >> hardResetTime;
		}
		////global settings
		//for (auto l : cfg.GetSection("global"))
		//{
		//	const auto pos = l.find('=');
		//	const auto key = l.substr(0U, pos);
		//	const auto value = l.substr(pos == std::string::npos ? l.size() : l.find('=') + 1);
		//	global_settings.push_back(key); global_settings.push_back(value);
		//	max_settings_size = std::max(max_settings_size, value.size());
		//}
        
        //{
        //    factory.Scan();
        //    std::cout << "Scanned Libraries: " << std::endl;
        //    auto foundLibs = factory.GetLibraries();
        //    for (auto& lib : foundLibs)
        //        std::cout << lib << std::endl;
        //    std::cout << std::endl;
        //}

		//instantiate modules
		for (auto moduleStr : cfg.GetSection("modules"))
		{
			const auto arguments = split_to_argv(moduleStr);
            if (arguments.empty())
                continue;
            const std::string moduleName = arguments[0];
            std::cout << "Module \"" << moduleName << "\" ... "; std::cout.flush();
            const FullModuleIdentifier moduleId(moduleName.c_str());

            switch (graph.AddModule(arguments, handle_statistics, handle_output_text, handle_output_image))
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
		for (auto c : cfg.GetSection("connections"))
		{
			if (c.find("->") == std::string::npos || c.find("->") != c.rfind("->"))
				continue;

			const auto fromModuleId = PortIdentifier(ConfigReader::trim1(c.substr(0, c.find("->"))));
			const auto toModuleId = PortIdentifier(ConfigReader::trim1(c.substr(c.find("->") + 2)));

            if (ErrorCode::Success != graph.AddConnection(fromModuleId, toModuleId, queueBehavior, queueLimit))
            {
                std::cout << "Couldn't connect " << (std::string)fromModuleId << " -> " << (std::string)toModuleId << " !";
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
	terminate_output_text();
	terminate_output_image();

	return 1;

no_halt:
    run = true;
    aq::Clock timer;
    double startTime = timer.Tock();

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

    graph.Wait();
    graph.Stop();

    terminate_output_text();
	terminate_output_image();
    
    return 0;
}
