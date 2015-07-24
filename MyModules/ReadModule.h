#pragma once

#include <iostream>
#include <fstream>

#include "slag/slag_interface.h"

template<class Type>
class ReadModule :	public slag::Module
{
public:
	class Message : public slag::Message
	{
	public:
		Type value;
	};
	ReadModule(void){}
	virtual ~ReadModule(void){}

	bool Initialize(int settingsc, const char* settingsv[])
	{
		if (settingsc > 1)
			ifs.open(settingsv[1]);
		return true;
	}
	slag::Message** Compute(slag::Message* const * input, int inputPortNumber, int* outputPortNumber)
	{
		if (ifs.good())
		{
			ifs >> output.value;
			output_msg = &output;
			*outputPortNumber = 1;
			return &output_msg;
		}else
		{
			*outputPortNumber = 0;
			output_msg = nullptr;
			return nullptr;
		}
	}

private:
	std::ifstream ifs;
	slag::Message* output_msg;
	Message output;
};
