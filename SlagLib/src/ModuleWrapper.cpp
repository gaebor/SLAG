#include "ModuleWrapper.h"

#include <vector>

#include "Factory.h"
#include "OS_dependent.h"

namespace slag {

ModuleWrapper::~ModuleWrapper(void)
{
}

const SlagDestroyMessage_t ModuleWrapper::deleteNothing([](void*){ return; });

ModuleWrapper::ModuleWrapper()
:   compute(nullptr),
    initialize(nullptr),
    deleteMsg(nullptr),
    imageOut({ nullptr, 0, 0, get_image_type() }),
    textOut({ nullptr, 0}),
    do_run(false), state(StatusCode::UnInitialized)
{    
}

bool ModuleWrapper::Initialize(const std::vector<std::string>& settings, module_callback callback)
{
    state = StatusCode::UnInitialized;

    handle_data = callback;

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
                    &textOut, &imageOut ) == 0;
            state = initialized ? StatusCode::Idle : StatusCode::UnInitialized;
            if (handle_data)
                handle_data(identifier.module, textOut, imageOut, stats);

            return initialized;
        }
		// module settings are lost after the module initialize!
		//TODO global settings
	}else
	    return false;
}

void ModuleWrapper::Start(bool dynamic)
{
    if (!IsRunning())
    {
        do_run = true;
        if  (dynamic)
            _thread = std::thread([](ModuleWrapper* _m) {_m->ThreadProcedure<true>(); }, this);
        else
            _thread = std::thread([](ModuleWrapper* _m) {_m->ThreadProcedure<false>(); }, this);
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
        AutoLock<> lock(input_mutex);
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
        AutoLock<> lock(output_mutex);
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
            AutoLock<> lock(output_mutex);
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
        AutoLock<> lock(input_mutex);
        inputQueues.erase(n);
        return true;
    }
}

StatusCode ModuleWrapper::GetStatus() const
{
    return state;
}

}
