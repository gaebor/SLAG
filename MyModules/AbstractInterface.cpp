#include "AbstractInterface.h"

MyModule::MyModule()
{

}

MyModule::~MyModule()
{

}

bool MyModule::Initialize(int settingsc, const char** settingsv,
    SlagTextOut* textout, SlagImageOut* imageout)
{
    strout = &(textout->str);
    strout_length = &(textout->size);
    outputPicture = &(imageout->data);
    outputPictureWidth = &(imageout->w);
    outputPictureHeight = &(imageout->h);
    imageType = imageout->type;

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