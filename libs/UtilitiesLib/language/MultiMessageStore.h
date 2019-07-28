/* File MultiMessageStore.h
 *	A MultiMessageStore is a message store that is capable of printing messages in multiple locales
 *	at the same time.
 *
 *	FIXME!!!
 *	Currently, this is just a quick hack to implement the functionality required to meet the 12/20/2002
 *	milestone.  The following things can be improved upon:
 *		1) Storing duplicate data
 *			Since each message store stores its own messageID->message hash map, it means that the
 *			messageIDs are being loaded multiple times.  In addition, each message type file is also
 *			stored seperately.  This will become problematic as the amount of text associated with 
 *			the game grows.
 *			
 *			To improve this, store messageIDs in a single StringTable in a mms and make all message
 *			store hash maps store a shallow copy only.
 *
 *		2) Redundant messageID lookups
 *			Since message files for each locale is stored in a seperate message store, if a message
 *			is not found in the store of a specified locale, the mms ought to revert to the default
 *			locale text.  This means that the mms needs to perform two looksups for each string
 *			that is not in a locale specific message store file.
 *
 *			To improve this, merge the different message store hash maps into one.  A single hash map
 *			would contain the messageID->array of locale specific messages.  In this way,
 *			all each message lookup will result in exactly one hash lookup.  In addition, problem
 *			#1 would also be solved since there is ever only one hash map.
 *	
 *	
 *	Requires:
 *		GameLocale
 */

#ifndef MULTI_MESSAGE_STORE_H
#define MULTI_MESSAGE_STORE_H

#include "MessageStore.h"

typedef struct Array			Array;
typedef struct ScriptVarsTable	ScriptVarsTable;

typedef void (*MMSPostProcessor)(MessageStore*);

typedef struct MultiMessageStore {
	MessageStore** localizedMessageStore;	// One message store for each locale.
	char** messageFilenames;				// alternating data and type filenames, null-terminated
	char const *idPrepend;					// pass to all message stores in this
	char *persistName;
	char *sharedName;
	MMSPostProcessor process;
	MessageStoreLoadFlags flags;
} MultiMessageStore;

// LoadMultiMessageStore handles all the binning, sharing, loading, and destroying
//   files is a null-terminated list of alternating message and type files
//   dirs is a null-terminated list of directories, whose contents will be added
//   files and dirs should not contain locale folder names, they will be inserted when each locale is loaded
void LoadMultiMessageStore(MultiMessageStore **pmms, char *bin_name, char *sm_name, int locale,
							char *idPrepend, char **files, char **folders, MMSPostProcessor process,
							MessageStoreLoadFlags flags);
void mmsLoadLocale(MultiMessageStore *mms, int locale); // in case you want to make sure a locale is loaded

char* mmsGetRealMessageFilename(char* virtualFilename, int localID);

const char* msxGetUnformattedMessageConst(MultiMessageStore* store, int localeID, const char *messageID);
char* msxGetUnformattedMessage(MultiMessageStore* store, char* outputBuffer, int bufferLength, int localeID, const char* messageID);

int msxPrintf(MultiMessageStore* store, char* outputBuffer, int bufferLength, int localeID, const char* messageID,
			   Array* messageTypeDef, ScriptVarsTable* vars, int flags, ...);

int msxPrintfVA(MultiMessageStore* store, char* outputBuffer, int bufferLength, int localeID, const char* messageID,
				Array* messageTypeDef, ScriptVarsTable* vars, int flags, va_list arg);


#endif
