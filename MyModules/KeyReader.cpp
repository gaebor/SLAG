#include "KeyReader.h"
#include <sstream>
#include <conio.h>
#include <ctype.h>

KeyReader::KeyReader() :outouts(1,nullptr)
{
}

KeyReader::~KeyReader()
{
}

MyMessage** KeyReader::Compute(MyMessage** input, int inputPortNumber, int* outputPortNumber)
{
	if (input == nullptr)
	{
		return nullptr;
	}
	else
	{
		auto output = new IntMessage();
		output->val = _getch();
		outouts[0] = output;
		std::ostringstream oss;
		oss << output->val;
		text = oss.str();
		*outputText = text.c_str();
		return outouts.data();
	}
}
