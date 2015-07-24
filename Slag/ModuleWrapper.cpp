#include "ModuleWrapper.h"

#include <iostream>
#include <set>

#include "Factory.h"
#include "Timer.h"
#include "HumanReadable.h"

ModuleWrapper::~ModuleWrapper(void)
{
}

ModuleWrapper::ModuleWrapper(slag::Module* m)
	: inputPortLength(0), outputPortLength(0)
{
	_module.reset(m);
}

bool ModuleWrapper::Initialize( cv::FileNode node, const Factory& factory)
{
	if (!node["Name"].isString())
	{
		return false;
	}

	std::string name = node["Name"];
	
	identifier.assign(name.c_str());
	//Instantiate
	_module.reset(factory.InstantiateModule(identifier));

	if (_module.get() != nullptr)
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
		if (!(_module.get()->Initialize(settings_array.size(), settings_array.data())))
		{
			_module.release();
		}else
		{
			return true;
		}
	}
	return false;
}

void ModuleWrapper::ThreadProcedure()
{
	std::vector<slag::Message*> inputMessages(inputPortLength);
	slag::Message** outputMessages_raw;
	std::set<slag::Message*> depricatedMessages;
	bufferSize.resize(inputPortLength, 0);

	//TODO more reference counting
	Timer timer;
	while (true)
	{
		depricatedMessages.clear();
		PortNumber holePortNumber = 0;

		//manage input data
		for (auto& q : inputQueues)
		{
			for (int i = holePortNumber; i < q.first; ++i)
			{//fill the holes with nullptr
				inputMessages[i]= nullptr;
			}
			holePortNumber = q.first+1;

			if (!q.second->DeQueue(inputMessages[q.first]))
				return;

			depricatedMessages.insert(inputMessages[q.first]);
			bufferSize[q.first] = q.second->GetSize();
		}
		for (int i = holePortNumber; i < inputPortLength; ++i)
		{//fill with nullptr
			inputMessages[i]= nullptr;
		}

		PortNumber outputNumber;
		diffTime = timer.Tock();
		timer.Tick();
		outputMessages_raw = _module->Compute(inputMessages.data(), inputMessages.size(), &outputNumber);
		computeTime = timer.Tock();

		//manage output data
		if (outputMessages_raw == nullptr)
			return;
		for (PortNumber i = 0; i < outputNumber; ++i)
		{
			auto it = outputQueues.find(i);
			if (it != outputQueues.end())
			{
				it->second->EnQueue(outputMessages_raw[i]);
				depricatedMessages.erase(outputMessages_raw[i]);
			}
			else
				depricatedMessages.insert(outputMessages_raw[i]);
		}
		for (auto m : depricatedMessages)
			delete m;

		//visualization
		if (_module->outputText != nullptr)
		{
			output_text = _module->outputText;
			std::cerr << (std::string)identifier << ": " << output_text << " (call interval: " << diffTime << ", compute time: " << computeTime << ")" <<std::endl;
		}

		size_t picure_size = 0;
		if (_module->outputPicture.imageInfo != nullptr && (picure_size = _module->outputPicture.width * _module->outputPicture.height) > 0)
			output_image.assign(_module->outputPicture.imageInfo, _module->outputPicture.imageInfo+picure_size);

	}

}
