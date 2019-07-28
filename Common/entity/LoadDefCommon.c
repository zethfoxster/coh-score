/***************************************************************************
 *     Copyright (c) 2005-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#include "utils.h"
#include "assert.h"
#include "mathutil.h"
#include "earray.h"
#include "MemoryPool.h"
#include "StashTable.h"
#include "LoadDefCommon.h"
#include "textparser.h"
#include "attrib_names.h"
#include "SharedMemory.h"
#include "file.h"

const char *MakeSharedMemoryName(const char *pchBinFilename)
{
	static char achSharedMemoryName[1024];

	sprintf(achSharedMemoryName, "DEFS_%s", pchBinFilename);

#if SERVER
	// Server-side only version!  Don't use the same shared memory name
	// This has to be done for (at least) classes as well, because classes *point into* powers
	//  which is a different set of shared memory on the client and server
	strcat(achSharedMemoryName, "_SERVER");
#endif

	return achSharedMemoryName;
}


/**********************************************************************func*
* *MakeBinFilename
*
*/
const char *MakeBinFilename(const char *pchFilename)
{
	static char achBinFile[1024];
	char *pos = NULL;
	const char *cpos = NULL;

	if((cpos=strrchr(pchFilename, '/')) == NULL)
	{
		if((cpos=strrchr(pchFilename, '\\')) == NULL)
		{
			cpos = pchFilename-1;
		}
	}

	strcpy_s(SAFESTR(achBinFile), cpos+1);
	pos = strrchr(achBinFile, '.');
	if(pos!=NULL)
	{
		*pos='\0';
	}
	strcat(achBinFile, ".bin");

	return achBinFile;
}
