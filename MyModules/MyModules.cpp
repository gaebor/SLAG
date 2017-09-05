#include <string>

#include "slag/slag_interface.h"

#include "AbstractInterface.h"

#include "AddModule.h"
#include "VideoSource.h"
#include "ReadModule.h"
#include "KeyReader.h"
#include "ImageProcessor.h"

#ifdef __cplusplus
extern "C" {
#endif

//! this function instantiates your modules
MODULE_EXPORT(void*) SlagInstantiate(const char* name, const char*)
{
	const std::string nameStr = name;
	MyModule* result = nullptr;
	if (nameStr == "Add")
		result = new AddModule();
	
	else if (nameStr == "VideoSource")
		result = new VideoSource();
	
	else if (nameStr == "ReadInt")
		result = new ReadModule<int>();
	
	else if (nameStr == "KeyReader")
		result = new KeyReader();
	
	else if (nameStr == "ImageProcessor")
		result = new ImageProcessor();

	return (void*)result;
}

MODULE_EXPORT(void) SlagDestroyMessage(void* message)
{
	delete (MyMessage*)(message);
}

MODULE_EXPORT(void) SlagDestroyModule(void* module)
{
	delete (MyModule*)(module);
}

MODULE_EXPORT(void**) SlagCompute(void* module, void** input, int inputPortNumber, int* outputPortNumber)
{
	return (void**)(((MyModule*)module)->Compute((MyMessage**)input, inputPortNumber, outputPortNumber));
}

//! returns 0 on success
MODULE_EXPORT(int) SlagInitialize(
	void* module,
	int settingsc, const char** settingsv,
	const char** out_text, int* l,
	unsigned char** out_img,
	int* w, int* h, enum ImageType imageType)
{
	return (((MyModule*)module)->Initialize(settingsc, settingsv, out_text, l, out_img, w, h, imageType)) ? 0 : -1 ;
}

#ifdef __cplusplus
}
#endif
