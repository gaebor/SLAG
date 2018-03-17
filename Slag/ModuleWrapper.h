#pragma once

#include <vector>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <map>

#include "slag/slag_interface.h"
#include "aq/AsyncQueue.h"
#include "ModuleIdentifier.h"
#include "InternalTypes.h"

class Factory;

class ModuleWrapper
{
public:
	enum State
	{
		None, //!< no module
		Uninitialized, //!< instantiated but not initialized
		Initializing, //!< initializing
		Initialized, //!< successfuly initialized, idle state
		Running, //!< IDK but doing something
		Waiting, //!< some of the input queues are blocking
		Computing, //!< hardly working
		Queueing, //!< some of the output queues are blocking
	};
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
	const ModuleIdentifier& GetIdentifier() const{ return identifier; }

	//! gets a global ptr
	//int global_settings_c;
	//const char** global_settings_v;
    bool ConnectToInputPort(PortNumber, MessageQueue*);
    bool ConnectOutputPortTo(PortNumber, MessageQueue*);

    bool RemoveInputPort(PortNumber);

	State GetState()const
	{
		return state;
	}
private:
    
    void ThreadProcedure();
    bool do_run, is_initialized;
    std::thread _thread;

    std::mutex input_mutex, output_mutex;
    // TODO async and sync inputs
	std::unordered_map<PortNumber, MessageQueue*> inputQueues; //!< non-responsible for MessageQueues
	std::unordered_map<PortNumber, std::vector<MessageQueue*>> outputQueues; //!< output can be duplicated and distributed to many modules

    std::map<PortNumber, size_t> bufferSize;

    static const SlagDestroyMessage_t deleteNothing;
private:
    void* txtin, *txtout;
	const char* strout;
	int strout_size;
	unsigned char* output_image_raw;
	int output_image_width, output_image_height;
	static const ImageType imageType;
    
protected:
	friend class Factory;

	ModuleIdentifier identifier;
    ManagedModule _module;
	SlagCompute_t compute;
	SlagInitialize_t initialize;
	SlagDestroyMessage_t deleteMsg;
	
    State state;
};
