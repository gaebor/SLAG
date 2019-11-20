#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "slag_interface.h"

static void** Add(void** input, int inputPortNumber, int* outputPortNumber)
{
	static void* result = NULL;

	double* sum = (double*)malloc(sizeof(double));
	*sum = 0.0;

	*outputPortNumber = 1;

	for(; inputPortNumber > 0; --inputPortNumber, ++input)
	{
		if (*input != NULL)
			(*sum) += *((int*)(*input));
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

        result = malloc(sizeof(void*)*(inputPortNumber));
        if (result)
        {
            outportNumber = inputPortNumber;
            *outputPortNumber = outportNumber;
        }
    }

    if (result)   
    {
        for (i = 0; i < inputPortNumber; ++i)
            result[i] = input[i];
    }

    return result;
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

static size_t quit_limit = 0;
static void** Quitter(void** input, int inputPortNumber, int* outputPortNumber)
{
    static void* result = NULL;
    static size_t i = 0;

    *outputPortNumber = 1;
    if (inputPortNumber > 0)
        result = input[0];
    else
        result = NULL;

    ++i;

    if (quit_limit > 0 && i > quit_limit)
    {
        *outputPortNumber = 0;
        return NULL; // quit
    }
    return &result;
}

static size_t seq_limit = 0;
static void** Seq(void** input, int inputPortNumber, int* outputPortNumber)
{
    static size_t i = 1;
    static int* result;

    if (seq_limit > 0 && i > seq_limit)
    {
        *outputPortNumber = 0;
        return NULL; // quit
    }

    result = (size_t*)malloc(sizeof(i));

    *outputPortNumber = 1;
    *result = i++;

    return &result;
}

SLAG_MODULE_EXPORT(int) SlagInitialize(
    void* module,
    int settingsc, const char** settingsv,
    SlagTextOut* textout, SlagImageOut* imageout)
{
    if (module == Read)
    {
        if (settingsc > 0)
            input_for_read = fopen(settingsv[0], "r");
    }
    if (module == Quitter)
    {
        if (settingsc > 0)
            quit_limit = (size_t)atoll(settingsv[0]);
    }
    
    textout->str = NULL; textout->size = 0;
    // resizes image window, even if empty
    imageout->data = NULL; imageout->w = 200; imageout->h = 0;

    return 0;
}


SLAG_MODULE_EXPORT(void*) SlagInstantiate(const char* moduleName, const char* InstanceName)
{
	SlagFunction_t function = NULL;

	if (0 == strcmp("Add", moduleName))
		function = Add;
    else if (0 == strcmp("Read", moduleName))
        function = Read;
    else if (0 == strcmp("Double", moduleName))
        function = Double;
    else if (0 == strcmp("Quitter", moduleName))
        function = Quitter;
    else if (0 == strcmp("Seq", moduleName))
        function = Seq;
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
