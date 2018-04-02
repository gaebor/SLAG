#pragma once

#include <memory>
#include <mutex>



typedef std::shared_ptr<void> ManagedMessage;
typedef std::shared_ptr<void> ManagedModule;

typedef std::lock_guard<std::mutex> AutoLock;

enum ErrorCode : unsigned char
{
    Success, //!< everything OK
    Duplicate, //!< more than one library was able to instantiate the requested module, the first one was used
    AlreadyExists, //!< nothing to do
    WrongArguments, //!< wrong usage
    CannotInitialize, //!< module's fault
    CannotInstantiate, //!< no library could instantiate your module
    CannotOpen, //!<< the requested library does not exists
    NotALibrary, //!<< the requested file exists but not a Slag Library
    CannotInstantiateByLibrary //!<< the requested library couldn't instantiate your module
};
