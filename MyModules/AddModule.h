#pragma once
#include "AbstractInterface.h"

#include <string>

class IntMessage : public ::MyMessage
{
public:
	IntMessage();
	virtual ~IntMessage();
	int val;
};

class AddModule : public MyModule
{
public:
	AddModule();
	virtual ~AddModule(void);

	MyMessage** Compute(MyMessage** input, int inputPortNumber, int* outputPortNumber);

private:
	MyMessage* output_msg;
	std::string text;
};
