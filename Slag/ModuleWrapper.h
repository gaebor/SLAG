#pragma once

#include <memory>
#include <vector>
#include <string>

#include "slag/slag_interface.h"
#include "AsyncQueue.h"
#include "opencv2/core/core.hpp"
#include "ModuleIdentifier.h"
#include "Factory.h"

typedef std::shared_ptr<slag::Message> ManagedMessage;
typedef AsyncQueue<ManagedMessage> MessageQueue;
typedef std::vector<MessageQueue*> MessageQueues;

class ModuleWrapper : private std::unique_ptr<slag::Module>
{
public:
	ModuleWrapper(slag::Module* module = nullptr);
	~ModuleWrapper();

	bool Initialize(cv::FileNode node, const Factory& factory);

public:
	std::vector<std::string> settings;
	ModuleIdentifier identifier;
	//non-responsible for MessageQueues
	MessageQueues inputQueues;
	MessageQueues outputQueues;

	std::string output_text;
	std::vector<unsigned char> output_image;
};
