#include "AddModule.h"

#include <sstream>

AddModule::AddModule()
{
}


AddModule::~AddModule(void)
{
}

slag::Message** AddModule::Compute( slag::Message* const * input, int inputPortNumber, int* outputPortNumber )
{
	if (inputPortNumber == 0)
	{
		*outputPortNumber = 0;
		output_msg = nullptr;
		return &output_msg;
	}else
	{
		output.val = 0;
		for (int i = 0; i < inputPortNumber; ++i)
		{
			if (auto ptr = dynamic_cast<IntMessage*>(input[i]))
				output.val += ptr->val;
		}
		*outputPortNumber = 1;
		output_msg = &output;
		std::ostringstream oss;
		oss << output.val;
		text = oss.str();
		outputText = text.c_str();
		return &output_msg;
	}
}

IntMessage::IntMessage() :val(0)
{

}
