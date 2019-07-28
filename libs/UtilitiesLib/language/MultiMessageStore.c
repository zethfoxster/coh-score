#include "MultiMessageStore.h"
#include "utils.h"
#include "AppLocale.h"
#include "earray.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "fileutil.h"
#include "strings_opt.h"
#include "error.h"

static void mmsAddMessages(MultiMessageStore* store, char* messageFilename, char* messageTypeFilename);
static void mmsAddMessageDirectory(MultiMessageStore* store, char* dirname);

char* mmsGetRealMessageFilename(char* virtualFilename, int localeID)
{
	static char realFilename[512];
	char* baseDirTerminator;

	if(!locIDIsValid(localeID))
		return NULL;

	realFilename[0] = '\0';

	// Where does the base directory end?
	baseDirTerminator = strstri(virtualFilename, "texts\\");

	// If we can't locate the "texts" directory anywhere in the path, that means
	// that the given file should be loaded directly, without any sort of locale mapping.
	if(!baseDirTerminator){
		baseDirTerminator = strstri(virtualFilename, "texts/");
		if(!baseDirTerminator){
			// This will remain an error condition for now because the server will always want to
			// load files with mapped paths.
			// It doesn't make much sense to use a MultiMessageStore if multiple message versions
			// are not required.  Use MessageStore in that case.
			assert(0);
		}
	}


	// Copy the base directory into the real filename.
	baseDirTerminator += strlen("texts\\");
	strncpy(realFilename, virtualFilename, baseDirTerminator - virtualFilename);
	realFilename[baseDirTerminator - virtualFilename] = '\0';

	// Copy the locale specific directory name into the real filename
	strcat(realFilename, locGetName(localeID));
	strcat(realFilename, "\\");

	// Copy the filename into the real filename now.
	strcat(realFilename, baseDirTerminator);
	return realFilename;
}

static MultiMessageStore* createMultiMessageStore(void)
{
	MultiMessageStore* store;
	
	callocStruct(store, MultiMessageStore);
	assert(store);
	callocStructs(store->localizedMessageStore, MessageStore*, locGetMaxLocaleCount());
	assert(store->localizedMessageStore);

	return store;
}

void destroyMessageStore(MessageStore *store); // special private access to MessageStore.c 
static void destroyMultiMessageStore(MultiMessageStore *store)
{
	int i;

	if (!store) return;
	if (!store->localizedMessageStore) { free(store); return; }
	for (i = 0; i < locGetMaxLocaleCount(); i++)
	{
		destroyMessageStore(store->localizedMessageStore[i]);
		store->localizedMessageStore[i] = NULL;
	}
	eaDestroyEx(&store->messageFilenames,NULL);
	SAFE_FREE(store->localizedMessageStore);
	SAFE_FREE(store);
}

static void ensureLocaleIsLoaded(MultiMessageStore* store, int localeID, bool error)
{
	char **files = NULL;

	// If the messages are loaded, then we're good
	if(store->localizedMessageStore[localeID])
		return;

	if(error)
		Errorf("MessageStore locales should already be loaded at this point");

	if(!store->persistName || isDevelopmentMode()) // in production, only bins are available
	{
		// recreate the list for the appropriate locale
		int i;
		for(i = 0; store->messageFilenames[i]; i += 2)
		{
			if(store->messageFilenames[i])
			{
				char *realMessageFilename = mmsGetRealMessageFilename(store->messageFilenames[i], localeID);
				assert(realMessageFilename); // Can't initialize the store if the file doesn't exist. (this shouldn't happen in production)
				eaPush(&files,strdup(realMessageFilename));
			}
			else
			{
				eaPush(&files,NULL);
			}
			eaPush(&files,strdup(store->messageFilenames[i+1])); // type file, doesn't need name adjustment
		}
		eaPush(&files,NULL);
	}

	// load up the store (where ever it happens to be coming from)
	LoadMessageStore(&store->localizedMessageStore[localeID],files,NULL,localeID,store->idPrepend,store->persistName,store->sharedName,store->flags);
	if(store->process)
		store->process(store->localizedMessageStore[localeID]);
	eaDestroyEx(&files,NULL); // free strings
}

void LoadMultiMessageStore(MultiMessageStore **pmms, char *bin_name, char *sm_name, int locale,
							char *idPrepend, char **files, char **folders, MMSPostProcessor process,
							MessageStoreLoadFlags flags)
{
	int i;
	MultiMessageStore *mms;
	int reloading;

	assert(pmms);

	reloading = *pmms != NULL;
	if(reloading)
		destroyMultiMessageStore(*pmms);
	*pmms = mms = createMultiMessageStore();
	mms->idPrepend = idPrepend;
	mms->persistName = bin_name;
	mms->sharedName = sm_name;
	mms->process = process;
	mms->flags = flags;
	if(reloading)
		mms->flags |= MSLOAD_RELOADING;

	if(!mms->persistName || isDevelopmentMode()) // in production, only bins are available
	{
		// build the file list
		for(i = 0; files && files[i]; i += 2)
			mmsAddMessages(mms,files[i],files[i+1]);
		for(i = 0; folders && folders[i]; ++i)
			mmsAddMessageDirectory(mms, folders[i]);
		eaPush(&mms->messageFilenames,NULL); // i'm the terminator
	}

	if(locale < 0)
	{
		// load them all
		ensureLocaleIsLoaded(mms,LOCALE_ID_ENGLISH,false);
	}
	else
	{
		ensureLocaleIsLoaded(mms,locale,false);
	}

	mms->flags &= ~MSLOAD_RELOADING;
}

