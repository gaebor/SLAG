#include "Factory.h"

#include <iostream>

#include "ModuleWrapper.h"
#include "OS_dependent.h"

Factory::Factory()
{
	auto files = enlist_libraries();

	for (const auto& filename : files)
	{
		TryToLoadLibrary(std::cout, filename);
		std::cout << std::endl;
	}
}

bool Factory::TryToLoadLibrary(std::ostream& os, const std::string& filename)
{
	os << "Loading library from \"" << filename << "\" ... ";
	os.flush();

	const std::string library_name = get_file_name(filename);
	
	auto const hndl = load_library(filename.c_str());

	if (hndl != nullptr)
	{
		auto instantiate = (SlagInstantiate_t)get_symbol_from_library(hndl, "SlagInstantiate");
		auto deleteMsg = (SlagDestroyMessage_t)get_symbol_from_library(hndl, "SlagDestroyMessage");
		auto deleteModule = (SlagDestroyModule_t)get_symbol_from_library(hndl, "SlagDestroyModule");
		auto compute = (SlagCompute_t)get_symbol_from_library(hndl, "SlagCompute");
		auto initialize = (SlagInitialize_t)get_symbol_from_library(hndl, "SlagInitialize");

		if (instantiate != NULL && deleteMsg != NULL && deleteModule != NULL && compute != NULL)
		{
			os << "loaded as " << library_name;
			os.flush();

			auto& f = pModuleFunctions[library_name];

			f.instantiate = instantiate;
			f.compute = compute;
			f.deleteModule = deleteModule;
			f.deleteMsg = deleteMsg;

			module_dll_handles.push_back(hndl);
			if (initialize != NULL)
				f.initialize = initialize;
			return true;
		}
		else
		{
			//none of my business
			os << "SLAG interface not found";
			os.flush();
			close_library(hndl);
		}
	}
	else
	{
		os << "cannot be opened";
		os.flush();
	}
	return false;
}

bool Factory::TryToInstantiate(ModuleWrapper& moduleWrapper, const Functions& f)
{
	if (moduleWrapper._module && moduleWrapper.deleteModule) // already a module
	{
		ManagedModule garbageCollector(moduleWrapper._module, moduleWrapper.deleteModule);
	}

	const auto& moduleId = moduleWrapper.identifier;

	const auto module = (f.instantiate)(
		moduleId.name.c_str(),
		moduleId.instance.c_str());
	
	if (module)
	{
		moduleWrapper._module = module;
		moduleWrapper.deleteModule = f.deleteModule;
		moduleWrapper.deleteMsg = f.deleteMsg;
		moduleWrapper.compute = f.compute;
		moduleWrapper.initialize = f.initialize;

		return true;
	}
	return false;
}

Factory::ErrorCode Factory::InstantiateModule(ModuleWrapper& moduleWrapper)
{
	ErrorCode result = CannotInstantiate;
	auto& moduleId = moduleWrapper.identifier;

	if (moduleId.library.empty())
	{// find out which library can instantiate it
		for (const auto& f : pModuleFunctions)
		{
			//instantiate module
			if (TryToInstantiate(moduleWrapper, f.second))
			{
				moduleWrapper.identifier.library = f.first;
				result = Success;
				break;
			}
		}
	}else
	{ // try with a specific library
		auto moduleFactory = pModuleFunctions.find(moduleId.library);
		if (moduleFactory != pModuleFunctions.end())
		{
			if (TryToInstantiate(moduleWrapper, moduleFactory->second))
				result = Success;
			else
				result = CannotInstantiateByLibrary;
		}else if (TryToLoadLibrary(std::cout, moduleId.library))
		{ // try to load from never-seen library
			std::cout << ", ";
			moduleId.library = get_file_name(moduleId.library);
			if (TryToInstantiate(moduleWrapper, pModuleFunctions.find(moduleId.library)->second))
				result = Success;
			else
				result = CannotInstantiateByLibrary;
		}
		else
		{
			std::cout << ", ";
			result = NoSuchLibrary;
		}
	}
	moduleWrapper.printableName = moduleWrapper.identifier;
	return result;
}

Factory::~Factory()
{
	for (auto hndl : module_dll_handles)
		close_library(hndl);
}

Factory::Functions::Functions()
	:instantiate(NULL), compute(NULL), deleteMsg(NULL), deleteModule(NULL), initialize(NULL)
{
}
