/***************************************************************************
 *     Copyright (c) 2003-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef TITLES_H__
#define TITLES_H__


typedef struct TitleList
{
	char *pchOrigin;
	char **ppchTitles;
	char **ppchChooseList;
	int  *piGenders;
	int  *piAttach;
	int  *piListGenders;
	int  *piAdjectiveGenders;
} TitleList;

typedef struct TitleDictionary
{
	TitleList **ppOrigins;

} TitleDictionary;

extern TitleDictionary g_TitleDictionary;
extern TitleDictionary g_v_TitleDictionary;
extern TitleDictionary g_p_TitleDictionary;
extern TitleDictionary g_l_TitleDictionary;

void LoadTitleDictionaries(void);

typedef struct Entity Entity;

void title_set( Entity *e, int showThe, int commonIdx, int originIdx, int color1, int color2 );

#define COMMON_TITLE_LEVEL 14
#define ORIGIN_TITLE_LEVEL 24
#define UNLIMITED_TITLE_LEVEL 49

#endif /* #ifndef TITLES_H__ */

/* End of File */

