#include <windows.h>

#include <iostream>
#include <vector>
#include <thread>
#include <list>
#include <memory>
#include <mutex>
#include <set>

#include "Factory.h"
#include "ModuleIdentifier.h"
#include "AsyncQueue.h"
#include "ModuleWrapper.h"
#include "HumanReadable.h"
#include "Imshow.h"
#include "OS_dependent.h"

bool run = true; //signaling the termination event
double hardResetTime = 0.0;

BOOL WINAPI consoleHandler(DWORD signal) {
	switch (signal)
	{
	case CTRL_C_EVENT:
	case CTRL_BREAK_EVENT:
	case CTRL_CLOSE_EVENT:
		run = false; //TODO end the Compute for each module
		break;
	default:
		break;
	}

	return TRUE;
}

int main(int argc, char* argv[])
{
	Factory factory;
	std::list<std::unique_ptr<MessageQueue>> messageQueues;
	std::map<ModuleIdentifier, ModuleWrapper> modules;
	MessageQueue::LimitBehavior queueBehavior = MessageQueue::None;
	size_t queueLimit = std::numeric_limits<size_t>::max();
	int vizualizationSpeed;
	
	std::vector<std::string> global_settings;
	std::vector<const char*> global_settings_v;

	if (argc < 2)
	{
		std::cerr << "USAGE: slag.exe >>process_graph.xml<<" << std::endl;
		return 0;
	}
	try{
	cv::FileStorage fs(argv[1], cv::FileStorage::READ);
	if ( !fs.isOpened() )
	{
		std::cerr << "Cannot read \"" << argv[1] << '\"' << std::endl;
		return 0;
	}

	//graph settings
	for (auto setting : fs["Graph"])
	{
		if (setting.name() == "QueueBehavior")
		{
			if (setting.operator std::string() == "Wait")
				queueBehavior = MessageQueue::Wait;
			else if (setting.operator std::string() == "Drop")
				queueBehavior = MessageQueue::Drop;
			else
				queueBehavior = MessageQueue::None;
		}else if (setting.name() == "QueueLimit")
			queueLimit = setting.operator int();
		else if (setting.name() == "VisualizationDelay")
			vizualizationSpeed = setting.operator int();
		else if (setting.name() == "HardResetTime")
			hardResetTime = setting.operator double();
	}

	//global settings
	global_settings = Factory::ReadSettings(fs["GlobalSettings"]);
	for (auto& setting : global_settings)
	{
		if (setting.size() >= SETTINGS_MAX_LENGTH)
		{
			std::cerr << "Global setting >>>\n" << setting << "\n<<< shouldn't exceed the length of " << SETTINGS_MAX_LENGTH << "!" << std::endl;
			return -1;
		}
		//be careful, from now on, settings may change, but the NUMBER of settings doesn't!
		//also no settings can be longer than this reserved length to ensure iterator validity
		setting.reserve(SETTINGS_MAX_LENGTH);

		global_settings_v.push_back(setting.c_str());
	}

	//instantiate modules
	if (fs["Modules"].isSeq()) //if there is no such node, then an empty graph will be constructed
	for (auto node : fs["Modules"])
	{
		std::string moduleName = node["Name"];
		if (moduleName.empty())
		{
			std::cerr << "module should have a non-empty \"Name\"!" << std::endl;
			return -1;
		}
		auto insret_result = modules.emplace(moduleName.c_str(), &run);
		auto& moduleId = insret_result.first->first;

		if (!insret_result.second)
		{
			std::cerr << "Module \"" << (std::string)moduleId << "\" appears more than one in the graph. Please give it a disjoint name or instance!" << std::endl;
			return -1;
		}
		auto& moduleWrapper = insret_result.first->second;
		moduleWrapper.identifier = moduleId;
		auto result = factory.InstantiateModule(moduleWrapper);

		switch (result)
		{
		case Factory::Duplicate:
			std::cerr << "More than one library can instantiate module \"" << (std::string)(moduleId) << "\", the one in \"" << moduleId.dll << "\" will be used!" << std::endl;
		case Factory::Success:
			{
			moduleWrapper.global_settings_c = global_settings.size();
			moduleWrapper.global_settings_v = global_settings_v.data();

			if (!moduleWrapper.Initialize(node))
			{
				std::cerr << "Module \"" << (std::string)moduleId << "\" cannot be initialized!" << std::endl;
				return 0;
			}
			}break;
		case Factory::NoSuchLibrary:
			{
				std::cerr << "No library \"" << moduleId.dll << "\" to instantiate Module \"" << (std::string)moduleId << "\"!" << std::endl;
				return -1;
			}break;
		case Factory::CannotInstantiateByLibrary:
			{
				std::cerr << "The library \"" << moduleId.dll << "\" cannot instantiate Module \"" << (std::string)moduleId << "\"!" << std::endl;
				return -1;
			}break;
		case Factory::CannotInstantiate:
			{
				std::cerr << "Module \"" << (std::string)moduleId << "\" cannot be instantiated!" << std::endl;
				return -1;
			}break;
		}
	}

	//read port connections topology

	//the key is the input port (destination), because a destination can receive only from one source!
	//the mapped value is the output port (source), one source can send to many destinations
	std::map<PortIdentifier,PortIdentifier> connections;
	
	for (auto node : fs["Modules"])
	{
		const std::string fromModuleStr = node["Name"];
		const auto fromModuleId = ModuleIdentifier(fromModuleStr.c_str());
		auto outputNode = node["Outputs"];
		
		if (outputNode.isString())
		{
			const std::string outputNodeStr = outputNode;
			const auto outputPortId = PortIdentifier(outputNodeStr.c_str());

			connections[outputPortId] = PortIdentifier(fromModuleId,0);
		}
		else if (outputNode.isSeq())
		{
			PortNumber fromPort = 0;
			for (auto& output : outputNode)
			{
				const std::string outputNodeStr = output;
				const auto outputPortId = PortIdentifier(outputNodeStr.c_str());

				connections[outputPortId] = PortIdentifier(fromModuleId,fromPort);
				++fromPort;
			}
		}else if (outputNode.isMap())
		for (auto& output : outputNode)
		{
			std::string outputNodeName = output.name();
			if (outputNodeName[0] != '_')
			{
				std::cerr << "output port number should be preceded by underscore in \"" << (std::string)fromModuleId <<  std::endl;
				return -1;
 			}
			PortNumber fromPort = atoi(outputNodeName.substr(1).c_str());
			if (output.isSeq())
			{
				for (auto& port : output)
				{
					PortIdentifier outputPortId(std::string(port).c_str());
					connections[outputPortId] = PortIdentifier(fromModuleId,fromPort);
				}
			}else
			{
				PortIdentifier outputPortId(std::string(output).c_str());
				connections[outputPortId] = PortIdentifier(fromModuleId,fromPort);
			}
		}
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
		inputLength = std::max(inputLength, toModule.port+(size_t)1);
	}
	}catch(std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return -1;
	}

	run = true;
	//! register console termination handler
	if (!SetConsoleCtrlHandler(consoleHandler, TRUE)) {
		printf("\nERROR: Could not set control handler"); 
		return 1;
	}

	Timer timer;
	const auto startTime = timer.Tock();
	std::thread resetTimer([&]()
	{
		if (hardResetTime > 0)
		{
			while (run)
			{
				if (startTime + hardResetTime <= timer.Tock())
					run = false;
			}
		}
	});

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

	///************************************************************************/
	///* image output                                                         */
	///************************************************************************/
	std::vector<std::string> names;
	for (auto& m : modules)
		names.push_back((std::string)m.first);

	while(run)
	{
		auto nameIt = names.begin();
		for (auto& m : modules)
		{
			m.second.output_image.Modify([&](const ImageContainer& self)
			{
				if (!self.data.empty())
					Imshow(nameIt->c_str(), self);
			});
			++nameIt;
		}
		FeedImshow();
	}

	//if the modules haven't ran empty at this point, then the termination must be a CTRL+C (hard reset)
	for (auto& q : messageQueues)
	{
		q->WakeUp();
	}

	module_processes.join();
	terminate_output_text();
	resetTimer.join();

	return 0;
}
