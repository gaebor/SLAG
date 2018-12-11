#include "AddModule.h"

#include <sstream>
#include "ReadModule.h"

AddModule::AddModule()
{
}


AddModule::~AddModule(void)
{
}

MyMessage** AddModule::Compute( MyMessage** input, int inputPortNumber, int* outputPortNumber )
{
	if (inputPortNumber == 0)
	{
		*outputPortNumber = 0;
		output_msg = nullptr;
		return &output_msg;
	}else
	{
		auto output = new IntMessage();
		for (int i = 0; i < inputPortNumber; ++i)
		{
			if (auto ptr = dynamic_cast<ReadModule<int>::Message*>(input[i]))
				output->val += ptr->value;
		}
		*outputPortNumber = 1;
		output_msg = output;
        str = std::to_string(output->val);
        *strout = str.c_str();
        *strout_length = (int)str.size();

		return &output_msg;
	}
}

IntMessage::IntMessage(int x)
	: val(x)
{
}

IntMessage::~IntMessage()
{
}
