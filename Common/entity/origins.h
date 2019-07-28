/***************************************************************************
 *     Copyright (c) 2003-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#if (!defined(ORIGINS_H__)) || (defined(ORIGINS_PARSE_INFO_DEFINITIONS)&&!defined(ORIGINS_PARSE_INFO_DEFINED))
#define ORIGINS_H__

// ORIGINS_PARSE_INFO_DEFINITIONS must be defined before including this file
// to get the ParseInfo structures needed to read in files.

#include <stdlib.h> // for offsetof

typedef struct CharacterClass CharacterClass;

/***************************************************************************/
/***************************************************************************/

typedef struct CharacterOrigin
{
	// Defines the character origin, which at this point is nothing more
	//   than a bit of color.

	const char *pchName;

	const char *pchDisplayName;
	const char *pchDisplayHelp;
	const char *pchDisplayShortHelp;
		// Various strings for user-interaction

	const char *pchIcon;
		// The icon for this origin

} CharacterOrigin;

#ifdef ORIGINS_PARSE_INFO_DEFINITIONS
TokenizerParseInfo ParseCharacterOrigin[] =
{
	{ "{",                        TOK_START,  0 },
	{ "Name",                     TOK_STRING(CharacterOrigin, pchName, 0)                   },
	{ "DisplayName",              TOK_STRING(CharacterOrigin, pchDisplayName, 0)            },
	{ "DisplayHelp",              TOK_STRING(CharacterOrigin, pchDisplayHelp, 0)            },
	{ "DisplayShortHelp",         TOK_STRING(CharacterOrigin, pchDisplayShortHelp, 0)       },
	{ "Icon",                     TOK_STRING(CharacterOrigin, pchIcon, 0)                   },
	{ "}",                        TOK_END,    0 },
	{ "", 0, 0 }
};
#endif

/***************************************************************************/
/***************************************************************************/

typedef struct CharacterOrigins
{
	const CharacterOrigin **ppOrigins;
} CharacterOrigins;

#ifdef ORIGINS_PARSE_INFO_DEFINITIONS
TokenizerParseInfo ParseCharacterOrigins[] =
{
	{ "Origin", TOK_STRUCT(CharacterOrigins, ppOrigins, ParseCharacterOrigin)},
	{ "", 0, 0 }
};
#endif

/***************************************************************************/
/***************************************************************************/

extern SHARED_MEMORY CharacterOrigins g_CharacterOrigins;
extern SHARED_MEMORY CharacterOrigins g_VillainOrigins;

const CharacterOrigin *origins_GetPtrFromName(const CharacterOrigins *porigins, const char *pch);
	// Returns a pointer to the origin associated with the given name.

int origins_GetIndexFromName(const CharacterOrigins *porigins, const char *pch);
	// returns index to origin of given name

int origins_GetIndexFromPtr(const CharacterOrigins *porigins, const CharacterOrigin *porigin);
	// returns index to origin from ptr (faster)

int origin_IsAllowedForClass(const CharacterOrigin *porigin, const CharacterClass *pclass);
	// Returns if a given origin is allowed for the given class/archetype

#ifdef ORIGINS_PARSE_INFO_DEFINITIONS
#define ORIGINS_PARSE_INFO_DEFINED
#endif

#endif /* #ifndef ORIGINS_H__ */

/* End of File */

