#include "ModuleWrapper.h"

#include "Factory.h"
#include <iostream>

ModuleWrapper::~ModuleWrapper(void)
{
}

ModuleWrapper::ModuleWrapper(slag::Module* m)
{
	reset(m);
}

bool ModuleWrapper::Initialize( cv::FileNode node, const Factory& factory)
{
	if (!node["Name"].isString())
	{
		std::cerr << "Module name should be given!" << std::endl;
		return false;
	}

	std::string name = node["Name"];
	
	identifier.assign(name.c_str());
	//Instantiate
	reset(factory.InstantiateModule(identifier));

	if (get() == nullptr)
	{
		std::cerr << "Instantiation of Module \"" << name << "\" has been failed!" << std::endl;
	}else
	{
		settings.push_back(identifier);
		auto settingsNode = node["Settings"];
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
		std::vector<const char*> settings_array;
		for (const auto& setting : settings)
			settings_array.push_back(setting.c_str());
		//Initialize
		if (!(get()->Initialize(settings_array.size(), settings_array.data())))
		{
			std::cerr << "Initialization of Module \"" << (std::string)identifier << "\" has been failed!" << std::endl;
			release();
		}else
		{
			return true;
		}
	}
	return false;
}
