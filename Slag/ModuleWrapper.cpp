#include "ModuleWrapper.h"

#include <iostream>
#include <set>
#include <algorithm>

#include "Factory.h"
#include "Imshow.h"

#include "OS_dependent.h"

ModuleWrapper::~ModuleWrapper(void)
{
	if (_module)
		deleteModule(_module);
}

ModuleWrapper::ModuleWrapper(const bool* run)
:	inputPortLength(0),
	_module(nullptr),
	output_image_raw(nullptr),
	output_text_raw(nullptr),
	output_image_width(0), output_image_height(0),
	do_run(run),
	imageType(ImageType::GREY),
	output_image()
{
}

bool ModuleWrapper::Initialize( cv::FileNode node)
{
	if (_module != nullptr)
	{
		if (initialize == NULL)
			return true;

		auto settings = Factory::ReadSettings(node["Settings"]);
		std::vector<const char*> settings_array;

		for (const auto& setting : settings)
			settings_array.push_back(setting.c_str());

		//Initialize
		return initialize(_module, settings_array.size(), settings_array.data()) == 0;
		// module settings are lost after the module initialize!
		//TODO global settings
	}
	return false;
}

void ModuleWrapper::ThreadProcedure()
{
	std::vector<void*> inputMessages(inputPortLength, nullptr);
	void** outputMessages_raw;

	//Dequeued input messages which haven't been Enqueued to the output yet
	std::vector<ManagedMessage> receivedMessages;
	
	//TODO more reference counting
	Timer timer;
	double prevTime = 0.0, diffTime = 0.0, computeTime = 0.0;
	
	while (*do_run) //a terminating signal leaves every message in the queue and quits the loop
	{
		//manage input data
		for (auto& q : inputQueues)
		{
			// this actual deleteMsg function won't be used here, because the dequeue will override it and the nullptr won't be deleted anyway
			ManagedMessage input;

			if (!q.second->DeQueue(input))
				goto halt;

			inputMessages[q.first] = input.get();
			receivedMessages.push_back(input);
		}
		
		PortNumber outputNumber;
		diffTime = timer.Tock();
		timer.Tick();
		outputMessages_raw = compute(_module, inputMessages.data(), inputMessages.size(), &outputNumber);
		computeTime = prevTime;
		prevTime = timer.Tock();
		handle_statistics(identifier.name, diffTime, computeTime);

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
				managedOutput.reset(messagePtr, deleteMsg);
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
			handle_output_text(identifier.name, output_text_raw);

		size_t picure_size = 0;
		if (output_image_raw != nullptr && (picure_size = output_image_width * output_image_height * GetByteDepth(imageType)) > 0)
		{
			output_image.Modify([&](ImageContainer& self)
			{
				self.data.assign(output_image_raw, output_image_raw + picure_size);
				self.h = output_image_height;
				self.w = output_image_width;
				self.type = imageType;
			});
		}

	}
halt:
	diffTime = timer.Tock();
	handle_statistics(identifier.name, diffTime, computeTime);

	if (*do_run) //in this case soft terminate
		for (auto& qs : outputQueues)
			for (auto& q : qs.second)
				q->WaitForEmpty();
	
	//TODO wake up only the synced queues
	for (auto& qs : outputQueues)
		for (auto& q : qs.second)
			q->WakeUp();

	//output_text.NonEditable();
	//std::cerr << (std::string)identifier << ": " << output_text.Get() << " (call interval: " << diffTime << ", compute time: " << computeTime << ")" <<std::endl;
	//output_text.MakeEditable();
}
