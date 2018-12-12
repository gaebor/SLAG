#pragma once
#include "slag_interface.h"

#include<string>

class MyMessage
{
public:
	MyMessage(void);
	virtual ~MyMessage(void);
};

//! node of the graph
class MyModule
{
public:
	MyModule();
	virtual ~MyModule(void);

	//! this is the essence of your module
	/*!
		@param input array of input messages
		@param inputPortNumber length of the input array
		@param outputPortNumber length of the returned array
		@return pointer to output array of messages
	*/
	virtual MyMessage** Compute( MyMessage** input, int inputPortNumber, int* outputPortNumber) = 0;
	//! receives the settings from the settings file
	/*!
		@return weather the initialization was successful.
		@param settingsc number of settings
		@param settingsc array of c-strings containing the received settings
	*/
	bool Initialize(int settingsc, const char** settingsv,
        SlagTextOut* textout, SlagImageOut* imageout);

protected:
	virtual bool InitializeCallback(int settingsc, const char** settingsv);
public:
	const char** strout;
    std::string str;
    int* strout_length;
	//! pointer to image output
	/*!
		You should allocate and free the memory
	*/
	unsigned char** outputPicture;
	int* outputPictureWidth;
	int* outputPictureHeight;
	SlagImageType imageType;
};

