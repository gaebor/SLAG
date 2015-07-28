#pragma once
#include "AbstractInterface.h"

#include <string>

class IntMessage : public Message
{
public:
	IntMessage();
	~IntMessage(){}
	int val;
};

class AddModule : public Module
{
public:
	AddModule();
	virtual ~AddModule(void);

	Message** Compute(Message** input, int inputPortNumber, int* outputPortNumber);

private:
	Message* output_msg;
	std::string text;
};
