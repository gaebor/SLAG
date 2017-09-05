#include "AbstractInterface.h"

MyModule::MyModule()
{

}

MyModule::~MyModule()
{

}

bool MyModule::Initialize(int settingsc, const char** settingsv,
	const char** out_text, int* l,
	unsigned char** out_img, int* w, int*h , enum ImageType type)
{
	outputText = out_text;
	outputTextLength = l;
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