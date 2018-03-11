#ifndef INCLUDE_MODULE_WRAPPER_H
#define INCLUDE_MODULE_WRAPPER_H

#include <vector>
#include <map>

#include "slag/slag_interface.h"
#include "aq/AsyncQueue.h"
#include "ModuleIdentifier.h"
#include "InternalTypes.h"

class Factory;

class ModuleWrapper
{
public:
	ModuleWrapper(const bool* run);
	~ModuleWrapper();

	bool Initialize(const std::vector<std::string> settings);
	void ThreadProcedure();

public:
	ModuleIdentifier identifier;
	// TODO async and sync inputs
	std::map<PortNumber, MessageQueue*> inputQueues; //!< non-responsible for MessageQueues
	std::map<PortNumber, std::vector<MessageQueue*>> outputQueues; //!< output can be duplicated and distributed to many modules
	size_t inputPortLength;

	std::map<PortNumber, size_t> bufferSize;

	//! gets a global ptr
	//int global_settings_c;
	//const char** global_settings_v;
	const bool* do_run;

private:
    void* txtin, *txtout;
	const char* strout;
	int strout_size;
	unsigned char* output_image_raw;
	int output_image_width, output_image_height;
	const ImageType imageType;
protected:
	friend class Factory;

	void* _module; // responsible for it!
	SlagCompute_t compute;
	SlagInitialize_t initialize;
	SlagDestroyModule_t deleteModule;
	SlagDestroyMessage_t deleteMsg;
public:
	ModuleWrapper(const ModuleWrapper& other);
};

#endif //INCLUDE_MODULE_WRAPPER_H