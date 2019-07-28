/***************************************************************************
 *     Copyright (c) 2003-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#include <string.h>
#include <ctype.h>

#include "stdtypes.h"
#include "earray.h"
#include "file.h"
#include "fileutil.h"
#include "StashTable.h"
#include "AppLocale.h"
#include "reserved_names.h"

#ifndef CHATSERVER
#include "storyarcutil.h"
#include "storyarcprivate.h"
#include "pnpc.h"
#include "contact.h"
#include "pnpcCommon.h"
#endif
#include "utils.h"

static StashTable s_hashReservedNames;

int GetReservedNameCount()
{
	if(s_hashReservedNames)
		return stashGetValidElementCount(s_hashReservedNames);
	else
		return 0;
}

/**********************************************************************func*
 * IsReservedName
 *
 */
int IsReservedName(const char *pchName, const char *pchAuth)
{
	static char *s_apchReservedPrefixes[] =
	{
		"Admin",
		"Cryptic",
		"CS",
		"CSR",
		"Game Master",
		"Gamemaster",
		"GM",
		"Moderator",
		"Monitor",
		"NCI",
		"NCSoft",
		"NC",
		"COH",
		"QA",
		"Test Master",
		"Tester",
		"Testmaster",
		"TM",
	};

	int i;
	char *pch;

	// disallow names with the prefixes above.
	for(i=0; i<ARRAY_SIZE(s_apchReservedPrefixes); i++)
	{
		if(strnicmp(s_apchReservedPrefixes[i], pchName, strlen(s_apchReservedPrefixes[i]))==0)
		{
			return true;
		}
	}

	// disallow names in the reserved file as well as pnpc and contact names.
	if(s_hashReservedNames && stashFindPointer(s_hashReservedNames, pchName, &pch))
	{
		if(pch && pchAuth && stricmp(pch, pchAuth)==0)
		{
			return false;
		}
		return true;
	}

	return false;
}


// chatserver gets reserved names from mapservers
void receiveReservedNames(Packet * pak)
{		
	if(s_hashReservedNames)
		stashTableDestroy(s_hashReservedNames);

	s_hashReservedNames = receiveHashTable(pak, StashDeepCopyKeys);

	printf("Received %d reserved names\n", stashGetValidElementCount(s_hashReservedNames));
}


void sendReservedNames(Packet * pak)
{
	sendHashTable(pak, s_hashReservedNames);
}

#ifndef CHATSERVER

/**********************************************************************func*
 * LoadNamesFile
 *
 */
static FileScanAction LoadNamesFile(char *dir, struct _finddata32_t *data)
{
	int len;
	char *mem;
	char *s;
	char *walk;
	char filename[MAX_PATH];
	FileScanAction retval = FSA_EXPLORE_DIRECTORY;

	// Ignore all directories.
	if(data->attrib & _A_SUBDIR)
		return retval;

	// Ignore all .bak files.
	if(strEndsWith(data->name, ".bak"))
		return retval;

	// Grab the full filename.
	sprintf(filename, "%s/%s", dir, data->name);

	walk = mem = fileAlloc(filename, &len);

	if (walk) {
		while (*walk=='\r' || *walk=='\n') walk++;
	}

	while (walk!=NULL && walk < mem + len)
	{
		s = walk;
		walk = strchr(s, '\r');

		if (!walk) break;

		while (*walk=='\r' || *walk=='\n') walk++;

		s = strtok(s, "\t\r\n");
		if(s && strncmp(s, "//", 2)!=0)
		{
			char *name = strchr(s, ':');
			if(name)
			{
				char *pos;

				*name=' ';
				while(name>=s && (*name==' ' || *name=='\t'))
					name--;
				name++;
				*name='\0';
				name++;
				while(*name==' ' || *name=='\t')
					name++;

				pos=name;
				while(*pos && *pos!=' ' && *pos!='\t' && *pos!='\n' && *pos!='\r')
					pos++;
				*pos = '\0';
			}
			if(*s)
				stashAddPointer(s_hashReservedNames, s, name, false);
		}
	}
	fileFree(mem);

	return retval;
}

void addReservedName(char * pch)
{
	if(!s_hashReservedNames)
	{
		s_hashReservedNames = stashTableCreateWithStringKeys(50, StashDeepCopyKeys);
	}
	stashAddPointer(s_hashReservedNames, pch, NULL, false);
}

/**********************************************************************func*
 * LoadReservedNames
 *
 */
void LoadReservedNames(void)
{
	int len;
	int i;
	const char *s;

	if(!s_hashReservedNames)
	{
		s_hashReservedNames = stashTableCreateWithStringKeys(50, StashDeepCopyKeys);
	}

	// Now add in all the contact and pnpc names
	len = eaSize(&g_contactlist.contactdefs);
	for (i = 0; i < len; i++)
	{
		s = ContactDisplayName(ContactDefinition(StoryIndexToHandle(i)), 0);
		if(s)
		{
			stashAddPointer(s_hashReservedNames, s, NULL, false);
		}
	}

	len = eaSize(&g_pnpcdeflist.pnpcdefs);
	for (i = 0; i < len; i++)
	{
		if(g_pnpcdeflist.pnpcdefs[i]->displayname)
		{
			s = saUtilScriptLocalize(s);
			stashAddPointer(s_hashReservedNames, s, NULL, false);
		}
	}
}

#endif

/* End of File */
