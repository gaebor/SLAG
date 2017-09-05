
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
#include "HumanReadable.h"
#include "OS_dependent.h"

bool run = true; //signaling the termination event

int main(int argc, char* argv[])
{
	Factory factory;
	std::list<std::unique_ptr<MessageQueue>> messageQueues;
	std::map<ModuleIdentifier, ModuleWrapper> modules;
	MessageQueue::LimitBehavior queueBehavior = MessageQueue::None;
	size_t queueLimit = std::numeric_limits<size_t>::max();
	double hardResetTime = 0.0;

	std::list<std::string> global_settings;
	std::vector<const char*> global_settings_v;
	size_t max_settings_size = 1024;

	if (argc < 2)
	{
		std::cerr << "USAGE: slag.exe >>config.cfg<<" << std::endl;
		goto halt;
	}
	try{

		std::ifstream f(argv[1]);
		if (!f.good())
		{
			std::cerr << "Cannot read " << argv[1] << "\"!" << std::endl;
			goto halt;
		}
		ConfigReader cfg(f);
		
		//graph settings
		for (auto l : cfg.GetSection("graph"))
		{
			const auto key = l.substr(0, l.find('='));
			const auto value = l.substr(l.find('=') + 1);

			if (key == "QueueBehavior")
			{
				if (value == "Wait")
					queueBehavior = MessageQueue::Wait;
				else if (value == "Drop")
					queueBehavior = MessageQueue::Drop;
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
		//instantiate modules
		for (auto moduleStr : cfg.GetSection("modules"))
		{
			auto arguments = split_to_argv(moduleStr);
			
			std::string moduleName = arguments[0];
			if (moduleName.empty())
			{
				std::cerr << "module should have a non-empty \"Name\"!" << std::endl;
				goto halt;
			}
			arguments.erase(arguments.begin());

			auto insert_result = modules.emplace(moduleName.c_str(), &run);
			auto& moduleWrapper = insert_result.first->second;
			auto& moduleId = moduleWrapper.identifier;
			moduleId.assign(moduleName.c_str());
			moduleName = moduleId;

			std::cout << "Module \"" << moduleName << "\" ... "; std::cout.flush();

			if (!insert_result.second)
			{
				std::cout<< "appears more than once in the graph. Please give it a distinctive name or instance ID!" << std::endl;
				goto halt;
			}

			auto result = factory.InstantiateModule(moduleWrapper);
			moduleName = moduleId;

			switch (result)
			{
			case Factory::Duplicate:
				std::cout << "found more than once ... ";
			case Factory::Success:
			{
				//moduleWrapper.global_settings_c = (int)global_settings.size();
				//moduleWrapper.global_settings_v = global_settings_v.data();

				std::cout << "instantiated by \"" << moduleId.library << "\" ... "; std::cout.flush();

				if (!moduleWrapper.Initialize(arguments))
				{
					std::cout << "cannot be initialized!" << std::endl;
					goto halt;
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
		//read port connections topology

		//the key is the input port (destination), because a destination can receive only from one source!
		//the mapped value is the output port (source), one source can send to many destinations
		std::map<PortIdentifier, PortIdentifier> connections;
		for (auto c : cfg.GetSection("connections"))
		{
			if (c.find("->") == std::string::npos || c.find("->") != c.rfind("->"))
				continue;

			const auto fromModuleId = PortIdentifier(ConfigReader::trim1(c.substr(0, c.find("->"))).c_str());
			const auto toModuleId = PortIdentifier(ConfigReader::trim1(c.substr(c.find("->") + 2)).c_str());
				
			connections[toModuleId] = fromModuleId;
		}

		//purge bad connections
		std::map<PortIdentifier, PortIdentifier> checked_connections;
		for (const auto& connection : connections)
		{
			const auto& toPort = connection.first;
			const auto& fromModule = connection.second;
			if (modules.find(toPort.module) != modules.end())
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
			fromModulePtr->second.outputQueues[fromModule.port].push_back(newMessageQueue);
			toModulePtr->second.inputQueues[toModule.port] = newMessageQueue;

			auto& inputLength = toModulePtr->second.inputPortLength;
			inputLength = std::max(inputLength, toModule.port + (size_t)1);
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

	return -1;

no_halt:
	init_termination_signal(&run, hardResetTime);

	std::thread module_processes([&]()
	{
		std::vector<std::shared_ptr<std::thread>> moduleThreads(modules.size());
		auto threadIt = moduleThreads.begin();
		for (auto& m : modules)
		{
			threadIt->reset(new std::thread([](ModuleWrapper* _m){_m->ThreadProcedure();}, &m.second));
			++threadIt;
		}

		threadIt = moduleThreads.begin();
		for (threadIt = moduleThreads.begin(); threadIt != moduleThreads.end(); ++threadIt)
		{
			(*threadIt)->join();
		}
		run = false;
	});

	wait_termination_signal();

	//if the modules haven't ran empty at this point, then the termination must be a CTRL+C (hard reset)
	for (auto& q : messageQueues)
	{
		q->WakeUp();
	}

	module_processes.join();
	terminate_output_text();
	terminate_output_image();

	return 0;
}
