#ifndef INCLUDE_SLAG_INTERFACE_H
#define INCLUDE_SLAG_INTERFACE_H

namespace slag{

class Message
{
public:
	Message(void);
	virtual ~Message(void);
};

class MessageSerailizable : public Message
{
public:
	MessageSerailizable();
	virtual ~MessageSerailizable();
	//! writes itself into buffer, sets the size of the buffer
	/*!
		if you don't implement it, then it returns negative number.
	*/
	virtual void* Serialize(int& bufferSize)const;
	//!constructs itself from buffer, returns the number of bytes read
	/*!
		if you don't implement it, then it returns negative number.
	*/
	virtual int DeSerialize(const void* buffer);
};

//! node of the graph
class Module
{
public:
	Module();
	virtual ~Module(void);

	//! this is the essence of your module
	/*!
		@param input array of input messages
		@param inputPortNumber length of the input array
		@param outputPortNumber length of the returned array
		@return pointer to output array of messages
	*/
	virtual Message** Compute( Message** input, int inputPortNumber, int* outputPortNumber) = 0;
	//! receives the settings from the settings file
	/*!
		@return weather the initialization was successful.
		@param settingsc number of settings
		@param settingsc array of c-strings containing the received settings
	*/
	virtual bool Initialize(int settingsc, const char* settingsv[]);

public:
	//! pointer to text output
	/*!
		pointer to a null-terminated string
		You should allocate and free the memory
	*/
	const char** outputText;
	//! pointer to image output
	/*!
		You should allocate and free the memory
	*/
	unsigned char** outputPicture;
	int* outputPictureWidth;
	int* outputPictureHeight;
};

typedef Module* (__cdecl *ModuleFactoryFunction)(const char* moduleName, const char* InstanceName, const char** out_text, unsigned char** out_img, int* w, int* h);

}

#endif