#include <string>

#include "slag_interface.h"

#include "AbstractInterface.h"

#include "VideoSource.h"
#include "ImageProcessor.h"

#ifdef __cplusplus
extern "C" {
#endif

//! this function instantiates your modules
SLAG_MODULE_EXPORT(void*) SlagInstantiate(const char* name, const char*)
{
	const std::string nameStr = name;
	MyModule* result = nullptr;
    
	if (nameStr == "VideoSource")
		result = new VideoSource();
	
    else if (nameStr == "ImageProcessor")
		result = new ImageProcessor();

	return (void*)result;
}

SLAG_MODULE_EXPORT(void) SlagDestroyMessage(void* message)
{
	delete (MyMessage*)(message);
}

SLAG_MODULE_EXPORT(void) SlagDestroyModule(void* module)
{
	delete (MyModule*)(module);
}

SLAG_MODULE_EXPORT(void**) SlagCompute(void* module, void** input, int inputPortNumber, int* outputPortNumber)
{
	return (void**)(((MyModule*)module)->Compute((MyMessage**)input, inputPortNumber, outputPortNumber));
}

//! returns 0 on success
SLAG_MODULE_EXPORT(int) SlagInitialize(
	void* module,
	int settingsc, const char** settingsv,
    SlagTextOut* textout, SlagImageOut* imageout)
{
	return (((MyModule*)module)->Initialize(settingsc, settingsv, textout, imageout)) ? 0 : -1 ;
}

#ifdef __cplusplus
}
#endif
