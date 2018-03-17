#include "Factory.h"

#include "ModuleWrapper.h"
#include "OS_dependent.h"

Factory::Factory()
{
}

Factory::ErrorCode Factory::TryToLoadLibrary(const std::string& filename)
{
    auto lib = std::shared_ptr<ManagedLibrary>(new ManagedLibrary(filename));
    const auto instertion = libraries.emplace(get_file_name(filename), lib);
    const ErrorCode error(*(instertion.first->second));
    if (instertion.second == false)
        return Duplicate;
    else if (!*(instertion.first->second))
        libraries.erase(instertion.first);
    return error;
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

        wrapper->state = ModuleWrapper::State::Initialized;
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

std::pair<ModuleWrapper*, Factory::ErrorCode> Factory::InstantiateModule(const ModuleIdentifier& moduleId)
{
    std::pair<ModuleWrapper*, ErrorCode> result(nullptr, CannotInstantiate);

	if (moduleId.library.empty())
	{// find out which library can instantiate it
		for (const auto& f : libraries)
		{
            const auto& functions = f.second->GetFunctions();
            const auto thisId = ModuleIdentifier(moduleId.name, moduleId.instance, f.first);
			if (result.first = TryToInstantiate(thisId, functions))
			{
                result.second = Success;
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
				result.second = Success;
			else
				result.second = CannotInstantiateByLibrary;
        }
        else if ((result.second = TryToLoadLibrary(moduleId.library)) <= Duplicate)
        { // try to load from never-seen library
            const auto libraryName = get_file_name(moduleId.library);
            const ModuleIdentifier thisId(moduleId.name, moduleId.instance, libraryName);
            if (result.first = TryToInstantiate(thisId, libraries[libraryName]->GetFunctions()))
                result.second = Success;
            else
                result.second = CannotInstantiateByLibrary;
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

Factory::ManagedLibrary::ManagedLibrary(const std::string & filename)
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
            error = Factory::Success;
        }
        else
        {
            error = Factory::NotALibrary;
        }
    }
    else
    {
        error = Factory::CannotOpen;
    }
}

Factory::ManagedLibrary::~ManagedLibrary()
{
    if (handle)
        close_library(handle);
}

Factory::ManagedLibrary::operator Factory::ErrorCode() const
{
    return error;
}

Factory::ManagedLibrary::operator bool()const
{
    return error == Factory::Success;
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
