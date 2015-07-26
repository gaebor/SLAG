#pragma once

#include <vector>
#include <string>
#include <memory>

#include "slag/slag_interface.h"
#include "AsyncQueue.h"
#include "opencv2/core/core.hpp"
#include "ModuleIdentifier.h"
#include "Factory.h"

typedef std::shared_ptr<slag::Message> ManagedMessage;
typedef AsyncQueue<ManagedMessage> MessageQueue;

class ModuleWrapper
{
public:
	ModuleWrapper(slag::Module* module = nullptr);
	~ModuleWrapper();

	bool Initialize(cv::FileNode node);
	void ThreadProcedure();

public:
	std::vector<std::string> settings;
	ModuleIdentifier identifier;
	//non-responsible for MessageQueues
	std::map<PortNumber, MessageQueue*> inputQueues;
	std::map<PortNumber, std::vector<MessageQueue*>> outputQueues; //!< output can be duplicated and distributed to many modules
	size_t inputPortLength;

	std::string output_text;
	std::vector<unsigned char> output_image;

	double diffTime, computeTime;
	std::map<PortNumber, size_t> bufferSize;

private:
	std::unique_ptr<slag::Module> _module; // non-copyable
};
