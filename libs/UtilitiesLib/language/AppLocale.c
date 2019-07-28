#include "AppLocale.h"
#include "utils.h"
#include <string.h>
#include "stdtypes.h"
#include "RegistryReader.h"
#include <stdlib.h>
#include "version/AppRegCache.h"
#include "assert.h"
#include "file.h" //for isdevelopmentmode()
#include "error.h"

struct {
	char *name;
	char *alpha2;
	LanguageLocale windowsLocale;
	int isImplemented;
	int isUserSelectable;
	int isEuropean;
} LocaleTable[] = {
	// Do not change the order of these
	{"English",				"en",	LOCALE_ENGLISH,				1,1,0},	// Do not move this entry!
																		// Index 0 is used as the default locale.
	{"Test",				"test",	LOCALE_TEST,				0,0,0},
	{"ChineseTraditional",	"zh",	LOCALE_CHINESE_TRADITIONAL,	0,0,0},
	{"Korean",				"ko",	LOCALE_KOREAN,				0,0,0},
	{"Japanese",			"ja",	LOCALE_JAPANESE,			0,0,0},
	{"German",				"de",	LOCALE_GERMAN,				1,1,1},
	{"French",				"fr",	LOCALE_FRENCH,				1,1,1},
	{"Spanish",				"es",	LOCALE_SPANISH,				0,0,1},
};
STATIC_ASSERT(ARRAY_SIZE(LocaleTable) == LOCALE_ID_COUNT);

static int currentLocale = DEFAULT_LOCALE_ID;
static int currentRegion = REGION_NA;

char* locGetName(int localeID){
	// If the given locale ID runs off of the table, just return a default.
	// English is the default locale.
	if(!locIDIsValid(localeID))
		localeID = DEFAULT_LOCALE_ID;

	return LocaleTable[localeID].name;
}

char* locGetAlpha2(int localeID)
{
	if(!locIDIsValid(localeID))
		localeID = DEFAULT_LOCALE_ID;

	return LocaleTable[localeID].alpha2;
}

int locIsUserSelectable(int localeID)
{
	if(!locIDIsValid(localeID))
		return 0;

	return LocaleTable[localeID].isUserSelectable;
}

int locIsImplemented(int localeID)
{
	if(!locIDIsValid(localeID))
		return 0;

	return LocaleTable[localeID].isImplemented;
}

int locGetID(char* ID){
	int i;
	for(i = 0; i < ARRAY_SIZE(LocaleTable); i++){
		if(stricmp(LocaleTable[i].name, ID) == 0)
			return i;
	}

	return DEFAULT_LOCALE_ID;
}

int locGetWindowsID(int loc_idx)
{
	if (loc_idx < 0 || loc_idx >= ARRAY_SIZE(LocaleTable))
		loc_idx = 0;
	return  LocaleTable[loc_idx].windowsLocale;
}

int locIsEuropean(int localID)
{
	int retval;

	if (localID < 0 || localID >= ARRAY_SIZE(LocaleTable))
		localID = 0;

	retval = LocaleTable[localID].isEuropean;

	// English is spoken everywhere, so we need to look at the registry code
	// to figure out which version of the game we're running.
	if (localID == LOCALE_ID_ENGLISH && getCurrentRegion() == REGION_EU)
	{
		retval = 1;
	}

	return retval;
}

int locIsBritish(int localID)
{
	if (localID == LOCALE_ID_ENGLISH && locIsEuropean(localID))
		return 1;

	return 0;
}

int locGetIDByWindowsLocale(int locale)
{
	int i;
	for(i = 0; i < ARRAY_SIZE(LocaleTable); i++){
		if(LocaleTable[i].windowsLocale == locale)
			return i;
	}

	return DEFAULT_LOCALE_ID;
}


int locGetIDByNameOrID(char* str)
{
	int locale;

	if(strIsNumeric(str))
	{
		locale = atoi(str);
		if(!locIDIsValid(locale))
			locale = DEFAULT_LOCALE_ID;
	}
	else
		locale = locGetID(str);

	return locale;
}

int locGetMaxLocaleCount(void){
	return ARRAY_SIZE(LocaleTable);
}

int locIDIsValid(int localeID)
{
	devassertmsg(localeID == -1 || localeID == LOCALE_ID_ENGLISH, "Locale is not being set to english properly.");
	return localeID == LOCALE_ID_ENGLISH;
}

int locIDIsValidForPlayers(int localeID)
{
	return locIDIsValid(localeID) && LocaleTable[localeID].isUserSelectable;
}

static int locIDRegistryOverride = -1;

void locOverrideIDInRegistryForServersOnly(int localeID)
{
	if (locIDIsValidForPlayers(localeID))
		locIDRegistryOverride = localeID;
	else
		Errorf("Unsupported locale '%d' for servers!", localeID);
}

int locGetIDInRegistry(void){
	return LOCALE_ID_ENGLISH;
}

void locSetIDInRegistry(int localeID){
	RegReader reader;

	// Get the locale setting from the registry.
	reader = createRegReader();
	initRegReader(reader, regGetAppKey());
	rrWriteInt(reader, "Locale", localeID);
	destroyRegReader(reader);
}

int getCurrentLocale(void)
{
	return LOCALE_ID_ENGLISH;
}

void setCurrentLocale(int locale){
	
	locale = LOCALE_ID_ENGLISH;
}

int getCurrentRegion(void)
{
	return currentRegion;
}

void setCurrentRegion(int region)
{
	currentRegion = region;
}


