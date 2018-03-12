#include "AbstractInterface.h"

MyModule::MyModule()
{

}

MyModule::~MyModule()
{

}

bool MyModule::Initialize(int settingsc, const char** settingsv,
    void* txtin, void* txtout,
    const char** _strout, int* strout_size,
	unsigned char** out_img, int* w, int*h , enum ImageType type)
{
	strout = _strout;
    strout_length = strout_size;
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