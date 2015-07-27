#include "ModuleWrapper.h"

#include <iostream>
#include <set>
#include <algorithm>

#include "Timer.h"
#include "HumanReadable.h"

ModuleWrapper::~ModuleWrapper(void)
{
	delete _module;
}

ModuleWrapper::ModuleWrapper()
:	inputPortLength(0),
	diffTime(std::make_pair(0.0, 0.0)),
	_module(nullptr),
	output_image_raw(nullptr),
	output_text_raw(nullptr),
	output_image_width(0), output_image_height(0)
{
}

bool ModuleWrapper::Initialize( cv::FileNode node)
{
	if (_module != nullptr)
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
	double prevTime = 0.0;
	
	while (true)
	{
		//manage input data
		for (auto& q : inputQueues)
		{
			ManagedMessage input;

			if (!q.second->DeQueue(input))
				goto halt;

			inputMessages[q.first] = input.get();
			receivedMessages.push_back(input);
		}
		
		PortNumber outputNumber;
		diffTime.Modify([&](std::pair<double, double>& self)
		{
			self.first = timer.Tock();
			timer.Tick();
			outputMessages_raw = _module->Compute(inputMessages.data(), inputMessages.size(), &outputNumber);
			self.second = prevTime;
		});
		prevTime = timer.Tock();

		//manage output data
		if (outputMessages_raw == nullptr)
		{
			goto halt;
		}

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
		bufferSize.Modify([&](std::map<PortNumber, size_t>& self)
		{
			for (const auto& q : inputQueues)
			{
				self[q.first] = q.second->GetSize();
			}
		});

		if (output_text_raw != nullptr)
		{
			output_text.Modify([&](std::string& self){self = output_text_raw;});
		}

		size_t picure_size = 0;
		if (output_image_raw != nullptr && (picure_size = output_image_width * output_image_height) > 0)
			output_image.Modify([&](std::vector<unsigned char>& self){self.assign(output_image_raw, output_image_raw + picure_size);});

	}
halt:
	diffTime.Modify([&](std::pair<double, double>& self)
	{
		self.first = timer.Tock();
	});
		
	for (auto& qs : outputQueues)
		for (auto& q : qs.second)
			q->WaitForEmpty();
	
	for (auto& qs : outputQueues)
		for (auto& q : qs.second)
			q->WakeUp();

	//output_text.NonEditable();
	//std::cerr << (std::string)identifier << ": " << output_text.Get() << " (call interval: " << diffTime << ", compute time: " << computeTime << ")" <<std::endl;
	//output_text.MakeEditable();
}

bool ModuleWrapper::SetModule( slag::Module* m )
{
	if (_module == nullptr)
	{
		_module = m;
		return true;
	}
	return false;
}

