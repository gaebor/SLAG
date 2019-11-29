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

typedef enum
{
	GREY,
	RGB,
	BGR,
	RGBA,
	BGRA
} SlagImageType;

typedef struct
{
    const char* str;
    int size;
} SlagTextOut;

typedef struct
{
    unsigned char* data;
    int w, h;
    const SlagImageType type;
} SlagImageOut;

/**
* \defgroup Mandatory you have to provide these function in your Library
* @{
*/

//! SLAG calls this once, your module or whatever necessary data should be allocated
/*!
	@param moduleName a C string, the name of your module, usually it identifies the job what your module is ought to do.
		Example: VideoSource, MyMagicFunction, OCRmodule
	@param InstanceName a C string, identifies your module, if more than one module of the same type is used.
		Example: VideoSource:leftCamera, MyMagicFunction:01, OCRmodule:ItalicRecongitionOnly
    @return pointer to whatever you call a module
*/
typedef void* (SLAG_CALL *SlagInstantiate_t)(
	const char* moduleName,
	const char* InstanceName);

//! SLAG calls this if a message is useless anymore
/*!
	Note that you should allocate your messages on the heap, and let the SLAG to free them.
	Actually YOU will free them, but only when SLAG tells you to.
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

/**@}*/

//! auxiliary
typedef void** (*SlagFunction_t)(void** input, int inputPortNumber, int* outputPortNumber);

/**
* \defgroup Optionals
* @{
*/

//! SLAG calls this once, after instantiation and before any Compute calls
/*!
	@param module pointer to the instantiated module to be initialized
	@param settingsc number of received settings
	@param settingsv pointer to array of settings, each entry is a null terminated C string.
		This array is valid only until the initializer has been completed, after that it is destructed
		If you want to keep any of these settings later, you have to copy it to your heap.
	@param textout pointer of a SlagTextOut object where you can store your output texts
	@param imageout pointer to a SlagImageOut object where you can store your output images
	@return 0 on success, otherwise initialization is considered to be failed
*/
typedef  int (SLAG_CALL *SlagInitialize_t)(
    void* module,
    int settingsc, const char** settingsv,
    SlagTextOut* textout, SlagImageOut* imageout);

//! you can send optional help messages about anything you want
/*!
    It is advised to return a how-to if the number of arguments is 0.
    A more detailed usage can be laid out then.
    Also, it is advised to return the possible modules to instantiate and their 
    SlagInitialize settings.

    @param argc number of argument strings
    @param argv array of null-terminated C-strings
    @return a string with some useful info in it.
        The returned string should be allocated and destroyed in your side.
        The returned string should be valid until the next SlagHelp call,
        which are guaranteed not to be concurrent to one and other.
*/
typedef const char* (SLAG_CALL *SlagHelp_t)(int argc, const char** argv);

/**@}*/

// #define SETTINGS_MAX_LENGTH 1024

#ifdef __cplusplus
}
#endif

#endif
