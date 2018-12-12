
#include "Graph.h"

#include "aq/Clock.h"

#include "Factory.h"
#include "ModuleWrapper.h"
#include "InternalTypes.h"

namespace slag {

struct Graph::ModulesType : public std::unordered_map<ModuleIdentifier, std::unique_ptr<ModuleWrapper>>
{
};

struct Graph::MessageQueuesType : public std::list<std::unique_ptr<MessageQueue>>
{
};

Graph::Graph()
    : messageQueues(new MessageQueuesType()), modules(new ModulesType()), factory(new Factory())
{
    Scan();
}

void Graph::Scan()
{
    factory->Scan();
}

std::vector<std::string> Graph::GetLibraries()const
{
    return factory->GetLibraries();
}

Graph::~Graph()
{
    Stop();
    messageQueues->clear();
    modules->clear();
}

ErrorCode Graph::AddModule(std::vector<std::string> arguments, module_callback c)
{
    if (arguments.empty())
        return WrongArguments;

    std::string moduleName = arguments[0];
    if (moduleName.empty())
        return WrongArguments;
    
    arguments.erase(arguments.begin());
    
    ErrorCode result = CreateModule(moduleName);
    if (result == ErrorCode::Success)
    {
        const ModuleIdentifier moduleId(FullModuleIdentifier(moduleName.c_str()).module);
        result = InitializeModule(moduleId, arguments, c);
    }

    return result;
}

ErrorCode Graph::CreateModule(const std::string& moduleName)
{
    if (moduleName.empty())
        return WrongArguments;

    const FullModuleIdentifier fullModuleId(moduleName.c_str());
    const ModuleIdentifier moduleId = fullModuleId.module;

    if (modules->find(moduleId) != modules->end())
        return AlreadyExists;

    const auto result = factory->InstantiateModule(moduleName);
    if (result.second == ErrorCode::Success)
    {
        (*modules)[moduleId].reset(result.first);
    } else if (result.first)
    {
        delete result.first;
    }
    return result.second;
}

ErrorCode Graph::InitializeModule(
    const std::string& moduleName,
    const std::vector<std::string>& arguments, module_callback c)
{
    if (moduleName.empty())
        return WrongArguments;

    const ModuleIdentifier moduleId(moduleName.c_str());

    const auto it = modules->find(moduleId);
    if (it != modules->end())
    {
        return it->second->Initialize(arguments, c) ? ErrorCode::Success : ErrorCode::CannotInitialize;
    }
    else
        return ErrorCode::NoSuchModule;
}

ErrorCode Graph::AddConnection(
                const PortIdentifier& fromModuleId, const PortIdentifier& toModuleId,
                aq::LimitBehavior behavior, size_t limit)
{
    const auto fromModulePtr = modules->find(fromModuleId.module);
    const auto toModulePtr = modules->find(toModuleId.module);

    if (fromModulePtr == modules->end())
        return ErrorCode::WrongArguments;

    if (toModulePtr == modules->end())
        return ErrorCode::WrongArguments;

    messageQueues->emplace_back(new MessageQueue());
    auto newMessageQueue = messageQueues->back().get();
    newMessageQueue->limitBehavior = behavior;
    newMessageQueue->queueLimit = limit;

    const auto ok1 = toModulePtr->second->ConnectToInputPort(toModuleId.port, newMessageQueue);
    if (ok1)
    {
        const auto ok2 = fromModulePtr->second->ConnectOutputPortTo(fromModuleId.port, newMessageQueue);
        if (ok2)
        {
            return ErrorCode::Success;
        }
        else
        {
            toModulePtr->second->RemoveInputPort(toModuleId.port);
        }
    }
    
    return ErrorCode::Duplicate;
}

void Graph::Start()
{
    if (!IsRunning())
    {
        for (auto& m : *modules)
            m.second->Start();
    }
}

void Graph::Stop()
{
    for (auto& m : *modules)
        m.second->Stop();

    for (auto& q : *messageQueues)
        q->WakeUp();

    Wait();
}

void Graph::Wait()
{
    for (auto& m : *modules)
        m.second->Wait();
}

bool Graph::IsRunning() const
{
    for (auto& m : *modules)
        if (m.second->IsRunning())
            return true;
    return false;
}

StatusCode Graph::GetStatus(const ModuleIdentifier & name) const
{
    auto it = modules->find(name);
    if (it != modules->end())
    {
        return it->second->GetStatus();
    }
    else
        return StatusCode::UnInitialized;
}

const FullModuleIdentifier * Graph::GetModuleId(const ModuleIdentifier& name) const
{
    auto it = modules->find(name);
    if (it != modules->end())
    {
        return &(it->second->GetFullIdentifier());
    }
    else
        return nullptr;
}

}
