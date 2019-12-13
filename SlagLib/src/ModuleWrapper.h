#pragma once

#include <vector>
#include <string>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <functional>

#include "aq/Clock.h"

#include "slag_interface.h"
#include "slag/Types.h"
#include "slag/Identifiers.h"
#include "InternalTypes.h"

namespace slag {

class Factory;

class ModuleWrapper
{
public:

    ModuleWrapper();
    ~ModuleWrapper();

    bool Initialize(const std::vector<std::string>& settings, module_callback c = module_callback());

    //! start processing
    void Start(bool dynamic = true);

    //! gracefully waits for the module to complete
    void Wait();

    //! tell to stop, but do not join or block
    /*!
        This means that the module does not start new computation.
        Also leaves the pending computations in the input queues.
    */
    void Stop();

    bool IsRunning() const;

public:
    const ModuleIdentifier& GetIdentifier() const;
    const FullModuleIdentifier& GetFullIdentifier() const;
    //! returns which library instantiated the module
    const std::string& GetLibrary() const;

    //! gets a global ptr
    //int global_settings_c;
    //const char** global_settings_v;

    bool ConnectToInputPort(PortNumber, MessageQueue*);
    bool ConnectOutputPortTo(PortNumber, MessageQueue*);

    bool RemoveInputPort(PortNumber);

    StatusCode GetStatus()const;

protected:
    friend class Factory;

    FullModuleIdentifier identifier;
    ManagedModule _module;
    SlagCompute_t compute;
    SlagInitialize_t initialize;
    SlagDestroyMessage_t deleteMsg;

private:
    template<bool dynamic>
    void ThreadProcedure()
    {
        state = StatusCode::Running;
        std::vector<void*> inputMessages_c(1);

        void** outputMessages_c;

        //Dequeued input messages which haven't been Enqueued to the output yet
        std::unordered_set<ManagedMessage> receivedMessages;

        aq::Clock<> timer_cycle, timer;
        PortNumber outputNumber;

        while (do_run) //a terminating signal leaves every message in the queue and quits the loop
        {
            timer.Tick();
            {
                AutoLock<dynamic> lock(input_mutex);
                state = StatusCode::Waiting;
                inputMessages_c.assign(inputMessages_c.size(), nullptr);
                //manage input data
                stats.buffers.clear();
                for (auto& q : inputQueues)
                {
                    stats.buffers.emplace_back(q.first, q.second->GetSize());
                    ManagedMessage managedMessage;
                    if (!q.second->DeQueue(managedMessage))
                    {
                        stats.wait = timer.Tock();
                        stats.load = 0.0;
                        stats.cycle = timer_cycle.Tock();
                        goto halt; // input causes halt
                    }
                    if (q.first + 2 > inputMessages_c.size())
                        inputMessages_c.resize(q.first + 2, nullptr);

                    inputMessages_c[q.first] = managedMessage.get();
                    receivedMessages.insert(managedMessage);
                }
            }
            stats.wait = timer.Tock();

            timer.Tick();
            state = StatusCode::Computing;
            outputMessages_c = compute(_module.get(), inputMessages_c.data(), (int)inputMessages_c.size() - 1, &outputNumber);
            stats.load = timer.Tock();
            state = StatusCode::Queueing;

            //manage output data
            if (outputMessages_c == nullptr)
            {
                stats.cycle = timer_cycle.Tock();
                goto halt; // module itself causes halt
            }

            {
                AutoLock<dynamic> lock(output_mutex);
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

            stats.cycle = timer_cycle.Tock();
            timer_cycle.Tick();

            state = StatusCode::Running;

            if (handle_data)
                handle_data(identifier.module, textOut, imageOut, stats);
        }
    halt:
        state = StatusCode::Running;
        if (handle_data)
            handle_data(identifier.module, textOut, imageOut, stats);

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

    std::thread _thread;

    std::mutex input_mutex, output_mutex;
    // TODO async and sync inputs
	std::unordered_map<PortNumber, MessageQueue*> inputQueues; //!< non-responsible for MessageQueues
	std::unordered_map<PortNumber, std::vector<MessageQueue*>> outputQueues; //!< output can be duplicated and distributed to many modules

    static const SlagDestroyMessage_t deleteNothing;
private:
    module_callback handle_data;
private:
    SlagTextOut textOut;
    SlagImageOut imageOut;
    Stats stats;

    // std::atomic<State> state;
    std::atomic<bool> do_run;
    std::atomic<StatusCode> state;
};

}
