#include "Factory.h"

#include <windows.h>
#include <iostream>
#include <mutex>
#include <memory>

#include "ModuleIdentifier.h"

std::vector<std::string> Factory::EnlistFiles( const std::string& path )
{
	std::vector<std::string> result;

	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;

	hFind = FindFirstFile(path.c_str(), &FindFileData);

	if (hFind != INVALID_HANDLE_VALUE)
	{
		do{
			result.emplace_back(FindFileData.cFileName);
		}while (FindNextFile(hFind, &FindFileData));
		FindClose(hFind);
	}

	return result;
}

Factory::Factory()
{
	HMODULE hndl;

	auto files = EnlistFiles(std::string("*")+extension);

	for (const auto& dll : files)
	{
		hndl = LoadLibrary(dll.c_str());
		if (hndl)
		{
			auto pModuleFactory = (slag::ModuleFactoryFunction) GetProcAddress(hndl, "InstantiateModule");
			if (pModuleFactory)
			{
				// cut the extension off
				pModuleFactories[dll.substr(0, dll.size()-extension.size())]= pModuleFactory;
				module_dll_handles.push_back(hndl);
			}else
			{
				//none of my business
				FreeLibrary(hndl);
			}
		}
	}
}

slag::Module* Factory::InstantiateModule(const ModuleIdentifier& moduleId)const
{
	slag::Module* result = nullptr;

	if (moduleId.dll.empty())
	{// find out which dll can instantiate it
		for (const auto& f : pModuleFactories)
		{
			//instantiate module
			auto module = (f.second)(moduleId.name.c_str(), moduleId.instance.c_str());
			if (result && module)
			{
				std::cerr << "Module \"" << (std::string)moduleId << "\" appears more than once! Found again in \"" << f.first << "\"" << std::endl;
				std::cerr << "Using first match instead" << std::endl;
				//throw out duplicated module
				std::shared_ptr<slag::Module> garbageCollector(module);
			}else if (module)
			{
				result = module;
				//moduleId.dll = f.first;
			}
		}
	}else
	{
		auto moduleFactory = pModuleFactories.find(moduleId.dll);
		if (moduleFactory != pModuleFactories.end())
		{
			auto module = (moduleFactory->second)(moduleId.name.c_str(), moduleId.instance.c_str());
			if (module)
			{
				result = module;
			}
		}
	}
	//may be null
	return result;
}

Factory::~Factory()
{
	for (auto hndl : module_dll_handles)
		FreeLibrary(hndl);
}

const std::string Factory::extension = ".dll";

static std::mutex mtx;
