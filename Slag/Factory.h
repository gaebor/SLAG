#pragma once

#include <string>
#include <map>
#include <memory>

#include "slag/slag_interface.h"
#include "ModuleIdentifier.h"

class ModuleWrapper;

class Factory
{
public:
    Factory();
    ~Factory();

    enum ErrorCode
    {
        Success, //!< module is ready to go
        Duplicate, //!< more than one library was able to instantiate the requested module, the first one was used
        CannotInstantiate, //!< no library could instantiate your module
        CannotOpen, //!<< the requested library does not exists
        NotALibrary, //!<< the requested file exists but not a Slag Library
        CannotInstantiateByLibrary //!<< the requested library couldn't instantiate your module
    };

    struct Functions
    {
        Functions();
        Functions& operator=(const Functions& other);
        SlagInstantiate_t instantiate;
        SlagCompute_t compute;
        SlagDestroyMessage_t deleteMsg;
        SlagDestroyModule_t deleteModule;
        SlagInitialize_t initialize;
    };

    void Scan();

    std::pair<ModuleWrapper*, ErrorCode> InstantiateModule(const ModuleIdentifier& id);

private:
    struct ManagedLibrary
    {
        ManagedLibrary(const std::string& filename);
        ~ManagedLibrary();

        operator ErrorCode()const;
        operator bool()const;
        const Functions& GetFunctions()const
        {
            return functions;
        }
    private:
        void* const handle;
        Functions functions;
        ErrorCode error;
    };
    std::map<std::string, std::shared_ptr<ManagedLibrary>> libraries;
	
	ErrorCode TryToLoadLibrary(const std::string& filename);
    ModuleWrapper* TryToInstantiate(const ModuleIdentifier& moduleId, const Functions& f);
};
