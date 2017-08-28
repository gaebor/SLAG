#include "KeyReader.h"
#include <sstream>
#include <stdio.h>
#include <ctype.h>

KeyReader::KeyReader() :outouts(1,nullptr)
{
}

KeyReader::~KeyReader()
{
}

MyMessage** KeyReader::Compute(MyMessage** input, int, int* outputPortNumber)
{
	if (input == nullptr)
	{
		return nullptr;
	}
	else
	{
		auto output = new IntMessage();
		output->val = fgetc(stdin);
		outouts[0] = output;
		*outputPortNumber = 1;
		std::ostringstream oss;
		oss << output->val;
		text = oss.str();
		*outputText = text.c_str();
		return outouts.data();
	}
}
