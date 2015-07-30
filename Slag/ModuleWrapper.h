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
	ModuleWrapper(const bool* run);
	~ModuleWrapper();

	bool Initialize(cv::FileNode node);
	void ThreadProcedure();

public:
	ModuleIdentifier identifier;
	// TODO async and sync inputs
	std::map<PortNumber, MessageQueue*> inputQueues; //!< non-responsible for MessageQueues
	std::map<PortNumber, std::vector<MessageQueue*>> outputQueues; //!< output can be duplicated and distributed to many modules
	size_t inputPortLength;

	ExclusiveAccess<std::string> output_text;
	ExclusiveAccess<std::vector<unsigned char>> output_image;

	ExclusiveAccess<std::pair<double, double>> diffTime;
	ExclusiveAccess<std::map<PortNumber, size_t>> bufferSize;

	//! gets a global ptr
	int global_settings_c;
	const char** global_settings_v;
	const bool* do_run;

protected:
	friend class Factory;

	const char* output_text_raw;
	unsigned char* output_image_raw;
	int output_image_width, output_image_height;

	void* _module; // responsible for it!
	SlagCompute_t compute;
	SlagInitialize_t initialize;
	SlagDestroyModule_t deleteModule;
	SlagDestroyMessage_t deleteMsg;
};
