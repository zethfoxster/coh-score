/***************************************************************************
 *     Copyright (c) 2004-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef CHARACTERINFO_H__
#define CHARACTERINFO_H__

#include "entPlayer.h"

#define XML_VERSION_STRING "1.20080111.0"

typedef struct ExistInfo
{
	int dbid;
	U32 uLastActive;
	 bool bMarkedOnline;
	bool bGenerated;
	U32 iCostumeCRC[MAX_COSTUMES];
    char *name;
} ExistInfo;

void characterInfoMonitor(void);

bool CheckGeneratedExistsInfo(int dbid, U32 lastactive);
char *UpdateExistsInfo(const char *pchSafeName, int dbid, U32 uLastActive);
void SetExistsInfoGenerated(const char *pchSafeName, int dbid);
bool ResetCostumeCRCs(int dbid);
char *MakeCharacterFilename(const char *pchSafeName, const char *pchAppend);
char *MakePictureFilename(const char *pchSafeName, const char *pchAppend);
char *MakeCharacterURL(const char *pchSafeName, const char *pchAppend);
char *MakePictureURL(const char *pchSafeName, const char *pchAppend);
void ConvertFilenameToLowercase(char *filename);

void LogBannedCharacter(char *pchName);
bool IsAuthnameOnBannedList(char *pchAuthname);

typedef enum RefreshModeType
{
    RefreshModeType_None,
    RefreshModeType_Normal,
    RefreshModeType_Always,
    RefreshModeType_Count,
} RefreshModeType;



typedef struct CharacterInfoSettings
{
	bool bDeleteMissing;
	bool bAlphaSubdir;
	int  iIntervalMinutes;
	U32  uLastUpdate;
	int  iSecsDelta;
	char *pchTemplateString;

	char achNameStart[32];
	char achNameEnd[32];
	char achLastUpdate[MAX_PATH];
	char achShardName[MAX_PATH];

	char achTemplate[MAX_PATH];
	char achRequestPath[MAX_PATH];
	char achImagePath[MAX_PATH];
	char achHTMLDestPath[MAX_PATH];
	char achCharacterImageDestPath[MAX_PATH];
	char achCostumeRequestPath[MAX_PATH];
	char achWebDBPath[MAX_PATH];

	char achImageWebRoot[MAX_PATH];
	char achCharacterWebRoot[MAX_PATH];
	char achCharacterImageWebRoot[MAX_PATH];

	char achLogFileName[MAX_PATH];

	bool bIncludeAccessLevelChars;
	bool bUseCaptions;
	bool bIgnoreLastUpdate;
	int iMinLevel;
	int iInactiveDeleteTimeDays;
	bool bNoHTML;
	bool bShowBanned;
    RefreshModeType RefreshMode;
	int iMinDiskFreeMB;
	int iLowDiskStallTime;
} CharacterInfoSettings;

extern CharacterInfoSettings g_CharacterInfoSettings;

#endif /* #ifndef CHARACTERINFO_H__ */

/* End of File */

