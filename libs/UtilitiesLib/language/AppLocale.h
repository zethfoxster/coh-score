/* File AppLocale.h
 *	Contains mappings between locale ID and string.  There is probably an ANSI way to do this.
 *	
 */

#ifndef GAME_LOCALE_H
#define GAME_LOCALE_H

#define DEFAULT_LOCALE_ID LOCALE_ID_ENGLISH

char* locGetName(int localeID);
char* locGetAlpha2(int localeID);
int locIsUserSelectable(int localeID);
int locIsImplemented(int localeID);
int locGetID(char* ID);
int locGetIDByNameOrID(char* str);
int locGetMaxLocaleCount(void);
int locIDIsValid(int localID);
int locIDIsValidForPlayers(int localID);
int locGetIDByWindowsLocale(int locale);
int locGetWindowsID(int loc_idx);
int locIsEuropean(int localID);
int locIsBritish(int localID);

void locOverrideIDInRegistryForServersOnly(int localeID);
int locGetIDInRegistry(void);
void locSetIDInRegistry(int localeID);

int getCurrentLocale(void);
void setCurrentLocale(int locale);

int getCurrentRegion(void);
void setCurrentRegion(int region);

typedef enum
{
	LOCALE_ENGLISH = 1033,
	LOCALE_TEST = 1337,
	LOCALE_CHINESE_TRADITIONAL = 1028,
	LOCALE_CHINESE_SIMPLIFIED = 2052,
	LOCALE_CHINESE_HONGKONG = 3076,
	LOCALE_KOREAN = 1042,
	LOCALE_JAPANESE = 1041,
	LOCALE_GERMAN = 1031,
	LOCALE_FRENCH = 1036,
	LOCALE_SPANISH = 1034,
} LanguageLocale;

typedef enum
{
	LOCALE_ID_ENGLISH,                  // 0 UK and US!!! use locIsBritish()!
	LOCALE_ID_TEST,                     // 1
	LOCALE_ID_CHINESE_TRADITIONAL,      // 2
	LOCALE_ID_KOREAN,                   // 3
	LOCALE_ID_JAPANESE,                 // 4
	LOCALE_ID_GERMAN,                   // 5
	LOCALE_ID_FRENCH,                   // 6
	LOCALE_ID_SPANISH,                  // 7
	LOCALE_ID_COUNT,                    // 8
};

typedef enum
{
	REGION_NA			= 0,
	REGION_EU			= 1,
};
#endif
