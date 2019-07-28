#ifndef INCLUDE_FILEWATCHER
#define INCLUDE_FILEWATCHER

#include "stdtypes.h"
#include "wininclude.h"
#include <sys/stat.h>	// have to include sys/stat.h on vs2005 to avoid _stat being defined in the .c but not here

S32			fwFindFirstFile(U32* handleOut, const char* fileSpec, WIN32_FIND_DATA* wfd);
S32			fwFindNextFile(U32 handle, WIN32_FIND_DATA* wfd);
S32			fwFindClose(U32 handle);

S32			fwStat(const char* fileName, struct _stat32* statInfo);

#endif
