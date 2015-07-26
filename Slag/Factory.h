#pragma once
#include <vector>
#include <string>
#include <map>

#include <windows.h>

#include "slag/slag_interface.h"
#include "ModuleIdentifier.h"

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
	std::pair<slag::Module*, ErrorCode> InstantiateModule(ModuleIdentifier& moduleId)const;

private:
	static const std::string extension;
	std::map<std::string, slag::ModuleFactoryFunction> pModuleFactories;
	std::vector<HMODULE> module_dll_handles;
};