
#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <list>
#include <memory>
#include <mutex>
#include <set>

#include "ConfigReader.h"
#include "Factory.h"
#include "ModuleIdentifier.h"
#include "aq/AsyncQueue.h"
#include "ModuleWrapper.h"
#include "OS_dependent.h"

bool run = true; //signaling the termination event

int main(int argc, char* argv[])
{
    Factory factory;

	std::list<std::unique_ptr<MessageQueue>> messageQueues;
	std::map<ModuleIdentifier, std::shared_ptr<ModuleWrapper>> modules;
	MessageQueue::LimitBehavior queueBehavior = MessageQueue::None;
	size_t queueLimit = std::numeric_limits<size_t>::max();
	double hardResetTime = 0.0;
    int loglevel = 1;

	std::list<std::string> global_settings;
	std::vector<const char*> global_settings_v;
	size_t max_settings_size = 1024;

	std::vector<std::string> output_text_argv;

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
		//global settings
		for (auto l : cfg.GetSection("global"))
		{
			const auto pos = l.find('=');
			const auto key = l.substr(0U, pos);
			const auto value = l.substr(pos == std::string::npos ? l.size() : l.find('=') + 1);
			global_settings.push_back(key); global_settings.push_back(value);
			max_settings_size = std::max(max_settings_size, value.size());
		}
		//TODO
		//for (auto& setting : global_settings)
		//{
		//	
		//	setting.reserve(SETTINGS_MAX_LENGTH);

		//	global_settings_v.push_back(setting.c_str());
		//}

        factory.Scan();

		//instantiate modules
		for (auto moduleStr : cfg.GetSection("modules"))
		{
			auto arguments = split_to_argv(moduleStr);
			
			std::string moduleName = arguments[0];
			if (moduleName.empty())
			{
				std::cerr << "module should have a non-empty \"Name\"!" << std::endl;
                continue;
			}
			arguments.erase(arguments.begin());

			//auto insert_result = modules.emplace(moduleName.c_str(), &run);
			//auto& moduleWrapper = insert_result.first->second;
			//auto& moduleId = moduleWrapper.identifier;
			//moduleId.assign(moduleName.c_str());
			//moduleName = moduleId;
            const ModuleIdentifier moduleId(moduleName.c_str());

			std::cout << "Module \"" << moduleName << "\" ... "; std::cout.flush();

			if (modules.find(moduleId) != modules.end())
			{
				std::cout<< "appears more than once in the graph. Please give it a distinctive name or instance ID!" << std::endl;
                continue;
			}

			auto result = factory.InstantiateModule(moduleId);
			moduleName = moduleId;

			switch (result.second)
			{
			case Factory::Duplicate:
				std::cout << "found more than once ... ";
			case Factory::Success:
			{
                modules[moduleId].reset(result.first);
				//moduleWrapper.global_settings_c = (int)global_settings.size();
				//moduleWrapper.global_settings_v = global_settings_v.data();

				std::cout << "instantiated by \"" << result.first->identifier.library << "\" ... ";
                std::cout.flush();

				if (!modules[moduleId]->Initialize(arguments))
				{
					std::cout << "cannot be initialized!" << std::endl;
                    modules.erase(moduleId);
				}else
					std::cout << "initialized" << std::endl;
			}break;
			case Factory::NoSuchLibrary:
			{
				std::cout << "cannot be instantiated because there is no library \"" << moduleId.library << "\"!" << std::endl;
				goto halt;
			}break;
			case Factory::CannotInstantiateByLibrary:
			{
				std::cout << "cannot be instantiated by the library \"" << moduleId.library << "\"!" << std::endl;
				goto halt;
			}break;
			case Factory::CannotInstantiate:
			{
				std::cout << "cannot be instantiated!" << std::endl;
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
				
			connections[toModuleId] = fromModuleId;
		}

		//purge bad connections
		std::map<PortIdentifier, PortIdentifier> checked_connections;
		for (const auto& connection : connections)
		{
			const auto& toPort = connection.first;
			const auto& fromModule = connection.second;
            if (modules.find(fromModule.module) == modules.end())
            {
                std::cerr << "Cannot connect !" << (std::string)fromModule << "! -> \"" <<
                    (std::string)toPort << "\"" << std::endl;
                continue;
            }
            if (modules.find(toPort.module) == modules.end())
            {
                std::cerr << "Cannot connect \"" << (std::string)fromModule << "\" -> !" <<
                    (std::string)toPort << "!" << std::endl;
                continue;
            }
            checked_connections[toPort] = fromModule;
			//automatically overrides duplicate inputs
		}

		//actually connect the output and input ports
		for (auto connection : checked_connections)
		{
			const auto& toModule = connection.first;
			const auto& fromModule = connection.second;

			messageQueues.emplace_back(new MessageQueue());
			auto newMessageQueue = messageQueues.back().get();
			newMessageQueue->limitBehavior = queueBehavior;
			newMessageQueue->queueLimit = queueLimit;

			auto fromModulePtr = modules.find(fromModule.module);
			auto toModulePtr = modules.find(toModule.module);
			fromModulePtr->second->outputQueues[fromModule.port].push_back(newMessageQueue);
			toModulePtr->second->inputQueues[toModule.port] = newMessageQueue;

			auto& inputLength = toModulePtr->second->inputPortLength;
			inputLength = std::max(inputLength, toModule.port + (size_t)1);

            std::cout << "Connected \"" << (std::string)fromModule << "\" -> \"" << (std::string)toModule << "\"" << std::endl;
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

    std::cout << "init_termination_signal ";
    init_termination_signal(&run, hardResetTime);	
    std::cout << "OK\nconfigure_output_text ";
    configure_output_text(output_text_argv);
    std::cout << "OK\nstart modules ";
	std::thread module_processes([&]()
	{
        for (auto& m : modules)
            m.second->Start();

        for (auto& m : modules)
            m.second->Wait();
		
        // if this point is reached the modules ran out of jobs and halted naturally
        run = false;
	});
    std::cout << "OK\nwait_termination_signal ";
	wait_termination_signal();
    std::cout << "OK\ntell modules to stop ";
    // modules stop processing (may have unprocessed inputs in the queues)
    for (auto& m : modules)
    {
        m.second->do_run = false;
    }
    std::cout << "OK\nwakeup queues ";
	//if the modules haven't ran empty at this point, then the termination must be a CTRL+C (hard reset)
	for (auto& q : messageQueues)
	{
		q->WakeUp();
	}
    std::cout << "OK\nstop modules ";
	module_processes.join();
    std::cout << "OK\nterminate_output_text ";
	terminate_output_text();
    std::cout << "OK\nterminate_output_image ";
	terminate_output_image();
    std::cout << "OK\nempty queues\n";
	return 0;
}
