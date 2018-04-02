#pragma once

#include <map>
#include <list>

#include "Factory.h"
#include "ModuleWrapper.h"

class Graph
{
public:
    Graph();
    ~Graph();

    ErrorCode AddModule(std::vector<std::string> arguments);
    ErrorCode AddConnection(const std::string& from, const std::string& to, MessageQueue::LimitBehavior behavior = MessageQueue::None);
private:
    std::map<ModuleIdentifier, std::unique_ptr<ModuleWrapper>> modules;
    std::list<std::unique_ptr<MessageQueue>> messageQueues;

    Factory factory;

    bool run;
};
