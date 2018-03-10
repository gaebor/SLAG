#include <stdlib.h>
#include <stdio.h>
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

FILE* input_for_read;
int* msg;

void** Read(void** input, int inputPortNumber, int* outputPortNumber)
{
    static int i;
    void** result = NULL;

    *outputPortNumber = 0;
    if (fscanf(input_for_read, "%d", &i) == 1)
    {
        msg = (int*)malloc(sizeof(int));
        if (msg)
        {
            *msg = i;
            *outputPortNumber = 1;
            result = &msg;
        }
    }
    return result;
}

SLAG_MODULE_EXPORT(int) SlagInitialize(
    void* module,
    int settingsc, const char** settingsv,
    void* txtin, void* txtout,
    const char** strout, int* strout_size,
    unsigned char** out_img, int* w, int* h, enum ImageType imageType)
{

    if (module == Read)
    {
        if (settingsc > 0)
            input_for_read = fopen(settingsv[0], "r");
    }
    return 0;
}


SLAG_MODULE_EXPORT(void*) SlagInstantiate(const char* moduleName, const char* InstanceName)
{
	SlagFunction_t function = NULL;

	if (0 == strcmp("Add", moduleName))
		function = Add;
	else if (0 == strcmp("Mul", moduleName))
		function = Mul;
    else if (0 == strcmp("Read", moduleName))
        function = Read;
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
    if (module == Read)
    {
        fclose(input_for_read);
    }
}
