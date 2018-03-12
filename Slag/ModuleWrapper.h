#pragma once

#include <vector>
#include <thread>
#include <map>

#include "slag/slag_interface.h"
#include "aq/AsyncQueue.h"
#include "ModuleIdentifier.h"
#include "InternalTypes.h"

class Factory;

class ModuleWrapper
{
public:
	ModuleWrapper();
	~ModuleWrapper();

    bool Initialize(const std::vector<std::string>& settings);

    //! start processing
    void Start();
    //! gracefully wait for the module
    void Wait();
    //! tell to stop, but do not join or block
    void Stop();

public:
	ModuleIdentifier identifier;
	// TODO async and sync inputs
	std::map<PortNumber, MessageQueue*> inputQueues; //!< non-responsible for MessageQueues
	std::map<PortNumber, std::vector<MessageQueue*>> outputQueues; //!< output can be duplicated and distributed to many modules
	size_t inputPortLength;

	//! gets a global ptr
	//int global_settings_c;
	//const char** global_settings_v;

private:
    
    void ThreadProcedure();
    bool do_run;
    std::thread _thread;
private:
    void* txtin, *txtout;
	const char* strout;
	int strout_size;
	unsigned char* output_image_raw;
	int output_image_width, output_image_height;
	const ImageType imageType;

    std::map<PortNumber, size_t> bufferSize;

protected:
	friend class Factory;

    ManagedModule _module;
	SlagCompute_t compute;
	SlagInitialize_t initialize;
	SlagDestroyMessage_t deleteMsg;
private:
	// ModuleWrapper(const ModuleWrapper& other);
};
