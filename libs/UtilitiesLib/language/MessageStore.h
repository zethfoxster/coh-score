/* File MessageStore.h
 *	The message store printing facility seperates the actual content of the message from the call that produces the message.
 *	This makes it easy for a single executable to be able to print messages in a configured language.
 *
 *	Data files required:
 *	MessageFile:
 *		A message file contains MessageID + Message pairs stored in UTF-8.  
 *		A MessageID is a short alpha-numerical string that identifies a string uniquely.  
 *		A Message is a string to be printed on screen, which can contain named variables.
 *		A NamedVariable is enclosed in braces and can be any combination of ascii characters, except for { and }.
 *
 *		An example line in a message file might look like this:
 *			"WelcomeUserToDistrict": "Welcome to the {DistrictName}, {UserName}."
 *
 *
 *		Note that message files do not contain any information as to the type of the variable or the order in which the 
 *		variables are expected in code.  The file can be turned over to translators for modification safely, assuming 
 *		they do not mess up the MessageID or the variable names.
 *
 *	MessageTypeFile:
 *		A message type file contains MessageID and NameVariableDefinition pairs stored in UTF-8 or ascii.  Each entry is meant 
 *		as a suppliment to the MessageFile so that the print function will be able to determine where and how to place variables.  
 *		The order in which the variables definitions appear will implicitly define the order in which the print function will expect the variables.
 *
 *		A NamedVariableDefinition is a VariableName + VariableType pair.  There will be a corresponding type for all types that printf accepts.
 *
 *		An example line in a message type file might look like this:
 *			"HelloToUser": {UserName, string}, {DistrictName, string}
 *
 *		Message type files are meant to be kept under programmer control so that variable types and order can be changed if neccessary.
 *
 *
 *	Todo:
 *		- Clean up.
 *		  
 *		- It might be a good idea to get rid of message type files so that all such information can be sent to the msPrintf() function
 *		  directly.  This allows the programmer the freedom to pass data to the msPrintf() function using the format that is most
 *		  immediate.  Otherwise, if a message is ever printed in two locations where data is stored slightly differently, the programmer
 *		  will have to manually convert the data to the correct type first.
 *
 */

#ifndef MESSAGESTORE_H
#define	MESSAGESTORE_H

#include <stdio.h>
#include <stdarg.h>
#include "ArrayOld.h"
#include "MemoryPool.h"
#include "SharedHeap.h"

typedef struct MessageStore		MessageStore;
typedef struct ScriptVarsTable	ScriptVarsTable;
typedef struct StaticDefineInt	StaticDefineInt;

typedef struct MessageStoreFormatHandlerParams {
	const char*			fileName;
	int					lineCount;
	
	void*				userData;
	void*				param;
	
	const char*			srcstr;
	const char*			wholemsg;
	const char*			attribstr;
	char*				outputBuffer;
	ScriptVarsTable*	vars; 
	int					bufferLength;
	int					done;					// are we done processing after the format handler?
} MessageStoreFormatHandlerParams;

typedef int (*MessageStoreFormatHandler)(MessageStoreFormatHandlerParams* params);

typedef enum MessageStoreLoadFlags
{
	MSLOAD_DEFAULT		= 0,
	MSLOAD_EXTENDED		= (1<<0), // extended messages for texwords store
	MSLOAD_SERVERONLY	= (1<<1), // bins are in server/bin/ instead of bin/
	MSLOAD_FORCEBINS	= (1<<2), // load from original files only and write bin. suppresses some warnings.
	MSLOAD_NOFILES		= (1<<3), // don't load original files, only bin or shared memory
	MSLOAD_RELOADING	= (1<<4), // force reloading behavior, this is a hook for multimessage stores
} MessageStoreLoadFlags;

// this function manages all the cool stuff that a dozen or so old functions did
//   files is a null-terminated list of alternating message and type files ( e.g. {"menuMessages.ms","menuMessages.types",NULL} )
//   dirs is a null-terminated list of directories, whose contents will be added
void LoadMessageStore( MessageStore **pStore, char **files, char **dirs, int locid, const char *idPrepend,
					  char* persistfile, char *pcSharedMemoryName, MessageStoreLoadFlags flags );

void LoadMessageStoreFromMem(MessageStore **pStore, char *messages, char *types); // special handler for the updater

void msSetFormatHandler(MessageStore* store, char character, MessageStoreFormatHandler handler);
void msSetAttributeTables(	MessageStore* store,
							StaticDefineInt* attributes, 
							StaticDefineInt** attribToValue,
							S32* valueOffsetInUserData);

void msSetPrepend(MessageStore* store, const char* prepend);

void msClearExtendedDataFlags(MessageStore* store);

int msIsLocked(MessageStore* store);

void msSetHideTranslationErrors(int hideErrors);
int msGetHideTranslationErrors(void);

char* msGetUnformattedMessage(MessageStore* store, char* outputBuffer, int bufferLength, char* messageID);
	// get string from message store without any formatting
const char* msGetUnformattedMessageConst(MessageStore* store, const char* messageID);
	// get stored string from message store, or messageID if it cannot be found

extern int msPrintfError;	// True when the messageID is unknown.
int msPrintf(MessageStore* store, char* outputBuffer, int bufferLength, const char* messageID, ...);
int msvaPrintfEx(int flags, MessageStore* store, char* outputBuffer, int bufferLength, const char* messageID, va_list arg);
int msvaPrintf(MessageStore* store, char* outputBuffer, int bufferLength, const char* messageID, va_list arg);

int msPrintfVars(MessageStore* store, char* outputBuffer, int bufferLength, const char* messageID, ScriptVarsTable* vars, int recur_depth, int flags, ...);

int msvaPrintfInternal(	MessageStore* store,
						char* outputBuffer,
						int bufferLength,
						const char* messageID, 
						Array* messageTypeDef,
						ScriptVarsTable* vars,
						va_list arg);

#define	TRANSLATE				(0)
#define	TRANSLATE_HELP			(1<<0)
#define	TRANSLATE_REVERSE		(1<<1)
#define	TRANSLATE_NOPREPEND		(1<<2)
#define HANDLE_PARAMS_ANYWAY	(1<<3)

// If you want to add more prefixes, check loadMessageTypeData()!
#define TRANSLATE_PREPEND_P		(1<<4)
#define TRANSLATE_PREPEND_L_P	(1<<5)

int msvaPrintfInternalEx(	MessageStore* store,
							char* outputBuffer,
							int bufferLength,
							const char* messageID,
							Array* messageTypeDef,
							ScriptVarsTable* vars,
							int recur_depth,
							int flags,
							va_list arg);


const char *cmdTranslated(const char *str, ...);
const char *cmdRevTranslated(const char * str, ...);
const char *cmdHelpTranslated(const char *str, ...);

Array* msCompileMessageType(const char* typeDefString);

// Utility functions for modifying message stores at run time
int msContainsKey(MessageStore* store, const char *messageID);
void msUpdateMessage(MessageStore *store, const char *messageID, const char *currentLocaleText, const char *comment);
void msSaveMessageStore(MessageStore *store);

int verifyPrintable(char* message, const char* messageFilename, int lineCount);

void msCreateCmdMessageStore(const char* fileName, int localeID);

#endif
