#pragma once

#include <string>
#include <functional>
#include <map>

#include "slag_interface.h"
#include "ModuleIdentifier.h"

typedef std::function<void(const std::string&, const char*, int)> output_text_callback;
typedef std::function<void(const std::string&, double cycle, double load, double wait, const std::map<PortNumber, size_t>&)> statistics_callback;
typedef std::function<void(const std::string&, int, int, ImageType, const unsigned char*)> output_image_callback;

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
