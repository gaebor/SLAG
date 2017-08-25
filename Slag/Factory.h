#ifndef INCLUDE_MODULE_FACTORY_H
#define INCLUDE_MODULE_FACTORY_H

#include <vector>
#include <string>
#include <map>

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
public:
	ErrorCode InstantiateModule(ModuleWrapper& moduleWrapper)const;

	struct Functions
	{
		Functions() :instantiate(NULL), compute(NULL), deleteMsg(NULL), deleteModule(NULL), initialize(NULL){}
		SlagInstantiate_t instantiate;
		SlagCompute_t compute;
		SlagDestroyMessage_t deleteMsg;
		SlagDestroyModule_t deleteModule;
		SlagInitialize_t initialize;
	};

private:
	std::map<std::string, Functions> pModuleFunctions;
	std::vector<void*> module_dll_handles;
};

#endif //INCLUDE_MODULE_FACTORY_H
