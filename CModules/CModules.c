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

SLAG_MODULE_EXPORT(void*) SlagInstantiate(const char* moduleName, const char* InstanceName)
{
	SlagFunction_t function = NULL;

	if (0 == strcmp("Add", moduleName))
		function = Add;
	else if (0 == strcmp("Mul", moduleName))
		function = Mul;

	return (void*)function;
}

SLAG_MODULE_EXPORT(void**) SlagCompute(void* module, void** input, int inputPortNumber, int* outputPortNumber)
{
	return ((SlagFunction_t)module)(input, inputPortNumber, outputPortNumber);
}

SLAG_MODULE_EXPORT(void) SlagDestroyMessage(void* message)
{
	free(message);
}

SLAG_MODULE_EXPORT(void) SlagDestroyModule(void* module)
{
}
