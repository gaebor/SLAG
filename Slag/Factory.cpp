#include "Factory.h"

#include <iostream>

#include "ModuleWrapper.h"
#include "OS_dependent.h"

Factory::Factory()
{
}

bool Factory::TryToLoadLibrary(std::ostream& os, const std::string& filename)
{
	os << "Loading library from \"" << filename << "\" ... ";
	os.flush();

    if (module_dll_handles.find(filename) == module_dll_handles.end() ||
        module_dll_handles[filename] == nullptr)
    {
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
                os << "loaded as \"" << library_name << '"';
                os.flush();

                auto& f = pModuleFunctions[library_name];

                f.instantiate = instantiate;
                f.compute = compute;
                f.deleteModule = deleteModule;
                f.deleteMsg = deleteMsg;

                module_dll_handles[filename] = hndl;
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
    }else
    {
        os << "already loaded";
    }
    return false;
}

ModuleWrapper* Factory::TryToInstantiate(const ModuleIdentifier& moduleId, const Factory::Functions& f)
{
    const auto module = (f.instantiate)(moduleId.name.c_str(), moduleId.instance.c_str());
    if (module)
    {
        auto wrapper = new ModuleWrapper();

        wrapper->_module.reset(module, f.deleteModule);
        wrapper->deleteMsg = f.deleteMsg;
        wrapper->compute = f.compute;
        wrapper->initialize = f.initialize;

        wrapper->identifier = moduleId;

        return wrapper;
    }
    return nullptr;
}

void Factory::Scan()
{
    auto files = enlist_libraries();

    for (const auto& filename : files)
    {
        TryToLoadLibrary(std::cout, filename);
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

std::pair<ModuleWrapper*, Factory::ErrorCode> Factory::InstantiateModule(const ModuleIdentifier& moduleId)
{
    std::pair<ModuleWrapper*, ErrorCode> result(nullptr, CannotInstantiate);

	if (moduleId.library.empty())
	{// find out which library can instantiate it
		for (const auto& f : pModuleFunctions)
		{
            const auto& functions = f.second;
            ModuleIdentifier thisId(moduleId.name, moduleId.instance, f.first);
			if (result.first = TryToInstantiate(thisId, functions))
			{
                result.second = Success;
                break;
			}
		}
	}else
	{ // try with a specific library
		auto moduleFactory = pModuleFunctions.find(moduleId.library);
		if (moduleFactory != pModuleFunctions.end())
		{
            const auto functions = moduleFactory->second;
            if (result.first = TryToInstantiate(moduleId, functions))
				result.second = Success;
			else
				result.second = CannotInstantiateByLibrary;
		}else if (TryToLoadLibrary(std::cout, moduleId.library))
		{ // try to load from never-seen library
			std::cout << ", ";
            auto thisId = moduleId;
            thisId.library = get_file_name(moduleId.library);
			if (result.first = TryToInstantiate(thisId, pModuleFunctions[thisId]))
				result.second = Success;
			else
				result.second = CannotInstantiateByLibrary;
		}
		else
		{
			std::cout << ", ";
			result.second = NoSuchLibrary;
		}
	}
	return result;
}

Factory::~Factory()
{
	for (auto& hndl : module_dll_handles)
		close_library(hndl.second);
    module_dll_handles.clear();
}

Factory::Functions::Functions()
	:instantiate(NULL), compute(NULL), deleteMsg(NULL), deleteModule(NULL), initialize(NULL)
{
}
