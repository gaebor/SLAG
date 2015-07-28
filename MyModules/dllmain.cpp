#include <string>

#include "slag/slag_interface.h"

#include "AbstractInterface.h"

#include "AddModule.h"
#include "VideoSource.h"
#include "ReadModule.h"

//BOOL APIENTRY DllMain( HMODULE hModule,
//                       DWORD  ul_reason_for_call,
//                       LPVOID lpReserved
//					 )
//{
//	switch (ul_reason_for_call)
//	{
//	case DLL_PROCESS_ATTACH:
//	case DLL_THREAD_ATTACH:
//	case DLL_THREAD_DETACH:
//	case DLL_PROCESS_DETACH:
//		break;
//	}
//	return TRUE;
//}

#ifdef __cplusplus
extern "C" {
#endif

//! this function instantiates your modules
DLL_EXPORT void* SlagInstantiate(const char* name, const char* instance, const char** out_text, unsigned char** out_img, int* w, int* h)
{
	std::string nameStr = name;
	Module* result = nullptr;
	if (nameStr == "AddModule")
	{
		result = new AddModule();
	}else if (nameStr == "VideoSource")
		result = new VideoSource();
	else if (nameStr == "ReadInt")
		result = new ReadModule<int>();

	if (result)
	{
		result->outputText = out_text;
		result->outputPicture = out_img;
		result->outputPictureWidth = w;
		result->outputPictureHeight = h;
	}
	return result;
}

DLL_EXPORT void SlagDestroyMessage( void* message)
{
	delete (Message*)message;
}

DLL_EXPORT void SlagDestroyModule( void* module)
{
	delete (Module*)module;
}

DLL_EXPORT void** SlagCompute( void* module, void** input, int inputPortNumber, int* outputPortNumber)
{
	return (void**)((Module*)module)->Compute((Message**)input, inputPortNumber, outputPortNumber);
}

//! returns 0 on success
DLL_EXPORT int SlagInitialize(void* module, int settingsc, const char* settingsv[])
{
	return (((::Module*)module)->Initialize(settingsc, settingsv)) ? 0 : -1 ;
}

#ifdef __cplusplus
}
#endif