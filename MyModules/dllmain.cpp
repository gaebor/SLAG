#include <string>

#include "slag/slag_interface.h"

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

__declspec(dllexport) slag::Module* InstantiateModule(const char* name, const char* instance, const char** out_text, unsigned char** out_img, int* w, int* h)
{
	std::string nameStr = name;
	slag::Module* result = nullptr;
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

__declspec(dllexport) void DestroyMessage( void* message)
{
	delete (slag::Message*)message;
}

__declspec(dllexport) void DestroyModule( void* module)
{
	delete (slag::Module*)module;
}

__declspec(dllexport) void** Compute( void* module, void** input, int inputPortNumber, int* outputPortNumber)
{
	return (void**)((slag::Module*)module)->Compute((slag::Message**)input, inputPortNumber, outputPortNumber);
}

#ifdef __cplusplus
}
#endif