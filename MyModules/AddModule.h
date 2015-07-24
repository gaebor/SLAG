#pragma once
#include "slag/slag_interface.h"

#include <string>

class IntMessage : public slag::Message
{
public:
	IntMessage();
	~IntMessage(){}
	int val;
};

class __declspec(dllexport) AddModule : public slag::Module
{
public:
	AddModule();
	virtual ~AddModule(void);

	slag::Message** Compute(slag::Message* const * input, int inputPortNumber, int* outputPortNumber);

private:
	IntMessage output;
	slag::Message* output_msg;
	std::string text;
};
