#pragma once

#include <iostream>
#include <fstream>

#include "AbstractInterface.h"

template<class Type>
class ReadModule :	public Module
{
public:
	class Message : public ::Message
	{
	public:
		Type value;
	};
	ReadModule(void){}
	virtual ~ReadModule(void){}

	bool Initialize(int settingsc, const char* settingsv[])
	{
		if (settingsc > 0)
			ifs.open(settingsv[0]);
		return true;
	}
	::Message** Compute(::Message** input, int inputPortNumber, int* outputPortNumber)
	{
		if (getline(ifs, line).good())
		{
			auto output = new Message();
			std::istringstream iss(line);
			iss >> output->value;

			*outputText = line.c_str();

			output_msg = output;
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
	std::string line;
	std::ifstream ifs;
	::Message* output_msg;
};
