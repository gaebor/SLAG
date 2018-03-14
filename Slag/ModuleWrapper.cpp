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
:	inputPortLength(0),
	_module(),
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

void ModuleWrapper::ThreadProcedure()
{
	std::vector<void*> inputMessages(inputPortLength+1, nullptr);//HACK
    inputMessages.resize(inputPortLength);
    
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
		//manage input data
		for (auto& q : inputQueues)
		{
			// this actual deleteMsg function won't be used here, because the dequeue will override it and the nullptr won't be deleted anyway
			ManagedMessage input;

			if (!q.second->DeQueue(input))
			{
				wait_time = timer.Tock();
				compute_time = 0.0;
				cycle_time = timer_cycle.Tock();
                goto halt; // input causes halt
			}

			inputMessages[q.first] = input.get();
			receivedMessages.push_back(input);
		}
		wait_time = timer.Tock();
		
		timer.Tick();
		outputMessages_raw = compute(_module.get(), inputMessages.data(), (int)inputMessages.size(), &outputNumber);
		compute_time = timer.Tock();

		for (const auto& q : inputQueues)
		{
			bufferSize[q.first] = q.second->GetSize();
		}

		//manage output data
		if (outputMessages_raw == nullptr)
		{
			compute_time = timer.Tock();
			cycle_time = timer_cycle.Tock();
			goto halt; // module itself causes halt
		}

		auto outputIt = outputQueues.begin();
		for (PortNumber i = 0; i < outputNumber; ++i)
		{
			auto messagePtr = outputMessages_raw[i];
			ManagedMessage managedOutput;

			auto inputIt = std::find_if(receivedMessages.begin(), receivedMessages.end(), [&](const ManagedMessage& m){return m.get()==messagePtr;});
			if (inputIt != receivedMessages.end())
			{
				managedOutput = *inputIt;
			}else
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
	handle_statistics(printableName, cycle_time, compute_time, wait_time, bufferSize);

    messages = "[STOPPED]";

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
