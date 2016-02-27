#pragma once
#include "AbstractInterface.h"
#include "AddModule.h"

#include <vector>
#include <string>

class KeyReader : public MyModule
{
public:
	KeyReader();
	virtual ~KeyReader();
	virtual MyMessage** Compute(MyMessage** input, int inputPortNumber, int* outputPortNumber);
private:
	std::vector<MyMessage*> outouts;
	std::string text;
};

