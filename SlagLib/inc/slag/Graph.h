#pragma once

#include <vector>
#include <memory>

#include "slag/Types.h"

namespace slag {

class Factory;

//! you can control the behavior of the overloaded queue
enum LimitBehavior : int
{
    None = 0, //!< the queue can grow as much as it can, queueLimit is irrelevant
    Drop = 1, //!< drop elements if queue size is above the given limit
    Wait = 2, //!< wait until the queue size drops below the given limit
    Refuse = 3, //!< refuse to enqueue elements if queue size is above the given limit
};

class Graph
{
public:
    Graph();
    ~Graph();

    void Scan();
    std::vector<std::string> GetLibraries()const;
    const char* Help(const std::string& library_name, int argc, const char** argv)const;
    //! creates a module and initializes it
    ErrorCode AddModule(std::vector<std::string> arguments,
        module_callback c = module_callback());

    //! instantiates a new module, does not initialize
    ErrorCode CreateModule(const std::string& name_and_instance);
    //! initializes a module
    ErrorCode InitializeModule(const std::string& name,
        const std::vector<std::string>& arguments = std::vector<std::string>(),
        module_callback c = module_callback());

    ErrorCode AddConnection(const PortIdentifier& from, const PortIdentifier& to, LimitBehavior behavior = LimitBehavior::None, size_t limit = 0);

    ErrorCode RemoveModule(std::string name);
    ErrorCode RemoveConnection(const std::string& from, const std::string& to);

    void Start();

    //! stops the graph immediately
    /*!
         Modules won't start any new computations, but current computations are carried out.
         Then stops the graph.

         If a module is stuck in an infinite loop then it will prevent you from stopping!
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
