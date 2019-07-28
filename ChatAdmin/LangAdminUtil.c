#include "LangAdminUtil.h"
#include "language/MessageStore.h"
#include "language/AppLocale.h"
#include "mathutil.h"
#include "ChatAdmin.h"
#include "earray.h"
#include "EString.h"

MessageStore* menuMessages;

char* vaTranslate( char *str, va_list args )
{
	static char  *buffer;
	static int buffersize = 0;
	int requiredBufferSize;

	if (!str)
		return NULL;

	requiredBufferSize = msvaPrintf(menuMessages, NULL, 0, str, args);
	requiredBufferSize = MAX(requiredBufferSize, (int)strlen(str));
	if( buffersize < requiredBufferSize+1 )
	{
		buffersize = requiredBufferSize+1;
		buffer = realloc( buffer, buffersize );
	}

	strcpy(buffer, str);
	msvaPrintf(menuMessages, buffer, buffersize, str, args);

	return buffer;
}


void conTransf(int type, char *str, ...)
{
	va_list va;

	va_start(va, str);
	conPrintf(type, "%s", vaTranslate(str, va));
	va_end(va);
}

char* localizedPrintf( char *str, ... )
{
	va_list va;
	char  *buffer;

	va_start(va, str);
	buffer = vaTranslate( str, va );
	va_end(va);

	return buffer;
}


void reloadAdminMessageStores(int localeID){
	char** ppchMessageFiles = NULL;
	char** ppchMessageDirs = NULL;
	char *loc_name = locGetName(localeID);

	// generate the list of files/folders
#define estr_print(dst,src) estrPrintf(&dst,"texts\\%s\\%s",loc_name,src)
#define push_estr(ea,str) { char *temp_str = NULL; estr_print(temp_str,str); eaPush(&ea,temp_str); }

	push_estr(ppchMessageFiles, "menuMessages.ms");		eaPush(&ppchMessageFiles, estrCloneCharString("texts\\menuMessages.types"));
	push_estr(ppchMessageFiles, "tools\\ChatAdmin.ms");	eaPush(&ppchMessageFiles, estrCloneCharString("texts\\ChatAdmin.types"));
	eaPush(&ppchMessageFiles, NULL);

//	push_estr(ppchMessageDirs,"tools");
	eaPush(&ppchMessageDirs,NULL);


	// do the actual loading
	LoadMessageStore(&menuMessages,ppchMessageFiles,ppchMessageDirs,localeID,NULL,NULL,NULL,MSLOAD_DEFAULT);


	// cleanup
	eaDestroyEx(&ppchMessageFiles,estrDestroyUnsafe);
	eaDestroyEx(&ppchMessageDirs,estrDestroyUnsafe);
}


