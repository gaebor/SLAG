#include <string>

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

extern "C" __declspec(dllexport) slag::Module* __cdecl InstantiateModule(const char* name, const char* instance)
{
	std::string nameStr = name;
	if (nameStr == "AddModule")
	{
		return new AddModule();
	}else if (nameStr == "VideoSource")
		return new VideoSource();
	else if (nameStr == "ReadInt")
		return new ReadModule<int>();
	return nullptr;
}
