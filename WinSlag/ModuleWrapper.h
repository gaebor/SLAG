#pragma once

#include <vector>
#include <string>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <map>

#include "slag/slag_interface.h"
#include "ModuleIdentifier.h"
#include "QueueTypes.h"

class Factory;

typedef void(*output_text_callback)(const std::string& module_name_and_instance, const char* text, int length);
typedef void (*statistics_callback)(const std::string& module_name_and_instance, double cycle, double load, double wait, const std::map<PortNumber, size_t>& buffer_sizes);
typedef void (*output_image_callback)(const std::string& module_name_and_instance, int w, int h, ImageType type, const unsigned char* data);

class ModuleWrapper
{
public:
	enum State
	{
		None, //!< no module
		Uninitialized, //!< instantiated but not initialized
		Initializing, //!< initializing
		Initialized, //!< successfully initialized, idle state
		Running, //!< IDK but doing something
		Waiting, //!< some of the input queues are blocking
		Computing, //!< hardly working
		Queueing, //!< some of the output queues are blocking
	};
	ModuleWrapper();
	~ModuleWrapper();

    bool Initialize(const std::vector<std::string>& settings,
        statistics_callback s = nullptr, output_text_callback t = nullptr, 
        output_image_callback i = nullptr);

    //! start processing
    void Start();
    
    //! gracefully wait for the module
    void Wait();

    //! tell to stop, but do not join or block
    /*!
        This means that the module does not start new computation.
        Also leaves the pending computations in the input queues.
    */
    void Stop();

public:
    const ModuleIdentifier& GetIdentifier() const;
    const std::string& GetPrintableName() const { return identifier.module; }

    //! returns which library instantiated the module
    const std::string& GetLibrary() const;

	//! gets a global ptr
	//int global_settings_c;
	//const char** global_settings_v;

    bool ConnectToInputPort(PortNumber, MessageQueue*);
    bool ConnectOutputPortTo(PortNumber, MessageQueue*);

    bool RemoveInputPort(PortNumber);

    State GetState()const;

protected:
    friend class Factory;

    FullModuleIdentifier identifier;
    ManagedModule _module;
    SlagCompute_t compute;
    SlagInitialize_t initialize;
    SlagDestroyMessage_t deleteMsg;

private:
    
    void ThreadProcedure();
    std::thread _thread;

    std::mutex input_mutex, output_mutex;
    // TODO async and sync inputs
	std::unordered_map<PortNumber, MessageQueue*> inputQueues; //!< non-responsible for MessageQueues
	std::unordered_map<PortNumber, std::vector<MessageQueue*>> outputQueues; //!< output can be duplicated and distributed to many modules

    std::map<PortNumber, size_t> bufferSize;

    static const SlagDestroyMessage_t deleteNothing;
private:
    statistics_callback handle_statistics;
    output_text_callback handle_output_text;
    output_image_callback handle_output_image;
private:
    void* txtin, *txtout;
	const char* strout;
	unsigned char* output_image_raw;
	static const ImageType imageType;
    
    int strout_size;
    int output_image_width, output_image_height;

    std::atomic<State> state;
    bool do_run, is_initialized;
    
};