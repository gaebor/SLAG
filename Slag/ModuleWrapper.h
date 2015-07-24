#pragma once

#include <vector>
#include <string>
#include <memory>

#include "slag/slag_interface.h"
#include "AsyncQueue.h"
#include "opencv2/core/core.hpp"
#include "ModuleIdentifier.h"
#include "Factory.h"

typedef AsyncQueue<slag::Message*> MessageQueue;

class ModuleWrapper
{
public:
	ModuleWrapper(slag::Module* module = nullptr);
	~ModuleWrapper();

	bool Initialize(cv::FileNode node, const Factory& factory);
	void ThreadProcedure();

public:
	std::vector<std::string> settings;
	ModuleIdentifier identifier;
	//non-responsible for MessageQueues
	std::map<PortNumber, MessageQueue*> inputQueues, outputQueues;
	size_t inputPortLength, outputPortLength;

	std::string output_text;
	std::vector<unsigned char> output_image;

	double diffTime, computeTime;
	std::vector<size_t> bufferSize;

private:
	std::unique_ptr<slag::Module> _module; // non-copyable
};