void mmsLoadLocale(MultiMessageStore *mms, int locale)
{
	ensureLocaleIsLoaded(mms,locale,false);
}

const char* msxGetUnformattedMessageConst(MultiMessageStore* store, int localeID, const char *messageID)
{
	if(!store)
		return messageID;

	if(!locIDIsValid(localeID))
		localeID = DEFAULT_LOCALE_ID;

	ensureLocaleIsLoaded(store,localeID,true);

	// Try to print the message using the locale specific message store first.
	if(store->localizedMessageStore[localeID])
	{
		MessageStore* ms = store->localizedMessageStore[localeID];
		const char *result = msGetUnformattedMessageConst(ms, messageID);
		if(result != messageID)
			return result;
	}

	// If that doesn't work, then recursively print using the default locale.
	if(localeID != DEFAULT_LOCALE_ID)
		return msxGetUnformattedMessageConst(store, DEFAULT_LOCALE_ID, messageID);

	// If all else fails, just give them their key back
	return messageID;
}

char* msxGetUnformattedMessage(MultiMessageStore* store, char* outputBuffer, int bufferLength, int localeID, const char* messageID)
{
	const char *result = msxGetUnformattedMessageConst(store, localeID, messageID);
	strcpy_s(outputBuffer, bufferLength, result);
	return outputBuffer;
}

int msxPrintf(MultiMessageStore* store, char* outputBuffer, int bufferLength, int localeID, const char* messageID,
			  Array* messageTypeDef, ScriptVarsTable* vars, int flags, ...)
{
	int result;

	VA_START(arg, flags);
	result = msxPrintfVA(store, outputBuffer, bufferLength, localeID, messageID, messageTypeDef, vars, flags, arg);
	VA_END();

	return result;
}

int msxPrintfVA(MultiMessageStore* store, char* outputBuffer, int bufferLength, int localeID, const char* messageID,
						Array* messageTypeDef, ScriptVarsTable* vars, int flags, va_list arg)
{
	if(!store)
		return -1;

	if(!locIDIsValid(localeID))
		localeID = DEFAULT_LOCALE_ID;

	ensureLocaleIsLoaded(store,localeID,true);

	// Try to print the message using the locale specific message store first.
	if(store->localizedMessageStore[localeID])
	{
		int printfResult;
		MessageStore* ms = store->localizedMessageStore[localeID];

		printfResult = msvaPrintfInternalEx(ms, outputBuffer, bufferLength, messageID, messageTypeDef, vars, 0, flags, arg);
		if(!msPrintfError || localeID == DEFAULT_LOCALE_ID)
			return printfResult;
	}

	// If that doesn't work, then recursively print using the default locale.
	if(localeID != DEFAULT_LOCALE_ID)
		return msxPrintfVA(store,outputBuffer,bufferLength,DEFAULT_LOCALE_ID,messageID,messageTypeDef,vars,flags,arg);

	return -1;
}


static int stringArrayFind(char*** eArrayPtr, char* str)
{
	int i, n = eaSize(eArrayPtr);

	for(i = 0; i < n; i += 2)
	{
		assert((*eArrayPtr)[i]); // we shouldn't have a terminator while we're adding stuff
		if(0 == stricmp((*eArrayPtr)[i], str))
			return 1;
	}
	return 0;
}

/* Function mssAddMessages()
 *	Set up the file list for initializing the default message store.
 */
static void mmsAddMessages(MultiMessageStore* store, char* messageFilename, char* messageTypeFilename)
{
	// this all bugs me, but it's apparently not significantly slow
	if(messageFilename && !stringArrayFind(&store->messageFilenames, messageFilename))
	{
		eaPush(&store->messageFilenames, strdup(messageFilename));
		eaPush(&store->messageFilenames, strdup(messageTypeFilename));
	}
}

static MultiMessageStore* mmsAddMessageDirStore;
static FileScanAction mmsAddMessageDirHelper(char* dir, struct _finddata32_t* data)
{
	char filename[MAX_PATH];
	char* localeNameStart;
	char* defaultLocaleName;
	int defaultLocaleNameLen;
	MultiMessageStore* store = mmsAddMessageDirStore;
	FileScanAction retval = FSA_EXPLORE_DIRECTORY;

	// Ignore all directories.
	if(data->attrib & _A_SUBDIR)
		return retval;

	// Ignore all .bak files.
	if(strEndsWith(data->name, ".bak"))
		return retval;

	// Grab the full filename.
	sprintf_s(SAFESTR(filename), "%s/%s", dir, data->name);

	// Get the default locale name.
	defaultLocaleName = locGetName(DEFAULT_LOCALE_ID);
	defaultLocaleNameLen = (int)strlen(defaultLocaleName);

	// Convert all real filenames back into virtual filenames.
	// Find the default locale directory and get rid of it to derive the virtual filename.
	localeNameStart = strstri(filename, defaultLocaleName);
	if(localeNameStart)
		memmove(localeNameStart, localeNameStart + defaultLocaleNameLen + 1, strlen(localeNameStart + defaultLocaleNameLen));

	// Store virtual filenames.
	mmsAddMessages(store, filename, NULL);

	return retval;
}

static void mmsAddMessageDirectory(MultiMessageStore* store, char* dirname)
{
	char* realDirname;

	// Convert the virtual dirname into real dirname.
	realDirname = mmsGetRealMessageFilename(dirname, 0);

	mmsAddMessageDirStore = store;

	// Get a list of filenames in the directory.
	fileScanAllDataDirs(realDirname, mmsAddMessageDirHelper);
}
