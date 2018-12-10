#pragma once

#include <unordered_map>
#include <list>
#include <memory>

#include "aq/AsyncQueue.h"
#include "SlagTypes.h"

namespace slag {

class Factory;

class Graph
{
public:
    Graph();
    ~Graph();

    void Scan();
    std::vector<std::string> GetLibraries()const;
    //! creates a module and initializes it
    ErrorCode AddModule(std::vector<std::string> arguments,
        statistics_callback s = statistics_callback(),
        statistics2_callback s2 = statistics2_callback(),
        output_text_callback t = output_text_callback(),
        output_image_callback i = output_image_callback());

    //! instantiates a new module, does not initialize
    ErrorCode CreateModule(const std::string& name_and_instance);
    //! initializes a module, optionally starts a thread for it and does not block
    ErrorCode InitializeModule(const std::string& name,
        const std::vector<std::string>& arguments = std::vector<std::string>(),
        statistics_callback s = statistics_callback(),
        statistics2_callback s2 = statistics2_callback(),
        output_text_callback t = output_text_callback(),
        output_image_callback i = output_image_callback());

    ErrorCode AddConnection(const PortIdentifier& from, const PortIdentifier& to, aq::LimitBehavior behavior = aq::LimitBehavior::None, size_t limit = 0);

    ErrorCode RemoveModule(std::string name);
    ErrorCode RemoveConnection(const std::string& from, const std::string& to);

    void Start();

    //! shuts down the modules as they are
    /*!
        mindenki dobja el, ami a kez�ben vam!
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

    StatusCode GetStatus(const ModuleIdentifier& name)const;

    const FullModuleIdentifier* GetModuleId(const ModuleIdentifier& name)const;
private:
    struct ModulesType;
    std::unique_ptr<ModulesType> modules;
    struct MessageQueuesType;
    std::unique_ptr<MessageQueuesType> messageQueues;
    std::unique_ptr<Factory> factory;
};

}