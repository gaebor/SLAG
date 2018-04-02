#include "Graph.h"

Graph::Graph()
:   run(true)
{
}

Graph::~Graph()
{
}

ErrorCode Graph::AddModule(std::vector<std::string> arguments)
{
    if (arguments.empty())
        return WrongArguments;

    std::string moduleName = arguments[0];
    if (moduleName.empty())
        return WrongArguments;
    
    arguments.erase(arguments.begin());

    const ModuleIdentifier moduleId(moduleName.c_str());

    if (modules.find(moduleId) != modules.end())
        return AlreadyExists;

    const auto result = factory.InstantiateModule(moduleId);
    switch (result.second)
    {
    case ErrorCode::Duplicate:
        return ErrorCode::Duplicate;
    case ErrorCode::Success:
    {
        modules[moduleId].reset(result.first);

        if (!modules[moduleId]->Initialize(arguments))
            return ErrorCode::CannotInitialize;
        else
            return ErrorCode::Success;
    }
    }
    return result.second;
}

ErrorCode Graph::AddConnection(
                const std::string & from, const std::string & to,
                MessageQueue::LimitBehavior behavior)
{
    const PortIdentifier fromModuleId(from);
    const PortIdentifier toModuleId(to);

    const auto fromModulePtr = modules.find(fromModuleId.module);
    const auto toModulePtr = modules.find(toModuleId.module);

    if (fromModulePtr == modules.end())
        return ErrorCode::WrongArguments;

    if (toModulePtr == modules.end())
        return ErrorCode::WrongArguments;

    messageQueues.emplace_back(new MessageQueue());
    auto newMessageQueue = messageQueues.back().get();
    newMessageQueue->limitBehavior = behavior;
    newMessageQueue->queueLimit = behavior;

    const auto ok1 = toModulePtr->second->ConnectToInputPort(toModuleId.port, newMessageQueue);
    if (ok1)
    {
        const auto ok2 = fromModulePtr->second->ConnectOutputPortTo(fromModuleId.port, newMessageQueue);
        if (ok2)
        {
            return ErrorCode::Success;
        } else
            toModulePtr->second->RemoveInputPort(toModuleId.port);
    }
}
