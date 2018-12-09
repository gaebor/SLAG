#pragma once

#include <functional>

#include "slag_interface.h"
#include "Identifiers.h"

namespace slag {

typedef std::function<void(const ModuleIdentifier&, const char*, int)> output_text_callback;
typedef std::function<void(const ModuleIdentifier&, double cycle, double load, double wait)> statistics_callback;
typedef std::function<void(const PortIdentifier&, size_t buffer_size)> statistics2_callback;
typedef std::function<void(const ModuleIdentifier&, int, int, SlagImageType, const unsigned char*)> output_image_callback;

enum ErrorCode : unsigned char
{
    Success, //!< everything OK
    Duplicate, //!< more than one library was able to instantiate the requested module
    AlreadyExists, //!< nothing to do
    WrongArguments, //!< wrong usage
    CannotInitialize, //!< module's SlagInitialize failed
    CannotInstantiate, //!< no library could instantiate your module
    CannotOpen, //!<< the requested library does not exists
    NotALibrary, //!<< the requested file exists but not a Slag Library
    CannotInstantiateByLibrary //!<< the requested library couldn't instantiate your module
};

}