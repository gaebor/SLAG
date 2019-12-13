#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <vector>

#include "slag_interface.h"
#include "slag/Types.h"
#include "slag/Identifiers.h"
#include "InternalTypes.h"

namespace slag {

class ModuleWrapper;

class Factory
{
public:
    Factory();
    ~Factory();

    struct Functions
    {
        Functions();

        SlagInstantiate_t instantiate;
        SlagCompute_t compute;
        SlagDestroyMessage_t deleteMsg;
        SlagDestroyModule_t deleteModule;
        SlagInitialize_t initialize;
        SlagHelp_t help;
    };
    const char* Help(const std::string& library_name, int argc, const char** argv)const;

    void Scan();
    std::vector<std::string> GetLibraries()const;
    std::pair<ModuleWrapper*, ErrorCode> InstantiateModule(const std::string& description);

private:
    struct LibraryWrapper
    {
        LibraryWrapper(const std::string& filename);
        ~LibraryWrapper();

        operator ErrorCode()const;
        operator bool()const;
        const Functions& GetFunctions()const;

    private:
        void* handle;
        Functions functions;
        ErrorCode error;
    };
    typedef std::unique_ptr<LibraryWrapper> ManagedLibrary;
    std::unordered_map<std::string, ManagedLibrary> libraries;
	
	ErrorCode TryToLoadLibrary(const std::string& filename);
    ModuleWrapper* TryToInstantiate(const FullModuleIdentifier& moduleId, const Functions& f);
};

}
