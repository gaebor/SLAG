#include <windows.h>
#include <iostream>

#include "ConfigReader.h"
#include "Graph.h"
#include "OS_dependent.h"

BOOL WINAPI consoleHandler(DWORD signal) {
    switch (signal)
    {
    case CTRL_C_EVENT:
    case CTRL_BREAK_EVENT:
    case CTRL_CLOSE_EVENT:
        *run = false; //TODO end the Compute for each module
        break;
    default:
        break;
    }

    return TRUE;
}

int main(int argc, char* argv[])
{
    if (!SetConsoleCtrlHandler(consoleHandler, TRUE)) {
        fprintf(stderr, "\nCould not set control handler!\n");
    }
	MessageQueue::LimitBehavior queueBehavior = MessageQueue::None;
	size_t queueLimit = std::numeric_limits<size_t>::max();
	double hardResetTime = 0.0;
    int loglevel = 1;

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
				output_text_argv = split_to_argv(value);
			else if (key == "ImageOutputSettings")
				configure_output_image(split_to_argv(value));
			else if (key == "HardResetTime")
				hardResetTime = atoi(value.c_str());
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
        Graph graph;
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
            const ModuleIdentifier moduleId(arguments[0].c_str());
            const std::string moduleName(moduleId);
            std::cout << "Module \"" << moduleName << "\" ... "; std::cout.flush();

            const auto result = graph.AddModule(arguments);

			switch (result)
			{
			case ErrorCode::Duplicate:
				std::cout << "found more than once ... ";
			case ErrorCode::Success:
			{
				std::cout << "instantiated by \"" << result.first->GetIdentifier().library << "\" ... ";
                std::cout.flush();

				if (!modules[moduleId]->Initialize(arguments))
				{
					std::cout << "cannot be initialized!" << std::endl;
                    // modules.erase(moduleId);
				}else
					std::cout << "initialized" << std::endl;
			}break;
			case ErrorCode::CannotOpen:
			{
				std::cout << "cannot be instantiated because couldn't open \"" << moduleId.library << "\"!" << std::endl;
				goto halt;
			}break;
			case ErrorCode::CannotInstantiateByLibrary:
			{
				std::cout << "cannot be instantiated by the library \"" << moduleId.library << "\"!" << std::endl;
				goto halt;
			}break;
			case ErrorCode::CannotInstantiate:
			{
				std::cout << "cannot be instantiated!" << std::endl;
				goto halt;
			}break;
            case ErrorCode::NotALibrary:
            {
                std::cout << "cannot be instantiated because \"" << moduleId.library << "\" is not a Library!" << std::endl;
                goto halt;
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
				
            if (modules.find(fromModuleId.module) == modules.end())
            {
                std::cerr << "Cannot connect !" << (std::string)fromModuleId << "! -> \"" <<
                    (std::string)toModuleId << "\"" << std::endl;
                continue;
            }
            if (modules.find(toModuleId.module) == modules.end())
            {
                std::cerr << "Cannot connect \"" << (std::string)fromModuleId << "\" -> !" <<
                    (std::string)toModuleId << "!" << std::endl;
                continue;
            }
			connections[toModuleId] = fromModuleId;
            //automatically overrides duplicate inputs
		}

		//actually connect the output and input ports
		for (auto connection : connections)
		{
			const auto& toModuleId = connection.first;
			const auto& fromModuleId = connection.second;
            
			messageQueues.emplace_back(new MessageQueue());
			auto newMessageQueue = messageQueues.back().get();
			newMessageQueue->limitBehavior = queueBehavior;
			newMessageQueue->queueLimit = queueLimit;

			auto fromModulePtr = modules.find(fromModuleId.module);
			auto toModulePtr = modules.find(toModuleId.module);

            const auto ok1 = toModulePtr->second->ConnectToInputPort(toModuleId.port, newMessageQueue);
            if (ok1)
            {
                const auto ok2 = fromModulePtr->second->ConnectOutputPortTo(fromModuleId.port, newMessageQueue);
                if (ok2)
                {
                    std::cout << "Connected \"" << (std::string)fromModuleId << "\" -> \"" << (std::string)toModuleId << "\"" << std::endl;
                }else
                    toModulePtr->second->RemoveInputPort(toModuleId.port);
            }
		}
        std::cout << std::endl;
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

	return -1;

no_halt:

    std::cerr << "init_termination_signal" << std::endl;
    init_termination_signal(&run, hardResetTime);	
    std::cerr << "configure_output_text" << std::endl;
    configure_output_text(output_text_argv);
    std::cerr << "start modules" << std::endl;
	std::thread modules_process([&]()
	{
        for (auto& m : modules)
            m.second->Start();

        for (auto& m : modules)
            m.second->Wait();
		
        // if this point is reached the modules ran out of jobs and halted naturally
        run = false;
	});
    std::cerr << "wait_termination_signal" << std::endl;
	wait_termination_signal();
    std::cerr << "tell modules to stop" << std::endl;
    // modules stop processing (may have unprocessed inputs in the queues)
    for (auto& m : modules)
    {
        m.second->Stop();
    }
    std::cerr << "wakeup queues" << std::endl;
	//if the modules haven't ran empty at this point, then the termination must be a CTRL+C (hard reset)
	for (auto& q : messageQueues)
	{
		q->WakeUp();
	}
    std::cerr << "stop modules" << std::endl;
	modules_process.join();
    std::cerr << "terminate_output_text" << std::endl;
	terminate_output_text();
    std::cerr << "terminate_output_image" << std::endl;
	terminate_output_image();
    std::cerr << "empty queues" << std::endl;
    messageQueues.clear();
    std::cerr << "destroy modules" << std::endl;
    modules.clear();
	return 0;
}
