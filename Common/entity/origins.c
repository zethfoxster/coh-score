/***************************************************************************
 *     Copyright (c) 2003-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#include <assert.h>
#include <string.h>

#include "earray.h"
#include "classes.h"
#include "origins.h"

// The central buckets for all the origins used in the game.
SHARED_MEMORY CharacterOrigins g_CharacterOrigins;
SHARED_MEMORY CharacterOrigins g_VillainOrigins;

/**********************************************************************func*
 * origin_GetPtrFromName
 *
 */
const CharacterOrigin *origins_GetPtrFromName(const CharacterOrigins *porigins, const char *pch)
{
	int i;

	assert(porigins!=NULL);
	assert(pch!=NULL);

	for(i=eaSize(&porigins->ppOrigins)-1; i>=0; i--)
	{
		if(stricmp(porigins->ppOrigins[i]->pchName, pch)==0)
		{
			return porigins->ppOrigins[i];
		}
	}

	return NULL;
}

/**********************************************************************func*
 * origins_GetIndexFromName
 *
 */
int origins_GetIndexFromName(const CharacterOrigins *porigins, const char * pch)
{
	int i;

	assert(pch!=NULL);

	for(i=eaSize(&porigins->ppOrigins)-1; i>=0; i--)
	{
		if(stricmp(porigins->ppOrigins[i]->pchName, pch)==0)
		{
			return i;
		}
	}

	return -1;
}

/**********************************************************************func*
 * origins_GetIndexFromPtr
 *
 */
int origins_GetIndexFromPtr(const CharacterOrigins *porigins, const CharacterOrigin *porigin)
{
	int i;

	for(i=eaSize(&porigins->ppOrigins)-1; i>=0; i--)
	{
		if( porigins->ppOrigins[i] == porigin )
		{
			return i;
		}
	}

	return -1;
}

/**********************************************************************func*
 * origin_IsAllowedForClass
 *
 */
int origin_IsAllowedForClass(const CharacterOrigin *porigin, const CharacterClass *pclass)
{
	int i;
	for(i=eaSize(&pclass->pchAllowedOriginNames)-1; i >=0; i--)
	{
		// Horrible hack because we hate skills, but hate taking them out more
		if( stricmp(pclass->pchAllowedOriginNames[i], "SkillType00" ) == 0 &&
			stricmp("Science", porigin->pchName)==0 )
		{
			return true;
		}
		if(stricmp(pclass->pchAllowedOriginNames[i], porigin->pchName)==0)
		{
			return true;
		}
	}

	return false;
}
/* End of File */
