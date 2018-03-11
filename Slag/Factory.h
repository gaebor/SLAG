#pragma once

#include <vector>
#include <string>
#include <map>
#include <ostream>

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
		NoSuchLibrary, //!<< the requested library cannot be found
		CannotInstantiateByLibrary //!<< the requested library couldn't instantiate your module
	};

    void Scan();
public:
	std::pair<ModuleWrapper*, ErrorCode> InstantiateModule(const ModuleIdentifier& id);

	struct Functions
	{
		Functions();
		SlagInstantiate_t instantiate;
		SlagCompute_t compute;
		SlagDestroyMessage_t deleteMsg;
		SlagDestroyModule_t deleteModule;
		SlagInitialize_t initialize;
	};

private:
	std::map<std::string, Functions> pModuleFunctions;
	
	bool TryToLoadLibrary(std::ostream& os, const std::string& filename);
    ModuleWrapper* TryToInstantiate(const ModuleIdentifier& moduleId, const Functions& f);
	std::map<std::string, void*> module_dll_handles;
};
