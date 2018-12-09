#pragma once

#include <unordered_map>
#include <list>
#include <memory>

#include "aq/AsyncQueue.h"
#include "SlagTypes.h"

class Factory;

class Graph
{
public:
    Graph();
    ~Graph();

    //! creates a module and initializes it
    ErrorCode AddModule(std::vector<std::string> arguments,
        statistics_callback s = statistics_callback(),
        output_text_callback t = output_text_callback(),
        output_image_callback i = output_image_callback());

    //! instantiates a new module
    ErrorCode CreateModule(std::vector<std::string> arguments);
    //! initializes a module, optionally starts a thread for it and does not block
    ErrorCode InitializeModule(std::vector<std::string> arguments, int);

    ErrorCode AddConnection(const std::string& from, const std::string& to, aq::LimitBehavior behavior = aq::LimitBehavior::None, size_t limit = 0);

    ErrorCode RemoveModule(std::string name);
    ErrorCode RemoveConnection(const std::string& from, const std::string& to);

    void Start();

    //! shuts down the modules as they are
    /*!
        mindenki dobja el, ami a kezében vam!
    */
    void Stop();

    //! waits for every module to finish
    /*!
        if Stop has not been called
        then this means that the modules should nicely run out of jobs and halt
    */
    void Wait();

    //! Returns whether the graph is busy
    bool IsRunning()const;

    const FullModuleIdentifier* GetModuleId(const std::string& name)const;
private:
    struct ModulesType;
    std::unique_ptr<ModulesType> modules;
    struct MessageQueuesType;
    std::unique_ptr<MessageQueuesType> messageQueues;
    std::unique_ptr<Factory> factory;
    // std::unordered_map<ModuleIdentifier, std::unique_ptr<ModuleWrapper>> modules;
    // std::list<std::unique_ptr<MessageQueue>> messageQueues;
};
