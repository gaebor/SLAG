#pragma once

#include <vector>
#include <string>
#include <memory>

#include "slag/slag_interface.h"
#include "AsyncQueue.h"
#include "opencv2/core/core.hpp"
#include "ModuleIdentifier.h"
#include "ExclusiveAccess.h"
#include "InternalTypes.h"

class Factory;

class ModuleWrapper
{
public:
	ModuleWrapper();
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

	ExclusiveAccess<std::string> output_text;
	ExclusiveAccess<std::vector<unsigned char>> output_image;

	ExclusiveAccess<std::pair<double, double>> diffTime;
	ExclusiveAccess<std::map<PortNumber, size_t>> bufferSize;

protected:
	friend class Factory;

	const char* output_text_raw;
	unsigned char* output_image_raw;
	int output_image_width, output_image_height;

	void* _module; // responsible for it!
	slag::SlagCompute compute;
	slag::SlagInitialize initialize;
	slag::SlagDestroyModule deleteModule;
	slag::SlagDestroyMessage deleteMsg;
};
