#include "Factory.h"

#include <iostream>
#include <mutex>
#include <memory>

#include "ModuleIdentifier.h"
#include "ModuleWrapper.h"


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
		hndl = LoadLibraryA(dll.c_str());
		if (hndl)
		{
			auto instantiate = (slag::SlagInstantiate)GetProcAddress(hndl, "SlagInstantiate");
			auto deleteMsg = (slag::SlagDestroyMessage)GetProcAddress(hndl, "SlagDestroyMessage");
			auto deleteModule = (slag::SlagDestroyModule)GetProcAddress(hndl, "SlagDestroyModule");
			auto compute = (slag::SlagCompute)GetProcAddress(hndl, "SlagCompute");
			auto initialize = (slag::SlagInitialize)GetProcAddress(hndl, "SlagInitialize");

			if (instantiate != NULL && deleteMsg != NULL && deleteModule != NULL && compute != NULL)
			{
				// cut the extension off
				auto& f = pModuleFunctions[dll.substr(0, dll.size()-extension.size())];
				f.instantiate = instantiate;
				f.compute = compute;
				f.deleteModule = deleteModule;
				f.deleteMsg = deleteMsg;

				module_dll_handles.push_back(hndl);
				if (initialize != NULL)
					f.initialize = initialize;
			}else
			{
				//none of my business
				FreeLibrary(hndl);
			}
		}
	}
}

Factory::ErrorCode Factory::InstantiateModule(ModuleWrapper& moduleWrapper)const
{
	ErrorCode result = CannotInstantiate;
	auto& moduleId = moduleWrapper.identifier;

	if (moduleId.dll.empty())
	{// find out which dll can instantiate it
		for (const auto& f : pModuleFunctions)
		{
			//instantiate module
			auto module = (f.second.instantiate)(moduleId.name.c_str(), moduleId.instance.c_str(), &moduleWrapper.output_text_raw, &moduleWrapper.output_image_raw, &moduleWrapper.output_image_width, &moduleWrapper.output_image_height);
			if (module)
			{
				result = Success;
				if (moduleWrapper._module)
				{
					result = Duplicate;
					ManagedModule garbageCollector(f.second.deleteModule);
					garbageCollector.reset(module);
				}else
				{
					moduleWrapper._module = module;
					moduleWrapper.deleteModule = f.second.deleteModule;
					moduleWrapper.deleteMsg = f.second.deleteMsg;
					moduleWrapper.compute = f.second.compute;
					moduleWrapper.initialize = f.second.initialize;

					moduleId.actual_dll = f.first;
				}
			}
		}
	}else
	{
		auto moduleFactory = pModuleFunctions.find(moduleId.dll);
		if (moduleFactory != pModuleFunctions.end())
		{
			auto module = (moduleFactory->second.instantiate)(moduleId.name.c_str(), moduleId.instance.c_str(), &moduleWrapper.output_text_raw, &moduleWrapper.output_image_raw, &moduleWrapper.output_image_width, &moduleWrapper.output_image_height);
			if (module)
			{
				moduleWrapper._module = module;
				moduleWrapper.deleteModule = moduleFactory->second.deleteModule;
				moduleWrapper.deleteMsg = moduleFactory->second.deleteMsg;
				moduleWrapper.compute = moduleFactory->second.compute;
				moduleWrapper.initialize = moduleFactory->second.initialize;

				moduleId.actual_dll = moduleFactory->first;
				result = Success;
			}else
				result = CannotInstantiateByLibrary;
		}else
			result = NoSuchLibrary;
	}
	return result;
}

Factory::~Factory()
{
	for (auto hndl : module_dll_handles)
		FreeLibrary(hndl);
}

std::vector<std::string> Factory::ReadSettings( cv::FileNode& settingsNode )
{
	std::vector<std::string> settings;
	if (settingsNode.isString())
	{
		settings.push_back(settingsNode);
	}else if (settingsNode.isSeq())
	{
		for (auto settingNode : settingsNode)
			settings.push_back(settingNode);
	}else if(settingsNode.isMap())
	{
		for (auto settingNode : settingsNode)
		{
			settings.push_back(settingNode.name());
			settings.push_back(settingNode);
		}
	}
	return settings;
}

const std::string Factory::extension = ".dll";

static std::mutex mtx;
