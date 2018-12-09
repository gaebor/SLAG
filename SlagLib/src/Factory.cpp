#include "Factory.h"

#include "ModuleWrapper.h"
#include "OS_dependent.h"

namespace slag {

Factory::Factory()
    : libraries()
{
}

ErrorCode Factory::TryToLoadLibrary(const std::string& filename)
{
    auto lib = new LibraryWrapper(filename);
    const auto instertion = libraries.emplace(get_file_name(filename), ManagedLibrary(lib));
    const ErrorCode error(*(instertion.first->second));
    if (instertion.second == false)
        return Duplicate;
    else if (!*(instertion.first->second))
        libraries.erase(instertion.first);
    return error;
}

ModuleWrapper* Factory::TryToInstantiate(const FullModuleIdentifier& moduleId, const Factory::Functions& f)
{
    const auto module = (f.instantiate)(moduleId.module.name.c_str(), moduleId.module.instance.c_str());
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
    for (const auto& filename : enlist_libraries())
        TryToLoadLibrary(filename);
}

std::vector<std::string> Factory::GetLibraries() const
{
    std::vector<std::string> result;
    for (const auto& lib : libraries)
    {
        result.emplace_back(lib.first);
    }
    return result;
}

std::pair<ModuleWrapper*, ErrorCode> Factory::InstantiateModule(const std::string& description)
{
    std::pair<ModuleWrapper*, ErrorCode> result(nullptr, CannotInstantiate);

    FullModuleIdentifier moduleId(description.c_str());
	if (moduleId.library.empty())
	{// find out which library can instantiate it
		for (const auto& f : libraries)
		{
            const auto& functions = f.second->GetFunctions();
            const auto thisId = FullModuleIdentifier(moduleId.module.name, moduleId.module.instance, f.first);
			if (result.first = TryToInstantiate(thisId, functions))
			{
                result.second = ErrorCode::Success;
                break;
			}
		}
	}else
	{ // try with a specific library
		auto library = libraries.find(moduleId.library);
		if (library != libraries.end())
		{
            const auto functions = library->second->GetFunctions();
            if (result.first = TryToInstantiate(moduleId, functions))
				result.second = ErrorCode::Success;
			else
				result.second = ErrorCode::CannotInstantiateByLibrary;
        }
        else if ((result.second = TryToLoadLibrary(moduleId.library)) <= Duplicate)
        { // try to load from never-seen library
            const auto libraryName = get_file_name(moduleId.library);
            const FullModuleIdentifier thisId(moduleId.module.name, moduleId.module.instance, libraryName);
            if (result.first = TryToInstantiate(thisId, libraries[libraryName]->GetFunctions()))
                result.second = ErrorCode::Success;
            else
                result.second = ErrorCode::CannotInstantiateByLibrary;
        }
	}
	return result;
}

Factory::~Factory()
{
    libraries.clear();
}

Factory::Functions::Functions()
	:instantiate(NULL), compute(NULL), deleteMsg(NULL), deleteModule(NULL), initialize(NULL)
{
}

Factory::LibraryWrapper::LibraryWrapper(const std::string & filename)
    : handle(load_library(filename.c_str())),
    error(NotALibrary)
{
    if (handle != nullptr)
    {
        functions.instantiate = (SlagInstantiate_t)get_symbol_from_library(handle, "SlagInstantiate");
        functions.deleteMsg = (SlagDestroyMessage_t)get_symbol_from_library(handle, "SlagDestroyMessage");
        functions.deleteModule = (SlagDestroyModule_t)get_symbol_from_library(handle, "SlagDestroyModule");
        functions.compute = (SlagCompute_t)get_symbol_from_library(handle, "SlagCompute");
        functions.initialize = (SlagInitialize_t)get_symbol_from_library(handle, "SlagInitialize");

        if (functions.instantiate != NULL && functions.compute != NULL &&
            functions.deleteMsg != NULL && functions.deleteModule != NULL)
        {
            error = ErrorCode::Success;
        }
        else
        {
            error = ErrorCode::NotALibrary;
        }
    }
    else
    {
        error = ErrorCode::CannotOpen;
    }
}

Factory::LibraryWrapper::~LibraryWrapper()
{
    if (handle)
        close_library(handle);
}

Factory::LibraryWrapper::operator ErrorCode() const
{
    return error;
}

Factory::LibraryWrapper::operator bool()const
{
    return error == ErrorCode::Success;
}

const Factory::Functions & Factory::LibraryWrapper::GetFunctions() const
{
    return functions;
}

Factory::Functions& Factory::Functions::operator=(const Factory::Functions& other)
{
    instantiate = other.instantiate;
    compute = other.compute;
    deleteMsg = other.deleteMsg;
    deleteModule = other.deleteModule;
    initialize = other.initialize;

    return *this;
}

}
