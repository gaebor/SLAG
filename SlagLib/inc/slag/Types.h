#pragma once

#include <functional>
#include <vector>
#include <utility>

#include "slag_interface.h"
#include "slag/Identifiers.h"

namespace slag {

struct Stats
{
    Stats() : cycle(1), wait(0), load(0), buffers(){}
    double cycle, wait, load;
    std::vector<std::pair<PortNumber, size_t>> buffers;
};

typedef std::function<void(const ModuleIdentifier&, const SlagTextOut&, const SlagImageOut&, Stats&)> module_callback;

enum ErrorCode : unsigned char
{
    Success, //!< everything OK
    Duplicate, //!< 
    AlreadyExists, //!< nothing to do
    WrongArguments, //!< wrong usage
    NoSuchModule, //!< module not found
    CannotInitialize, //!< module's SlagInitialize failed
    CannotInstantiate, //!< no library could instantiate your module
    CannotOpen, //!< the requested library does not exists
    NotALibrary, //!< the requested file exists but not a Slag Library
    CannotInstantiateByLibrary //!< the requested library couldn't instantiate your module
};

enum StatusCode : unsigned char
{
    UnInitialized, //!< invalid
    Idle, //!< doing nothing
    Initializing, //!< initializing
    Running, //!< running, but the control is at Slag
    Waiting, //!< waiting for input
    Computing, //!< (hardly) working
    Queueing, //!< waiting to output
};

}