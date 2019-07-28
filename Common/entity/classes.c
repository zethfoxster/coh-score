/***************************************************************************
 *     Copyright (c) 2000-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#include <assert.h>
#include <string.h>


#include "error.h"
#include "earray.h"
#include "textparser.h"
#include "StashTable.h"
#include "SharedMemory.h"

#include "powers.h"
#include "classes.h"

// The central buckets for all the classes used in the game.
SHARED_MEMORY CharacterClasses g_CharacterClasses;
SHARED_MEMORY CharacterClasses g_VillainClasses;

extern TokenizerParseInfo ParseCharacterAttributesTable[];

TokenizerParseInfo DestroyUnusedCharacterAttributeTable[] =
{
	{ "AttribMaxTable",         TOK_STRUCT(CharacterClass, ppTempAttribMax, ParseCharacterAttributesTable) },
	{ "AttribMaxMaxTable",      TOK_STRUCT(CharacterClass, ppTempAttribMaxMax, ParseCharacterAttributesTable) },
	{ "StrengthMaxTable",       TOK_STRUCT(CharacterClass, ppTempStrengthMax, ParseCharacterAttributesTable) },
	{ "ResistanceMaxTable",     TOK_STRUCT(CharacterClass, ppTempResistanceMax, ParseCharacterAttributesTable) },
	{ "", 0, 0 }
};

/**********************************************************************func*
 * classes_Repack
 *
 */
void classes_Repack(CharacterClasses *pclasses, const PowerDictionary *pdict)
{
	int i;

	assert(pclasses!=NULL);
	assert(pdict!=NULL);

	for(i=eaSize(&pclasses->ppClasses)-1; i>=0; i--)
	{
		int j;
		CharacterClass *pclass = (CharacterClass*)pclasses->ppClasses[i];

		assert(eaSize(&pclass->ppattrMin)==1);
		assert(eaSize(&pclass->ppattrBase)==1);
		assert(eaSize(&pclass->ppTempAttribMax)==1);
		assert(eaSize(&pclass->ppTempAttribMaxMax)==1);
		assert(eaSize(&pclass->ppTempStrengthMax)==1);
		assert(eaSize(&pclass->ppTempResistanceMax)==1);

		// StrengthMin was added in a patch and I wanted to provide for some
		// backward compatibility with old data. So, if no StrengthMins
		// were specified, make them up.
		if(pclass->ppattrStrengthMin==NULL)
		{
			eaCreate(&cpp_const_cast(CharacterAttributes**)(pclass->ppattrStrengthMin));
			eaPushConst(&pclass->ppattrStrengthMin, ParserAllocStruct(sizeof(CharacterAttributes)));
		}

		// ResistanceMin was added in a patch and I wanted to provide for some
		// backward compatibility with old data. So, if no ResistanceMins
		// were specified, make them up.
		if(pclass->ppattrResistanceMin==NULL)
		{
			float *pf = ParserAllocStruct(sizeof(CharacterAttributes));

			eaCreate(&cpp_const_cast(CharacterAttributes**)(pclass->ppattrResistanceMin));
			eaPushConst(&pclass->ppattrResistanceMin, pf);

			for(j=0; j<sizeof(CharacterAttributes)/sizeof(float); j++)
			{
				*pf++ = -100.0f;
			}
		}

		// Get the actual pointer to the PowerCategory from its string
		for(j=0; j<kCategory_Count; j++)
		{
			if(pclass->pchTempCategory[j])
				pclass->pcat[j] = powerdict_GetCategoryByName(pdict, pclass->pchTempCategory[j]);
		}
		if(!pclass->pcat[kCategory_Pool]) pclass->pcat[kCategory_Pool] = powerdict_GetCategoryByName(pdict, "Pool");
		if(!pclass->pcat[kCategory_Epic]) pclass->pcat[kCategory_Epic] = powerdict_GetCategoryByName(pdict, "Epic");

		for(j=0; j<kCategory_Count; j++)
		{
			assert(pclass->pcat[j]);
		}

		// Translate the great big attribute tables into different, easier
		//   to use, great big tables.

		pclass->iNumLevels = eafSize(&pclass->ppTempAttribMaxMax[0]->pfHitPoints);

		pclass->pattrMax = CreateCharacterAttributes(pclass->ppTempAttribMax, pclass->iNumLevels, 1.0);
		pclass->pattrMaxMax = CreateCharacterAttributes(pclass->ppTempAttribMaxMax, pclass->iNumLevels, 1.0);
		pclass->pattrStrengthMax = CreateCharacterAttributes(pclass->ppTempStrengthMax, pclass->iNumLevels, 1.0);
		pclass->pattrResistanceMax = CreateCharacterAttributes(pclass->ppTempResistanceMax, pclass->iNumLevels, 1.0);

		pclass->iNumBytesTables = pclass->iNumLevels*sizeof(CharacterAttributes);

		ParserDestroyStruct(DestroyUnusedCharacterAttributeTable, pclass);
	}
}

