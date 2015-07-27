#pragma once

#include <vector>
#include <string>
#include <map>

#include "slag/slag_interface.h"
#include "ModuleIdentifier.h"
#include "ModuleWrapper.h"

#include <windows.h>

class Factory
{
private:
	static std::vector<std::string> EnlistFiles(const std::string& path);
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

private:
	static const std::string extension;
	std::map<std::string, slag::ModuleFactoryFunction> pModuleFactories;
	std::vector<HMODULE> module_dll_handles;
};