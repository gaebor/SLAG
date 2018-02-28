#ifndef INCLUDE_SLAG_INTERFACE_H
#define INCLUDE_SLAG_INTERFACE_H

#ifdef __GNUC__
#   define SLAG_VISIBILITY __attribute__((visibility("default")))
#	define SLAG_CALL 
#elif defined _MSC_VER
#   define SLAG_VISIBILITY __declspec(dllexport)
#	define SLAG_CALL __stdcall
#endif

#define SLAG_MODULE_EXPORT(X) SLAG_VISIBILITY X SLAG_CALL

#ifdef __cplusplus
extern "C" {
#endif

enum ImageType
{
	GREY,
	RGB,
	BGR,
	RGBA,
	BGRA
};

//! SLAG calls this once, your module or whatever necessary data should be allocated
/*!
	@param module pointer to the instantiated module to be initialized
	@param moduleName a C string, the name of your module, usually it identifies the job what your module is ought to do.
		Example: VideoSource, MyMagicFunction, OCRmodule
	@param InstanceName a C string, identifies your module, if more than one module of the same type is used.
		Example: VideoSource:leftCamera, MyMagicFunction:01, OCRmodule:ItalicRecongitionOnly
	@param out_text pointer to a C string, save this pointer for later, this pointer is used to extract textual data from your module.
		Allocate a C string, and write useful info into it. Then set this pointer to the beginning of your allocated string.
		Voila, your text will be readable for the SLAG. Set to NULL if your module doesn't have anything to say!
		Note that the actual C string should be allocated by YOU and should be freed by YOU.
		Moreover the pointed piece of memory should be valid outside of the SlagCompute function!
		Don't free it during the whole lifetime of your module (or set to NULL)
	@param out_img pointer to an array of pixels (image information)
		same memory management considerations apply like out_text.
		The pointer array is a 3*width*height byte array, containing RGB pixels.
		Set to NULL if you don't have any image to show!
	@param w output variable, save this pointer for later, write the width of your image here.
		Set to 0 if you don't have any image to show!
	@param h output variable, write the height of your image here.
		Set to 0 if you don't have any image to show!
	@return pointer to whatever you call a module
*/
typedef void* (SLAG_CALL *SlagInstantiate_t)(
	const char* moduleName,
	const char* InstanceName);

//! SLAG calls this if a message is useless anymore
/*!
	Note that you should allocate your messages on the heap, and let the SLAG to free them.
	Actually YOU will free them, but only when the SLAG tells you to.
	Every message what you return by the SlagCompute function is taken care of.
	Your destructor will be called if SLAG doesn't need your message anymore
	@param message pointer to a message, which was allocated by some of your modules.
		You should know how to delete it!
*/
typedef void(SLAG_CALL *SlagDestroyMessage_t)(void* message);

//! SLAG calls this after the graph has been finished
/*!
	Same memory management issues apply like SlagDestroyMessage
	@param module pointer to a module. You can delete it however you like!
*/
typedef void(SLAG_CALL *SlagDestroyModule_t)(void* module);

//! the core of SLAG
/*!

*/
typedef void** (SLAG_CALL *SlagCompute_t)(void* module, void** input, int inputPortNumber, int* outputPortNumber);

//! auxiliary
typedef void** (*SlagFunction_t)(void** input, int inputPortNumber, int* outputPortNumber);

//optional

//! SLAG calls this once, after instantiation and before any Compute calls
/*!
	@param module pointer to the instantiated module to be initialized
	@param settingsc number of received settings
	@param settingsv pointer to array of settings, each entry is a null terminated C string.
		This array is valid only until the initializer has been completed, after that it is destructed
		If you want to keep any of these settings later, you have to copy it to your heap.
	@param out_text pointer where you can set your output text
	@param l write the length of your output text here
	@param 	out_img write your output image here
	@param w write the width of your output image here
	@param h write the height of your output image here
	@param imageType you have to convert your output image to this format
	@return 0 on success, otherwise initialization is considered to be failed and the graph doesn't even start
*/
typedef  int (SLAG_CALL *SlagInitialize_t)(
	void* module, int settingsc, const char** settingsv,
	void* outputtext,
	unsigned char** out_img,
	int* w, int* h, enum ImageType imageType);

// #define SETTINGS_MAX_LENGTH 1024

#ifdef __cplusplus
}
#endif

#endif
