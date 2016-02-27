
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
#include "AsyncQueue.h"
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
	int vizualizationSpeed = 500;
	double hardResetTime = 0.0;

	std::vector<std::string> global_settings;
	std::vector<const char*> global_settings_v;

	if (argc < 2)
	{
		std::cerr << "USAGE: slag.exe >>config.cfg<<" << std::endl;
		goto halt;
	}
	try{

		ConfigReader cfg(argv[1]);
		
		//graph settings
		for (auto l : cfg.GetSection("graph"))
		{
			const auto key = l.substr(0U,l.find('='));
			const auto value = l.substr(l.find('=')+1);

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
			else if (key == "VisualizationDelay")
				vizualizationSpeed = atoi(value.c_str());
			else if (key == "HardResetTime")
				hardResetTime = atoi(value.c_str());
		}
		//global settings
		for (auto l : cfg.GetSection("global"))
		{
			const auto key = l.substr(0U, l.find('='));
			const auto value = l.substr(l.find('=') + 1);
			global_settings.push_back(key); global_settings.push_back(value);
		}
		for (auto& setting : global_settings)
		{
			if (setting.size() >= SETTINGS_MAX_LENGTH)
			{
				std::cerr << "Global setting >>>\n" << setting << "\n<<< shouldn't exceed the length of " << SETTINGS_MAX_LENGTH << "!" << std::endl;
				goto halt;
			}
			//be careful, from now on, settings may change, but the NUMBER of settings doesn't!
			//also no settings can be longer than this reserved length to ensure iterator validity
			setting.reserve(SETTINGS_MAX_LENGTH);

			global_settings_v.push_back(setting.c_str());
		}
		//instantiate modules
		for (auto moduleStr : cfg.GetSection("modules"))
		{
			auto arguments = split_to_argv(moduleStr);
			
			const std::string moduleName = arguments[0];
			if (moduleName.empty())
			{
				std::cerr << "module should have a non-empty \"Name\"!" << std::endl;
				goto halt;
			}
			arguments.erase(arguments.begin());

			auto insert_result = modules.emplace(moduleName.c_str(), &run);
			auto& moduleId = insert_result.first->first;

			if (!insert_result.second)
			{
				std::cerr << "Module \"" << (std::string)moduleId << "\" appears more than one in the graph. Please give it a distinctive name or instance ID!" << std::endl;
				goto halt;
			}
			auto& moduleWrapper = insert_result.first->second;
			moduleWrapper.identifier = moduleId;
			auto result = factory.InstantiateModule(moduleWrapper);

			switch (result)
			{
			case Factory::Duplicate:
				std::cerr << "More than one library can instantiate module \"" << (std::string)(moduleId) << "\", the one in \"" << moduleId.library << "\" will be used!" << std::endl;
			case Factory::Success:
			{
				moduleWrapper.global_settings_c = global_settings.size();
				moduleWrapper.global_settings_v = global_settings_v.data();

				if (!moduleWrapper.Initialize(arguments))
				{
					std::cerr << "Module \"" << (std::string)moduleId << "\" cannot be initialized!" << std::endl;
					goto halt;
				}
			}break;
			case Factory::NoSuchLibrary:
			{
				std::cerr << "No library \"" << moduleId.library << "\" to instantiate Module \"" << (std::string)moduleId << "\"!" << std::endl;
				goto halt;
			}break;
			case Factory::CannotInstantiateByLibrary:
			{
				std::cerr << "The library \"" << moduleId.library << "\" cannot instantiate Module \"" << (std::string)moduleId << "\"!" << std::endl;
				goto halt;
			}break;
			case Factory::CannotInstantiate:
			{
				std::cerr << "Module \"" << (std::string)moduleId << "\" cannot be instantiated!" << std::endl;
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

			//auto outputNode = node->createView("Outputs");
			//Poco::Util::AbstractConfiguration::Keys output_keys;
			//outputNode->keys(output_keys);
			//if (output_keys.size() == 0)
			//{
			//	const std::string outputNodeStr = node->getString("Outputs");
			//	const auto outputPortId = PortIdentifier(outputNodeStr.c_str());

			//	connections[outputPortId] = PortIdentifier(fromModuleId, 0);
			//}
			//else if (outputNode.isSeq())
			//{
			//	PortNumber fromPort = 0;
			//	for (auto& output : outputNode)
			//	{
			//		const std::string outputNodeStr = output;
			//		const auto outputPortId = PortIdentifier(outputNodeStr.c_str());

			//		connections[outputPortId] = PortIdentifier(fromModuleId, fromPort);
			//		++fromPort;
			//	}
			//}
			//else if (outputNode.isMap())
			//	for (auto& output : outputNode)
			//	{
			//		std::string outputNodeName = output.name();
			//		if (outputNodeName[0] != '_')
			//		{
			//			std::cerr << "output port number should be preceded by underscore in \"" << (std::string)fromModuleId << std::endl;
			//			goto halt;
			//		}
			//		PortNumber fromPort = atoi(outputNodeName.substr(1).c_str());
			//		if (output.isSeq())
			//		{
			//			for (auto& port : output)
			//			{
			//				PortIdentifier outputPortId(std::string(port).c_str());
			//				connections[outputPortId] = PortIdentifier(fromModuleId, fromPort);
			//			}
			//		}
			//		else
			//		{
			//			PortIdentifier outputPortId(std::string(output).c_str());
			//			connections[outputPortId] = PortIdentifier(fromModuleId, fromPort);
			//		}
			//	}
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

	set_output_text_speed(vizualizationSpeed);

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
