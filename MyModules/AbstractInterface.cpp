#include "AbstractInterface.h"

MyModule::MyModule()
{

}

MyModule::~MyModule()
{

}

bool MyModule::Initialize(int settingsc, const char** settingsv,
	FILE* outtext,
	unsigned char** out_img, int* w, int*h , enum ImageType type)
{
	outputText = outtext;
	outputPicture = out_img;
	outputPictureWidth = w;
	outputPictureHeight = h;
	imageType = type;

	return InitializeCallback(settingsc, settingsv);
}

bool MyModule::InitializeCallback(int, const char**)
{
	return true;
}

MyMessage::~MyMessage( void )
{

}

MyMessage::MyMessage( void )
{

}