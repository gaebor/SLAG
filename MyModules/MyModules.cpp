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
DLL_EXPORT void* __stdcall SlagInstantiate(const char* name, const char* instance, const char** out_text, unsigned char** out_img, int* w, int* h, enum ImageType type)
{
	const std::string nameStr = name;
	MyModule* result = nullptr;
	if (nameStr == "Add")
	{
		result = new AddModule();
	}else if (nameStr == "VideoSource")
		result = new VideoSource();
	else if (nameStr == "ReadInt")
		result = new ReadModule<int>();
	else if (nameStr == "KeyReader")
		result = new KeyReader();
	else if (nameStr == "ImageProcessor")
		result = new ImageProcessor();

	if (result)
	{
		result->outputText = out_text;
		result->outputPicture = out_img;
		result->outputPictureWidth = w;
		result->outputPictureHeight = h;
		result->imageType = type;
	}
	return result;
}

DLL_EXPORT void __stdcall SlagDestroyMessage(void* message)
{
	delete (MyMessage*)(message);
}

DLL_EXPORT void __stdcall SlagDestroyModule(void* module)
{
	delete (MyModule*)(module);
}

DLL_EXPORT void** __stdcall SlagCompute(void* module, void** input, int inputPortNumber, int* outputPortNumber)
{
	return (void**)(static_cast<MyModule*>(module)->Compute((MyMessage**)input, inputPortNumber, outputPortNumber));
}

//! returns 0 on success
DLL_EXPORT int __stdcall SlagInitialize(void* module, int settingsc, const char* settingsv[])
{
	return (static_cast<MyModule*>(module)->Initialize(settingsc, settingsv)) ? 0 : -1 ;
}

#ifdef __cplusplus
}
#endif
