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

bool run = true; //signaling the visualizer

static inline int round_int( double x )
{
	return x+0.5;
}

void visualizer(std::map<ModuleIdentifier, std::shared_ptr<ModuleWrapper>>& modules, int speed = 0)
{
	std::string nameTag = "  module  ";

	size_t nameOffset = nameTag.size();
	for (auto& m : modules)
	{
		nameOffset = std::max(nameOffset, std::string(m.first).size());
	}

	std::vector<std::string> names;
	for (auto& m : modules)
	{
		auto name = (std::string)m.first;
		name += std::string(nameOffset - name.size(), ' ');
		names.push_back( name );
	}

	auto internal_func = [&]()
	{
		system("cls");

		printf("%s", nameTag.c_str());
		for (auto i = nameTag.size(); i < nameOffset; ++i)
			putchar(' ');
		printf("|  speed   | overhead | text output\n");
		for (int i = 0; i < nameOffset; ++i)
			putchar('-');
		printf("+----------+----------+-\n");

		auto nameIt = names.begin();
		for (auto& m : modules)
		{
			char line[1024];
			m.second->diffTime.NonEditable();
			print_humanreadable_time(line, 1024, m.second->diffTime.Get().first);
			const int load = round_int(10*(m.second->diffTime.Get().second)/(m.second->diffTime.Get().first));
			m.second->diffTime.MakeEditable();
			printf("%s|%s|", nameIt->c_str(), line);
			for (int i = 0; i < 10-load; ++i)
				putchar('=');
			for (int i = 10-load; i < 10; ++i)
				putchar(' ');
			putchar('|');

			m.second->output_text.NonEditable();
			std::cout << m.second->output_text.Get() << "\n";
			m.second->output_text.Set() = "";
			m.second->output_text.MakeEditable();

			m.second->bufferSize.NonEditable();
			for (auto& q : m.second->bufferSize.Get())
			{
				int offset = print_humanreadable_giga(line, 1024, q.second, " ");
				for (; offset < nameOffset;++offset)
					putchar(' ');
				printf("%s|> %d\n", line, q.first);
			}
			m.second->bufferSize.MakeEditable();
			for (int i = 0; i < nameOffset; ++i)
				putchar('-');
			printf("+----------+----------+-\n");

			/************************************************************************/
			/* image output                                                         */
			/************************************************************************/
			
			m.second->output_image.NonEditable();
			if (!m.second->output_image.Get().data.empty())
				Imshow(nameIt->c_str(), m.second->output_image.Get());
			m.second->output_image.MakeEditable();
			++nameIt;
		}
		FeedImshow();
	};

	while (run)
	{
		internal_func();
		Sleep(speed);
	}
	internal_func();
}

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
	std::map<ModuleIdentifier, std::shared_ptr<ModuleWrapper>> modules;
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
	}

	std::map<std::string, ModuleIdentifier> moduleIdentifiers;

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
		std::shared_ptr<ModuleWrapper> moduleWrapper(new ModuleWrapper(&run));
		moduleWrapper->identifier = ModuleIdentifier(moduleName.c_str());
		auto& moduleId = moduleWrapper->identifier;

		auto result = factory.InstantiateModule(*moduleWrapper);

		switch (result)
		{
		case Factory::Duplicate:
			std::cerr << "More than one library can instantiate module \"" << (std::string)(moduleId) << "\", the one in \"" << moduleId.actual_dll << "\" will be used!" << std::endl;
		case Factory::Success:
			{
			auto it = modules.find(moduleId);
			if (it != modules.end())
			{
				std::cerr << "Module \"" << (std::string)moduleId << "\" from library \"" << moduleId.actual_dll << "\" has been loaded more than once!" << std::endl;
				return 0;
			}
			modules[moduleId] = moduleWrapper;
			modules[moduleId]->global_settings_c = global_settings.size();
			modules[moduleId]->global_settings_v = global_settings_v.data();

			if (!(modules[moduleId]->Initialize(node)))
			{
				std::cerr << "Module \"" << (std::string)moduleId << "\" cannot be initialized!" << std::endl;
				return 0;
			}
			moduleIdentifiers[moduleName] = moduleId;
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
	std::map<std::string,PortIdentifier> connections;
	
	for (auto node : fs["Modules"])
	{
		const auto& fromModuleId = moduleIdentifiers[node["Name"]];
		auto outputNode = node["Outputs"];

		if (outputNode.isString())
		{
			connections.insert(std::pair<std::string, PortIdentifier>(outputNode, PortIdentifier(fromModuleId,0)));
		}
		else if (outputNode.isSeq())
		{
			PortNumber fromPort = 0;
			for (auto& output : outputNode)
			{
				connections.insert(std::make_pair(output, PortIdentifier(fromModuleId,fromPort)));
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
			PortIdentifier portId(std::string(output).c_str());
			if (output.isSeq())
			{
				for (auto& port : output)
				{
					connections.insert(std::make_pair(port, PortIdentifier(fromModuleId,fromPort)));
				}
			}else
				connections.insert(std::make_pair(output, PortIdentifier(fromModuleId,fromPort)));
		}
	}

	//purge bad connections
	std::map<PortIdentifier, PortIdentifier> checked_connections;
	for (const auto& connection : connections)
	{
		const auto colon_pos = connection.first.find(':');
		const auto toModuleStr = connection.first.substr(0,colon_pos);
		const auto toPort = colon_pos == std::string::npos ? 0 : atoi(connection.first.substr(colon_pos+1).c_str());
		if (moduleIdentifiers.find(toModuleStr) != moduleIdentifiers.end())
			checked_connections.emplace(PortIdentifier(moduleIdentifiers[toModuleStr], toPort),connection.second);
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

		modules[fromModule.module]->outputQueues[fromModule.port].push_back(newMessageQueue);
		modules[toModule.module]->inputQueues[toModule.port] = newMessageQueue;

		auto& inputLength = modules[toModule.module]->inputPortLength;
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

	std::thread module_processes([&]()
	{
		std::vector<std::shared_ptr<std::thread>> moduleThreads(modules.size());
		auto threadIt = moduleThreads.begin();
		for (auto m : modules)
		{
			threadIt->reset(new std::thread([](ModuleWrapper* _m){_m->ThreadProcedure();}, m.second.get()));
			++threadIt;
		}

		threadIt = moduleThreads.begin();
		for (threadIt = moduleThreads.begin(); threadIt != moduleThreads.end(); ++threadIt)
		{
			(*threadIt)->join();
		}
		run = false;
	});

	visualizer(modules, vizualizationSpeed);

	module_processes.join();

	return 0;
}
