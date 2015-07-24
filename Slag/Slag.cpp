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

int main(int argc, char* argv[])
{

	Factory factory;
	ModuleWrapper* module_array;
	size_t moduleNumber = 0;
	MessageQueue* messageQueue_array;
	size_t messageQueueNumber = 0;

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

	std::map<ModuleIdentifier, ModuleWrapper*> modules;
	moduleNumber = fs["Modules"].size();
	module_array = new ModuleWrapper[moduleNumber]();
	auto module_ptr = module_array;

	for (auto node : fs["Modules"])
	{
		std::string moduleName = node["Name"];
		auto it = modules.insert(std::make_pair(ModuleIdentifier(moduleName.c_str()), module_ptr++));
		if (!it.second)
		{
			std::cerr << "Module \"" << moduleName << "\" appears more than once in module list!" << std::endl;
			return 0;
		}

		auto moduleWrapper = it.first->second;
		if (!moduleWrapper->Initialize(node, factory))
		{
			std::cerr << "Couldn't initialize module \"" << moduleName << "\"!" << std::endl;
			return -1;
		}	
	}

	//read port connections topology
	std::map<ModuleIdentifier, std::set<PortNumber>> inputPorts;
	
	for (auto node : fs["Modules"])
	{
		std::string moduleName = node["Name"];
		ModuleIdentifier moduleId(moduleName.c_str());
		for (auto& synced : node["Outputs"])
		{
			std::string idStr(synced);
			PortIdentifier portId(idStr.c_str());
			auto moduleIt = modules.find(portId.module);
			if (moduleIt != modules.end())
			{
				//inputPorts[portId.module].insert(portId.port);
				//modules[portId.module]->inputQueues.resize(portId.port+1, nullptr);
				++messageQueueNumber;
			}else
			{
				//skip output port (data goes down the toilet)
			}
		}
	}

	//// check that the input ports are aligned correctly
	//for (const auto& ports : inputPorts)
	//{
	//	const std::string moduleId = ports.first;
	//	int output_port = 0;
	//	for (auto port : ports.second)
	//	{
	//		if (port != output_port)
	//		{
	//			std::cerr << "The " << output_port << "th input port of module \"" << moduleId << "\" is empty!" << std::endl;
	//			return -1;
	//		}
	//		++output_port;
	//	}
	//}

	//allocate input ports
	messageQueue_array = new MessageQueue[messageQueueNumber]();
	auto queuePtr = messageQueue_array;

	//actually connect the output and input ports
	for (auto node : fs["Modules"])
	{
		std::string moduleName = node["Name"];
		ModuleIdentifier moduleId(moduleName.c_str());
		PortNumber portNumber = 0;
		for (auto& synced : node["Outputs"])
		{
			std::string idStr(synced);
			PortIdentifier portId(idStr.c_str());
			if (modules.find(portId.module) != modules.end())
			{
				modules[moduleId]->outputQueues[portNumber] = queuePtr;
				modules[portId.module]->inputQueues[portId.port] = queuePtr;
				++queuePtr;
			}else
			{

			}
			++portNumber;
		}
		modules[moduleId]->outputPortLength = portNumber;
	}

	for (auto m : modules)
	{
		PortNumber i = 0;
		for (auto q : m.second->inputQueues)
			i = std::max(i,q.first+1);
		m.second->inputPortLength = i;
	}

	}catch(std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return -1;
	}

	////
	//while (cv::waitKey(1) != 27)
	//{
	//	for (auto& buffer: buffers)
	//		if (buffer.second.ready())
	//		{
	//			ManegedMessage message;
	//			buffer.second.pop(message);
	//			std::vector<slag::Message* const> assembledMessage(1);
	//			assembledMessage[0] = message.get();

	//			slag::Module* sender = modules[buffer.first.module].get();

	//			auto result = sender->Compute(&assembledMessage[0]);

	//			//TODO result leak

	//			VisualizeResult(sender);
	//		}
	//	//do process

	//	cv::imshow("SLAG", gui);
	//}

	//// stop graph
	//for (auto& source : sources)
	//{
	//	source.second->Stop();

	//}

	for (int i = 0; i < moduleNumber; ++i)
	{
		module_array[i].ThreadProcedure();
	}

	delete [] messageQueue_array;
	delete [] module_array;

	return 0;
}
