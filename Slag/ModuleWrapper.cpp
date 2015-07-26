#include "ModuleWrapper.h"

#include <iostream>
#include <set>
#include <algorithm>

#include "Factory.h"
#include "Timer.h"
#include "HumanReadable.h"

ModuleWrapper::~ModuleWrapper(void)
{
}

ModuleWrapper::ModuleWrapper(slag::Module* m)
	: inputPortLength(0)
{
	_module.reset(m);
}

bool ModuleWrapper::Initialize( cv::FileNode node)
{
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
		return _module->Initialize(settings_array.size(), settings_array.data());
	}
	return false;
}

void ModuleWrapper::ThreadProcedure()
{
	std::vector<slag::Message*> inputMessages(inputPortLength, nullptr);
	slag::Message** outputMessages_raw;

	std::vector<ManagedMessage> receivedMessages;
	
	//TODO more reference counting
	Timer timer;
	while (true)
	{
		//manage input data
		for (auto& q : inputQueues)
		{
			ManagedMessage input;

			if (!q.second->DeQueue(input))
				return;

			inputMessages[q.first] = input.get();
			receivedMessages.push_back(input);
		}
		
		PortNumber outputNumber;
		diffTime = timer.Tock();
		timer.Tick();
		outputMessages_raw = _module->Compute(inputMessages.data(), inputMessages.size(), &outputNumber);
		computeTime = timer.Tock();

		//manage output data
		if (outputMessages_raw == nullptr)
			break; //TODO handle halt signals

		auto outputIt = outputQueues.begin();
		for (PortNumber i = 0; i < outputNumber; ++i)
		{
			auto messagePtr = outputMessages_raw[i];
			ManagedMessage managedOutput;
			auto inputIt = std::find_if(receivedMessages.begin(), receivedMessages.end(), [&](const ManagedMessage& m){return m.get()==messagePtr;});
			if (inputIt != receivedMessages.end())
			{
				managedOutput = *inputIt;
			}else
			{
				managedOutput.reset(messagePtr);
			}

			outputIt = outputQueues.find(i);
			if (outputIt != outputQueues.end())
			{
				for (auto out : outputIt->second)
				{
					out->EnQueue(managedOutput);
				}
			}
		}
		receivedMessages.clear();

		//visualization
		for (const auto& q : inputQueues)
		{
			bufferSize[q.first] = q.second->GetSize();
		}

		if (_module->outputText != nullptr)
		{
			output_text = _module->outputText;
			//std::cerr << (std::string)identifier << ": " << output_text << " (call interval: " << diffTime << ", compute time: " << computeTime << ")\n";
		}

		size_t picure_size = 0;
		if (_module->outputPicture.imageInfo != nullptr && (picure_size = _module->outputPicture.width * _module->outputPicture.height) > 0)
			output_image.assign(_module->outputPicture.imageInfo, _module->outputPicture.imageInfo+picure_size);

	}
	std::cerr << (std::string)identifier << ": " << output_text << " (call interval: " << diffTime << ", compute time: " << computeTime << ")" <<std::endl;

}
