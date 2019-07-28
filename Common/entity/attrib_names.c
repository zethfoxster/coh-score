/***************************************************************************
 *     Copyright (c) 2003-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#include <stddef.h>

#include "earray.h"
 
#include "character_attribs.h"
#include "attrib_names.h"

SHARED_MEMORY AttribNames g_AttribNames;
int g_offHealAttrib = -1;

/**********************************************************************func*
 * dbg_AttribName
 *
 */
char *dbg_AttribName(int offset, char *pchOrig)
{
	// This is a hack
	if(offset>=offsetof(CharacterAttributes, fDamageType)
		&& offset<offsetof(CharacterAttributes, fHitPoints))
	{
		// TODO; Add lookup
		int i = (offset-offsetof(CharacterAttributes, fDamageType))/sizeof(float);
		if(i>=0 && i<eaSize(&g_AttribNames.ppDamage))
		{
			return g_AttribNames.ppDamage[i]->pchName;
		}
	}
	if(offset>=offsetof(CharacterAttributes, fDefenseType)
		&& offset<offsetof(CharacterAttributes, fDefense))
	{
		// TODO; Add lookup
		int i = (offset-offsetof(CharacterAttributes, fDefenseType))/sizeof(float);
		if(i>=0 && i<eaSize(&g_AttribNames.ppDefense))
		{
			return g_AttribNames.ppDefense[i]->pchName;
		}
	}
	if(offset>=offsetof(CharacterAttributes, fElusivity)
		&& offset<offsetof(CharacterAttributes, fElusivityBase))
	{
		// TODO; Add lookup
		int i = (offset-offsetof(CharacterAttributes, fElusivity))/sizeof(float);
		if(i>=0 && i<eaSize(&g_AttribNames.ppElusivity))
		{
			return g_AttribNames.ppElusivity[i]->pchName;
		}
	}

	return pchOrig;
}

/**********************************************************************func*
 * AttribOffset
 *
 */
int attrib_Offset(char *pch)
{
	int i, n;
	
	n = eaSize(&g_AttribNames.ppDamage);
	for(i=0; i<n; i++)
	{
		if(stricmp(g_AttribNames.ppDamage[i]->pchName, pch)==0)
		{
			return offsetof(CharacterAttributes, fDamageType)+sizeof(float)*i;
		}
	}

	n = eaSize(&g_AttribNames.ppDefense);
	for(i=0; i<n; i++)
	{
		if(stricmp(g_AttribNames.ppDefense[i]->pchName, pch)==0)
		{
			return offsetof(CharacterAttributes, fDefenseType)+sizeof(float)*i;
		}
	}

	n = eaSize(&g_AttribNames.ppElusivity);
	for(i=0; i<n; i++)
	{
		if(stricmp(g_AttribNames.ppElusivity[i]->pchName, pch)==0)
		{
			return offsetof(CharacterAttributes, fElusivity)+sizeof(float)*i;
		}
	}

	return -1;
}
/* End of File */
