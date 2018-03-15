#include "ModuleWrapper.h"

#include <set>
#include <algorithm>

#include "Factory.h"
#include "aq/Clock.h"
#include "OS_dependent.h"

ModuleWrapper::~ModuleWrapper(void)
{
}

ModuleWrapper::ModuleWrapper()
:	_module(),
	output_image_raw(nullptr),
    strout(nullptr),
    strout_size(0),
	output_image_width(0), output_image_height(0),
	do_run(false), is_initialized(false),
	imageType(get_image_type())
{
    messages.reserve(1024);
}

bool ModuleWrapper::Initialize(const std::vector<std::string>& settings)
{
	if (_module != nullptr)
	{
        if (initialize == NULL)
        {
            is_initialized = true;
            messages = "";
        }
        else
        {
            std::vector<const char*> settings_array;

            // outtext;
            for (const auto& setting : settings)
                settings_array.push_back(setting.c_str());

            //Initialize
            is_initialized =
                initialize(
                    _module.get(),
                    (int)settings_array.size(), settings_array.data(),
                    get_txtin((std::string)identifier), get_txtout((std::string)identifier),
                    &strout, &strout_size,
                    &output_image_raw, &output_image_width, &output_image_height, imageType
                ) == 0;
            if (is_initialized)
                messages = "[INITIALIZED]";
            else
                messages = "[INIT FAILED]";
            handle_output_text(identifier, messages.c_str(), (int)messages.size());
        }
		// module settings are lost after the module initialize!
		//TODO global settings
	}
	return is_initialized;
}

void ModuleWrapper::Start()
{
    do_run = true;
    _thread = std::thread([](ModuleWrapper* _m) {_m->ThreadProcedure(); }, this);
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

bool ModuleWrapper::ConnectToInputPort(PortNumber n, MessageQueue* p)
{
    if (inputQueues.find(n) == inputQueues.end())
    {
        AutoLock lock(input_mutex);
        inputQueues[n] = p;
        return true;
    }
    else // already receives inpu
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

void ModuleWrapper::ThreadProcedure()
{
	std::vector<void*> inputMessages(1);
    
    const std::string printableName(identifier);

    void** outputMessages_raw;

	//Dequeued input messages which haven't been Enqueued to the output yet
	std::vector<ManagedMessage> receivedMessages;
	
	aq::Clock timer_cycle, timer;
	double cycle_time = 1.0, compute_time = 0.0, wait_time = 0.0;
	PortNumber outputNumber;

	while (do_run) //a terminating signal leaves every message in the queue and quits the loop
	{
		timer.Tick();
        {
            AutoLock lock(input_mutex);
            inputMessages.assign(inputMessages.size(), nullptr);
            //manage input data
            for (auto& q : inputQueues)
            {
                // this actual deleteMsg function won't be used here, because the dequeue will override it and the nullptr won't be deleted anyway
                ManagedMessage input;

                bufferSize[q.first] = q.second->GetSize();

                if (!q.second->DeQueue(input))
                {
                    wait_time = timer.Tock();
                    compute_time = 0.0;
                    cycle_time = timer_cycle.Tock();
                    goto halt; // input causes halt
                }
                if (q.first + 2 > inputMessages.size())
                    inputMessages.resize(q.first + 2, nullptr);

                inputMessages[q.first] = input.get();
                receivedMessages.push_back(input);
            }
        }
		wait_time = timer.Tock();
		
		timer.Tick();
		outputMessages_raw = compute(_module.get(), inputMessages.data(), (int)inputMessages.size()-1, &outputNumber);
		compute_time = timer.Tock();

		//manage output data
		if (outputMessages_raw == nullptr)
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
                auto messagePtr = outputMessages_raw[i];
                ManagedMessage managedOutput;

                auto inputIt = std::find_if(receivedMessages.begin(), receivedMessages.end(), [&](const ManagedMessage& m) {return m.get() == messagePtr; });
                if (inputIt != receivedMessages.end())
                {
                    managedOutput = *inputIt;
                }
                else
                {
                    managedOutput.reset(messagePtr, deleteMsg);
                }

                outputIt = outputQueues.find(i);
                if (outputIt != outputQueues.end())
                {
                    for (auto out : outputIt->second)
                    {
                        out->EnQueue(managedOutput);
                    }
                }
            }
        }
		receivedMessages.clear();

		cycle_time = timer_cycle.Tock();
		timer_cycle.Tick();

		handle_statistics(printableName, cycle_time, compute_time, wait_time, bufferSize);
		if (strout != nullptr)
			handle_output_text(printableName, strout, strout_size);

		if (output_image_raw != nullptr)
		{
			handle_output_image(printableName, output_image_width, output_image_height, imageType, output_image_raw);
		}

	}
halt:
    messages = "[STOPPED]";
    handle_statistics(printableName, cycle_time, compute_time, wait_time, bufferSize);
    handle_output_text(printableName, messages.c_str(), (int)messages.size());

	if (do_run) //in this case soft terminate
		for (auto& qs : outputQueues)
			for (auto& q : qs.second)
				q->WaitForEmpty();
	
	//TODO wake up only the synced queues
	for (auto& qs : outputQueues)
		for (auto& q : qs.second)
			q->WakeUp();
    
    do_run = false;
}
