#pragma once

#include <map>
#include <list>

#include "aq/Clock.h"

#include "Factory.h"
#include "ModuleWrapper.h"

class Graph
{
public:
    Graph();
    ~Graph();

    ErrorCode AddModule(std::vector<std::string> arguments);
    ErrorCode AddConnection(const std::string& from, const std::string& to, MessageQueue::LimitBehavior behavior = MessageQueue::None);

    void Start();
    
    //! signals the modules to stop
    /*!
        Does not block or halt, but tells the modules
        that they should not start new computations.
        Also leaves the pending computations in the input queues.
    */
    void SignalStop();

    void Stop();

    //! waits for every module to finish
    /*!
        if the SignalStop has not been called
        then this means that the modules should nicely run out of jobs and halt
    */
    void Wait();

    //! Returns whether the graph is busy
    bool IsRunning()const;

    const ModuleIdentifier* GetModuleId(const std::string& name)const
    {
        auto it = modules.find(ModuleIdentifier(name.c_str()));
    }
private:
    std::map<ModuleIdentifier, std::unique_ptr<ModuleWrapper>> modules;
    std::list<std::unique_ptr<MessageQueue>> messageQueues;

    Factory factory;
    aq::Clock timer;
    std::thread modules_process;

    double timeout, startTime;
};
