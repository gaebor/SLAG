#pragma once

#include <sstream>
#include <fstream>

#include "AbstractInterface.h"

template<class Type>
class ReadModule :	public MyModule
{
public:
	class Message : public ::MyMessage
	{
	public:
		Type value;
	};
	ReadModule(void){}
	virtual ~ReadModule(void){}

	::MyMessage** Compute(::MyMessage**, int, int* outputPortNumber)
	{
		if (getline(ifs, str).good())
		{
			auto output = new Message();
			std::istringstream iss(str);
			iss >> output->value;

            *strout = str.c_str();
            *strout_length = (int)str.size();
            //if (outputText)
			//	fprintf(outputText, line.c_str());

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

protected:
	bool InitializeCallback(int settingsc, const char** settingsv)
	{
		if (settingsc > 0)
			ifs.open(settingsv[0]);
		return true;
	}
private:
	std::ifstream ifs;
	::MyMessage* output_msg;
};
