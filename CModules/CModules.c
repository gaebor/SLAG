#include <stdlib.h>
#include <string.h>

#include "slag/slag_interface.h"

void** Add(void** input, int inputPortNumber, int* outputPortNumber)
{
	static void* result = NULL;

	double* sum = (double*)malloc(sizeof(double));
	*sum = 0.0;

	*outputPortNumber = 0;

	for(; inputPortNumber > 0; --inputPortNumber, ++input)
	{
		if (*input != NULL)
			(*sum) += *((int*)(*input));

		*outputPortNumber = 1;
	}
	result = sum;
	return &result;
}

void** Mul(void** input, int inputPortNumber, int* outputPortNumber)
{
	static void* result = NULL;

	double* sum = (double*)malloc(sizeof(double));
	*sum = 1.0;

	*outputPortNumber = 0;
	for(; inputPortNumber > 0; --inputPortNumber, ++input)
	{
		if (*input != NULL)
			(*sum) *= *((int*)(*input));
		*outputPortNumber = 1;
	}
	result = sum;
	return &result;
}

DLL_EXPORT void* __stdcall SlagInstantiate(const char* moduleName, const char* InstanceName, const char** out_text, unsigned char** out_img, int* w, int* h, enum ImageType* type)
{
	SlagFunction_t function = NULL;

	if (0 == strcmp("ADD", moduleName))
		function = Add;
	else if (0 == strcmp("MUL", moduleName))
		function = Mul;

	return (void*)function;
}

DLL_EXPORT void** __stdcall SlagCompute(void* module, void** input, int inputPortNumber, int* outputPortNumber)
{
	return ((SlagFunction_t)module)(input, inputPortNumber, outputPortNumber);
}

DLL_EXPORT void __stdcall SlagDestroyMessage(void* message)
{
	free(message);
}

DLL_EXPORT void __stdcall SlagDestroyModule(void* module)
{
}
