/***************************************************************************
 *     Copyright (c) 2005-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef SKILLOBJ_H__
#define SKILLOBJ_H__


typedef struct MapSkillObjectives
{
	const char *pchMap;
	const char *pchVillainGroup;
	const char **ppchSkillObjectives;
} MapSkillObjectives;


typedef struct MapSkillObjectivesDict
{
	const MapSkillObjectives **ppMapSkillObjectives;
} MapSkillObjectivesDict;


extern SHARED_MEMORY MapSkillObjectivesDict g_MapSkillObjectivesDict;
extern cStashTable g_hashMapsAndVillainsToSkillObjectives;


void load_SkillObjectives(void);
const char * const **skillobj_GetPossibleSkillObjectives(const char *pchMap, const char *pchVillainGroup);


#endif /* #ifndef SKILLOBJ_H__ */

/* End of File */

