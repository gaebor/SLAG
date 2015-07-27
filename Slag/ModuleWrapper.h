#pragma once

#include <vector>
#include <string>
#include <memory>

#include "slag/slag_interface.h"
#include "AsyncQueue.h"
#include "opencv2/core/core.hpp"
#include "ModuleIdentifier.h"
#include "ExclusiveAccess.h"

typedef std::shared_ptr<slag::Message> ManagedMessage;
typedef AsyncQueue<ManagedMessage> MessageQueue;

class ModuleWrapper
{
public:
	ModuleWrapper();
	bool SetModule(slag::Module* m);
	~ModuleWrapper();

	bool Initialize(cv::FileNode node);
	void ThreadProcedure();

public:
	std::vector<std::string> settings;
	ModuleIdentifier identifier;
	// TODO async and sync inputs
	std::map<PortNumber, MessageQueue*> inputQueues; //!< non-responsible for MessageQueues
	std::map<PortNumber, std::vector<MessageQueue*>> outputQueues; //!< output can be duplicated and distributed to many modules
	size_t inputPortLength;

	const char* output_text_raw;
	slag::Picture output_image_raw;
	ExclusiveAccess<std::string> output_text;
	ExclusiveAccess<std::vector<unsigned char>> output_image;

	ExclusiveAccess<std::pair<double, double>> diffTime;
	ExclusiveAccess<std::map<PortNumber, size_t>> bufferSize;

private:
	slag::Module* _module; // responsible for it!
};
