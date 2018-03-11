#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "slag/slag_interface.h"

static void** Add(void** input, int inputPortNumber, int* outputPortNumber)
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

static void** Double(void** input, int inputPortNumber, int* outputPortNumber)
{
    static void** result = NULL;
    static int outportNumber = 0;
    int i;

    if (inputPortNumber > outportNumber)
    {
        if (result)
            free(result);

        result = malloc(sizeof(void*)*(inputPortNumber +1 ));
        if (result)
        {
            outportNumber = inputPortNumber + 1;
            *outputPortNumber = outportNumber;
        }
    }

    if (result)   
    {
        for (i = 0; i < inputPortNumber; ++i)
            result[i] = input[i];
        if (inputPortNumber > 0)
        {
            result[inputPortNumber] = malloc(sizeof(int));
            *(int*)(result[inputPortNumber]) = 0xffff;
        }

    }

    return result;
}

static void** Mul(void** input, int inputPortNumber, int* outputPortNumber)
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

static FILE* input_for_read = NULL;

static void** Read(void** input, int inputPortNumber, int* outputPortNumber)
{
    static void* result = NULL;
    int i;

    *outputPortNumber = 0;
    if (input_for_read && fscanf(input_for_read, "%d", &i) == 1)
    {
        result = malloc(sizeof(int));
        if (result)
        {
            *(int*)result = i;
            *outputPortNumber = 1;
            return &result;
        }
        else
            return NULL;
    }else
        return NULL;
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
    else if (0 == strcmp("Double", moduleName))
        function = Double;
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
    if (module == Read && input_for_read)
    {
        fclose(input_for_read);
    }
}
