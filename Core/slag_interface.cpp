#include "slag/slag_interface.h"

namespace slag{

Picture::Picture() : imageInfo(nullptr), width(0), height(0)
{

}

Module::Module()
	: outputText(nullptr), outputPicture()
{

}

Module::~Module()
{

}

bool Module::Initialize( int settingsc, const char* settingsv[] )
{
	return true;
}

MessageSerailizable::MessageSerailizable(){}
MessageSerailizable::~MessageSerailizable(){}

void* MessageSerailizable::Serialize( int& bufferSize ) const
{
	bufferSize = -1;
	return nullptr;
}

int MessageSerailizable::DeSerialize( const void* buffer )
{
	return -1;
}

Message::~Message( void )
{

}

Message::Message( void )
{

}

}