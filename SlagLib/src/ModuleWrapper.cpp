#include "ModuleWrapper.h"

#include <set>
#include <unordered_set>
#include <vector>

#include "Factory.h"
#include "aq/Clock.h"
#include "OS_dependent.h"

namespace slag {

ModuleWrapper::~ModuleWrapper(void)
{
}

const SlagImageType ModuleWrapper::imageType(get_image_type());
const SlagDestroyMessage_t ModuleWrapper::deleteNothing([](void*){ return; });

ModuleWrapper::ModuleWrapper()
:   compute(nullptr),
    initialize(nullptr),
    deleteMsg(nullptr),
    handle_statistics(nullptr),
    handle_output_text(nullptr),
    handle_output_image(nullptr),
    strout(nullptr),
    output_image_raw(nullptr),
    
    strout_size(0),
	output_image_width(0), output_image_height(0),
    do_run(false), state(StatusCode::UnInitialized)
{
}

bool ModuleWrapper::Initialize(const std::vector<std::string>& settings,
    statistics_callback s, statistics2_callback s2, output_text_callback t, output_image_callback i)
{
    state = StatusCode::UnInitialized;

    handle_statistics = s;
    handle_statistics2 = s2;
    handle_output_text = t;
    handle_output_image = i;
	if (_module != nullptr)
	{
        if (initialize == NULL)
        {
            state = StatusCode::Idle;
            return true;
        }
        else
        {
			state = StatusCode::Initializing;
            std::vector<const char*> settings_array;

            for (const auto& setting : settings)
                settings_array.push_back(setting.c_str());

            //Initialize
            const bool initialized = 
                initialize(
                    _module.get(),
                    (int)settings_array.size(), settings_array.data(),
                    nullptr, nullptr,
                    &strout, &strout_size,
                    &output_image_raw, &output_image_width, &output_image_height, imageType
                ) == 0;
            state = initialized ? StatusCode::Idle : StatusCode::UnInitialized;
            return initialized;
        }
		// module settings are lost after the module initialize!
		//TODO global settings
	}else
	    return false;
}

void ModuleWrapper::Start()
{
    if (!IsRunning())
    {
        do_run = true;
        _thread = std::thread([](ModuleWrapper* _m) {_m->ThreadProcedure(); }, this);
    }
}

void ModuleWrapper::Wait()
{
    if (_thread.joinable())
        _thread.join();
    do_run = false;
}

void ModuleWrapper::Stop()
{
    do_run = false;
}

bool ModuleWrapper::IsRunning() const
{
    return do_run && _thread.joinable();
}

const ModuleIdentifier & ModuleWrapper::GetIdentifier() const
{
    return identifier.module;
}

const FullModuleIdentifier & ModuleWrapper::GetFullIdentifier() const
{
    return identifier;
}

const std::string & ModuleWrapper::GetLibrary() const
{
    return identifier.library;
}

bool ModuleWrapper::ConnectToInputPort(PortNumber n, MessageQueue* p)
{
    if (inputQueues.find(n) == inputQueues.end())
    {
        AutoLock lock(input_mutex);
        inputQueues[n] = p;
        return true;
    }
    else // already receives input
        return false;
}

bool ModuleWrapper::ConnectOutputPortTo(PortNumber n, MessageQueue* p)
{
    if (outputQueues.find(n) == outputQueues.end())
    {
        AutoLock lock(output_mutex);
        outputQueues[n].push_back(p);
        return true;
    }
    else // already connected to somewhere   
    { 
        auto& queues = outputQueues[n];
        for (auto q : queues)
        {
            if (q == p)
                return false;
        }
        {
            AutoLock lock(output_mutex);
            queues.push_back(p);
            return true;
        }
    }
}

bool ModuleWrapper::RemoveInputPort(PortNumber n)
{
    if (inputQueues.find(n) == inputQueues.end())
    {
        return false; // there is nothing to remove.
    } else
    {
        AutoLock lock(input_mutex);
        inputQueues.erase(n);
        return true;
    }
}

StatusCode ModuleWrapper::GetStatus() const
{
    return state;
}

void ModuleWrapper::ThreadProcedure()
{
    state = StatusCode::Running;
	std::vector<void*> inputMessages_c(1);

    void** outputMessages_c;

	//Dequeued input messages which haven't been Enqueued to the output yet
	std::unordered_set<ManagedMessage> receivedMessages;

	aq::Clock timer_cycle, timer;
	double cycle_time = 1.0, compute_time = 0.0, wait_time = 0.0;
	PortNumber outputNumber;

	while (do_run) //a terminating signal leaves every message in the queue and quits the loop
	{
		timer.Tick();
        {
            AutoLock lock(input_mutex);
            state = StatusCode::Waiting;
			inputMessages_c.assign(inputMessages_c.size(), nullptr);
            //manage input data
            for (auto& q : inputQueues)
            {
                if (handle_statistics2)
                    bufferSize[q.first] = q.second->GetSize();
				ManagedMessage managedMessage;
				if (!q.second->DeQueue(managedMessage))
                {
                    wait_time = timer.Tock();
                    compute_time = 0.0;
                    cycle_time = timer_cycle.Tock();
                    goto halt; // input causes halt
                }
				if (q.first + 2 > inputMessages_c.size())
					inputMessages_c.resize(q.first + 2, nullptr);

				inputMessages_c[q.first] = managedMessage.get();
				receivedMessages.insert(managedMessage);
            }
        }
		wait_time = timer.Tock();
		
		timer.Tick();
        state = StatusCode::Computing;
		outputMessages_c = compute(_module.get(), inputMessages_c.data(), (int)inputMessages_c.size() - 1, &outputNumber);
		compute_time = timer.Tock();
        state = StatusCode::Queueing;

		//manage output data
		if (outputMessages_c == nullptr)
		{
			compute_time = timer.Tock();
			cycle_time = timer_cycle.Tock();
			goto halt; // module itself causes halt
		}

        {
            AutoLock lock(output_mutex);
            auto outputIt = outputQueues.begin();
            for (PortNumber i = 0; i < outputNumber; ++i)
            {
				const auto message_c = outputMessages_c[i];
				ManagedMessage managedMessage(message_c, deleteNothing);
				auto inputIt = receivedMessages.find(managedMessage); //find by .get()
                if (inputIt != receivedMessages.end())
                {
                    managedMessage = *inputIt; // gets input's deleter
                }
				else if (message_c != nullptr) // does it worth deleting
				{  // gets this module's deleter
					managedMessage.reset(message_c, deleteMsg);
				}
                outputIt = outputQueues.find(i);
                if (outputIt != outputQueues.end())
                {
                    for (auto out : outputIt->second)
                    {
						out->EnQueue(managedMessage);
                    }
                }
            }
        }
		receivedMessages.clear();

		cycle_time = timer_cycle.Tock();
		timer_cycle.Tick();
        
        state = StatusCode::Running;

        if (handle_statistics)
    		handle_statistics(identifier.module, cycle_time, compute_time, wait_time);
        if (handle_statistics2)
        {
            for (const auto& s : bufferSize)
                handle_statistics2(identifier.module, s.second);
            bufferSize.clear();
        }
        if (strout && handle_output_text)
			handle_output_text(identifier.module, strout, strout_size);
		
        if (output_image_raw && handle_output_image)
            handle_output_image(identifier.module, output_image_width, output_image_height, imageType, output_image_raw);
	}
halt:
    state = StatusCode::Running;
    if (handle_statistics)
        handle_statistics(identifier.module, cycle_time, compute_time, wait_time);
    if (handle_statistics2)
        for (const auto& s : bufferSize)
            handle_statistics2(identifier.module, s.second);
    if (handle_output_text)
        handle_output_text(identifier.module, nullptr, 0);
    if (handle_output_image)
        handle_output_image(identifier.module, 0, 0, imageType, nullptr);

	if (do_run) //in this case soft terminate
		for (auto& qs : outputQueues)
			for (auto& q : qs.second)
				q->WaitForEmpty();
	
	//TODO wake up only the synced queues
	for (auto& qs : outputQueues)
		for (auto& q : qs.second)
			q->WakeUp();
    
    do_run = false;
    state = StatusCode::Idle;
}

}