/**********************************************************************func*
 * classes_Finalize
 *
 */
bool classes_Finalize(CharacterClasses *pclasses, bool shared_memory)
{
	bool ret = true;
	int i;

	assert(pclasses!=NULL);

	for(i=0; i<eaSize(&pclasses->ppClasses); i++)
	{
		int j;
		CharacterClass *pclass = (CharacterClass*)pclasses->ppClasses[i];

		// Build a table for faster lookups
		assert(!pclass->namedStash);
		pclass->namedStash = stashTableCreateWithStringKeys(stashOptimalSize(eaSize(&pclass->ppNamedTables)), stashShared(shared_memory));
		for(j=0; j<eaSize(&pclass->ppNamedTables); j++)
		{
			if (!stashAddPointerConst(pclass->namedStash, pclass->ppNamedTables[j]->pchName, pclass->ppNamedTables[j], false))
			{
				assertmsg(0, "Duplicate entry in named table");
				ret = false;
			}
		}
	}

	return ret;
}

/**********************************************************************func*
 * classes_GetPtrFromName
 *
 */
const CharacterClass *classes_GetPtrFromName(const CharacterClasses *pclasses, const char *pch)
{
	const CharacterClass *pclass = NULL;
	int i;

	assert(pclasses!=NULL);

	if(pch!=NULL)
	{
		for(i=eaSize(&pclasses->ppClasses)-1; i>=0; i--)
		{

			if(stricmp(pclasses->ppClasses[i]->pchName, pch)==0)
			{
				pclass = pclasses->ppClasses[i];
				break;
			}
		}
	}

	if(pclass==NULL)
	{
		Errorf("Class name not found: <%s>\n", pch==NULL ? "(null)" : pch);
	}
	return pclass;
}

/**********************************************************************func*
* classes_GetIndexFromName
*
*/
int classes_GetIndexFromName(const CharacterClasses *pclasses, const char *pch)
{
	int i;

	if(pch!=NULL && pclasses!=NULL)
	{
		for(i=eaSize(&pclasses->ppClasses)-1; i>=0; i--)
		{
			if(stricmp(pclasses->ppClasses[i]->pchName, pch)==0)
			{
				return i;
			}
		}
	}

	return -1;
}

/**********************************************************************func*
* classes_GetIndexFromPtr
*
*/
int classes_GetIndexFromPtr(const CharacterClasses *pclasses, const CharacterClass *pclass)
{
	int i;

	if(pclass!=NULL && pclasses!=NULL)
	{
		for(i=eaSize(&pclasses->ppClasses)-1; i>=0; i--)
		{
			if(pclasses->ppClasses[i] == pclass)
			{
				return i;
			}
		}
	}

	return -1;
}

/**********************************************************************func*
 * class_GetNamedTableValue
 *
 */
float class_GetNamedTableValue(const CharacterClass *pclass, const char *pch, int iIdx)
{
	const NamedTable *ptab = NULL;
	int i;

	assert(pclass!=NULL);
	assert(pch);
	assert(iIdx>=0);

	if(stashFindPointerConst(pclass->namedStash, pch, &ptab))
	{
		assert(ptab);

		i = eafSize(&ptab->pfValues);
		if(iIdx>=i)
		{
			iIdx = i-1;
		}

		return ptab->pfValues[iIdx];
	}

	Errorf("Unable to find table %s\n", pch);
	return 1.0f;
}

/**********************************************************************func*
* class_MatchSpecialRestriction
*
*/

bool class_MatchesSpecialRestriction(const CharacterClass *pclass, const char * keyword)
{
	int i;

	assert(pclass);

	for( i = eaSize(&pclass->pchSpecialRestrictions)-1; i>=0; i--)
	{
		if( stricmp(pclass->pchSpecialRestrictions[i], keyword) == 0 )
			return true;
	}

	return false;
}

/**********************************************************************func*
* class_IsRespecLevelUp
*
*/
bool class_IsRespecLevelUp(const CharacterClass *pclass, int level)
{
	bool retval;
	int count;

	retval = false;

	devassert(pclass);

	if (pclass && pclass->piLevelUpRespecs)
	{
		count = 0;

		while (!retval && count < eaiSize(&(pclass->piLevelUpRespecs)))
		{
			if (pclass->piLevelUpRespecs[count] == (level + 1 + 1))
			{
				retval = true;
			}

			count++;
		}
	}

	return retval;
}

bool class_UsesAVStyleReduction(const char* characterClassName)
{
	const CharacterClass *pclass;
	
	pclass = classes_GetPtrFromName(&g_VillainClasses, characterClassName);

	if(pclass)
		return pclass->bReduceAsAV;
	return false;
}

const char* class_GetReduced(const char* characterClassName)
{
	const CharacterClass *pclass;
	
	pclass = classes_GetPtrFromName(&g_VillainClasses, characterClassName);

	if(pclass)
		return pclass->pchReductionClass; // NULL for not a reducible class
	return NULL;
}

/* End of File */
