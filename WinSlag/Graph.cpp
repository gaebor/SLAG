#include "Graph.h"

Graph::Graph()
:   timeout(0.0)
{
    factory.Scan();
}

Graph::~Graph()
{
    Stop();
    messageQueues.clear();
    modules.clear();
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
        startTime = timer.Tock();
        modules_process = std::thread([&]()
        {
            for (auto& m : modules)
                m.second->Start();

            for (auto& m : modules)
                m.second->Wait();
        });
    }
}

void Graph::SignalStop()
{
    for (auto& m : modules)
        m.second->Stop();
}

void Graph::Stop()
{
    SignalStop();
    for (auto& q : messageQueues)
        q->WakeUp();

    if (modules_process.joinable())
        modules_process.join();

}

void Graph::Wait()
{
    while (IsRunning())
    {
        if (timeout > 0 && startTime + timeout <= timer.Tock())
            SignalStop();
        std::this_thread::yield();
    }
}

bool Graph::IsRunning() const
{
    return modules_process.joinable();
}
