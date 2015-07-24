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

public:
	slag::Module* InstantiateModule(const ModuleIdentifier& moduleId)const;

private:
	static const std::string extension;
	std::map<std::string, slag::ModuleFactoryFunction> pModuleFactories;
	std::vector<HMODULE> module_dll_handles;
};