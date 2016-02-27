#include "Factory.h"

#include <iostream>

#include "ModuleWrapper.h"
#include "OS_dependent.h"

Factory::Factory()
{
	void* hndl;

	auto files = enlist_libraries();

	for (const auto& library : files)
	{
		hndl = load_library(library.c_str());
		if (hndl != nullptr)
		{
			auto instantiate = (SlagInstantiate_t)get_symbol_from_library(hndl, "SlagInstantiate");
			auto deleteMsg = (SlagDestroyMessage_t)get_symbol_from_library(hndl, "SlagDestroyMessage");
			auto deleteModule = (SlagDestroyModule_t)get_symbol_from_library(hndl, "SlagDestroyModule");
			auto compute = (SlagCompute_t)get_symbol_from_library(hndl, "SlagCompute");
			auto initialize = (SlagInitialize_t)get_symbol_from_library(hndl, "SlagInitialize");

			if (instantiate != NULL && deleteMsg != NULL && deleteModule != NULL && compute != NULL)
			{
				// cut the extension off
				auto& f = pModuleFunctions[get_file_name(library)];
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
				close_library(hndl);
			}
		}
	}
	for (auto l : pModuleFunctions)
	{
		std::cout << l.first << std::endl;
	}
}

Factory::ErrorCode Factory::InstantiateModule(ModuleWrapper& moduleWrapper)const
{
	ErrorCode result = CannotInstantiate;
	auto& moduleId = moduleWrapper.identifier;

	if (moduleId.library.empty())
	{// find out which library can instantiate it
		for (const auto& f : pModuleFunctions)
		{
			//instantiate module
			auto module = (f.second.instantiate)(
				moduleId.name.c_str(),
				moduleId.instance.c_str(),
				&moduleWrapper.output_text_raw,
				&moduleWrapper.output_image_raw,
				&moduleWrapper.output_image_width,
				&moduleWrapper.output_image_height,
				moduleWrapper.imageType);

			if (module)
			{
				result = Success;
				if (moduleWrapper._module)
				{
					result = Duplicate;
					ManagedModule garbageCollector(module, f.second.deleteModule);
				}else
				{
					moduleWrapper._module = module;
					moduleWrapper.deleteModule = f.second.deleteModule;
					moduleWrapper.deleteMsg = f.second.deleteMsg;
					moduleWrapper.compute = f.second.compute;
					moduleWrapper.initialize = f.second.initialize;

					moduleId.library = f.first;
				}
			}
		}
	}else
	{
		auto moduleFactory = pModuleFunctions.find(moduleId.library);
		if (moduleFactory != pModuleFunctions.end())
		{
			auto module = (moduleFactory->second.instantiate)(
				moduleId.name.c_str(),
				moduleId.instance.c_str(),
				&moduleWrapper.output_text_raw,
				&moduleWrapper.output_image_raw,
				&moduleWrapper.output_image_width,
				&moduleWrapper.output_image_height,
				moduleWrapper.imageType);

			if (module)
			{
				moduleWrapper._module = module;
				moduleWrapper.deleteModule = moduleFactory->second.deleteModule;
				moduleWrapper.deleteMsg = moduleFactory->second.deleteMsg;
				moduleWrapper.compute = moduleFactory->second.compute;
				moduleWrapper.initialize = moduleFactory->second.initialize;

				moduleId.library = moduleFactory->first;
				result = Success;
			}else
				result = CannotInstantiateByLibrary;
		}else
			result = NoSuchLibrary;
	}
	moduleWrapper.printableName = moduleWrapper.identifier;
	return result;
}

Factory::~Factory()
{
	for (auto hndl : module_dll_handles)
		close_library(hndl);
}

std::vector<std::string> Factory::ReadSettings(const Poco::Util::AbstractConfiguration* settingsNode)
{
	std::vector<std::string> settings;
	if (settingsNode)
	{
		Poco::Util::AbstractConfiguration::Keys keys;
		settingsNode->keys(keys);
		for (auto k : keys)
		{
			settings.push_back(k);
			settings.push_back(settingsNode->getString(k));
		}
	}
	//if (settingsNode.isString())
	//{
	//	settings.push_back(settingsNode);
	//}else if (settingsNode.isSeq())
	//{
	//	for (auto settingNode : settingsNode)
	//		settings.push_back(settingNode);
	//}else if(settingsNode.isMap())
	//{
	//	for (auto settingNode : settingsNode)
	//	{
	//		settings.push_back(settingNode.name());
	//		settings.push_back(settingNode);
	//	}
	//}
	return settings;
}
