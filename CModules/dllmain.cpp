#include <stdlib.h>
#include <string.h>

#include "slag\slag_interface.h"

void** Add(void** input, int inputPortNumber, int* outputPortNumber)
{
	static void* result = NULL;

	double* sum = (double*)malloc(sizeof(double));
	*sum = 0.0;

	*outputPortNumber = 0;

	for(; inputPortNumber > 0; --inputPortNumber, ++input)
	{
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
		(*sum) *= *((int*)(*input));
		*outputPortNumber = 1;
	}
	result = sum;
	return &result;
}

DLL_EXPORT void* SlagInstantiate(const char* moduleName, const char* InstanceName, const char** out_text, unsigned char** out_img, int* w, int* h)
{
	SlagFunction_t function = NULL;

	if (0 == strcmp("ADD", moduleName))
		function = Add;
	else if (0 == strcmp("MUL", moduleName))
		function = Mul;

	return function;
}

DLL_EXPORT void** SlagCompute( void* module, void** input, int inputPortNumber, int* outputPortNumber)
{
	return ((SlagFunction_t)module)(input, inputPortNumber, outputPortNumber);
}

DLL_EXPORT void SlagDestroyMessage( void* message)
{
	free(message);
}

DLL_EXPORT void SlagDestroyModule( void* module)
{
}
