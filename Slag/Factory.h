#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <vector>

#include "slag/slag_interface.h"
#include "ModuleIdentifier.h"
#include "InternalTypes.h"

class ModuleWrapper;

class Factory
{
public:
    Factory();
    ~Factory();

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
    std::vector<std::string> GetLibraries()const;
    std::pair<ModuleWrapper*, ErrorCode> InstantiateModule(const ModuleIdentifier& id);

private:
    struct LibraryWrapper
    {
        LibraryWrapper(const std::string& filename);
        ~LibraryWrapper();

        operator ErrorCode()const;
        operator bool()const;
        const Functions& GetFunctions()const
        {
            return functions;
        }

    private:
        void* handle;
        Functions functions;
        ErrorCode error;
    };
    typedef std::unique_ptr<LibraryWrapper> ManagedLibrary;
    std::unordered_map<std::string, ManagedLibrary> libraries;
	
	ErrorCode TryToLoadLibrary(const std::string& filename);
    ModuleWrapper* TryToInstantiate(const ModuleIdentifier& moduleId, const Functions& f);
};
