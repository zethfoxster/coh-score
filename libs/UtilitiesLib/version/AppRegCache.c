#ifndef _XBOX

#include "AppRegCache.h"
#include "wininclude.h"
#include "RegistryReader.h"
#include "stdtypes.h"
#include <memory.h>
#include "assert.h"
#include <stdio.h>
#include "strings_opt.h"
#include "utils.h"

#define CRYPTIC_REG_KEY		"HKEY_CURRENT_USER\\SOFTWARE\\Cryptic"
#define CUR_VER				"CurrentVersion"
#define LAST_VER			"LastVersion"
#define APPLIED_PATCH		"ApplyTarget"



#define REFRESH_REQUIRED 0

static struct{
	unsigned int curVersion		: 1;
	unsigned int lastVersion	: 1;
} RegRefreshFlags = {0};

// "CoH Server" is what most projects use, so it's the default for now.

char app_registry_name[128] = "CoH";

void regSetAppName(const char* appName)
{
	Strncpyt(app_registry_name, appName);
}

const char* regGetAppName(void)
{
	return app_registry_name;
}

const char *regGetAppKey(void)
{
	static char buf[1000];

	STR_COMBINE_SS(buf, "HKEY_CURRENT_USER\\SOFTWARE\\Cryptic\\", regGetAppName());
	
	return buf;
}

void regRefresh(void){
	// Set all registry refresh flags to "Refresh Required".
	ZeroStruct(&RegRefreshFlags);
}

const char* regGetCurrentVersion(const char* projectName){
	static char lastProjectName[MAX_PATH] = "";
	static char currentVersion[MAX_PATH] = "";
	static int valueExists = 1;
	
	if(stricmp(projectName, lastProjectName))
		RegRefreshFlags.curVersion = REFRESH_REQUIRED;

	// If a refresh of the local cache is required...
	if(REFRESH_REQUIRED == RegRefreshFlags.curVersion){
		RegReader reader;

		// grab the new value from the registry now.
		reader = createRegReader();
		initRegReaderEx(reader, "%s\\%s", CRYPTIC_REG_KEY, projectName);

		if(!rrReadString(reader, CUR_VER, currentVersion, MAX_PATH))
			valueExists = 0;
		else
			valueExists = 1;
			
		destroyRegReader(reader);
		RegRefreshFlags.curVersion = 1;
	}

	if(valueExists)
		return currentVersion;
	else
		return NULL;
}

const char* regGetLastVersion(const char* projectName){
	static char lastProjectName[MAX_PATH] = "";
	static char lastVersion[MAX_PATH] = "";
	static int valueExists = 1;
	
	if(stricmp(projectName, lastProjectName))
		RegRefreshFlags.lastVersion = REFRESH_REQUIRED;

	// If a refresh of the local cache is required...
	if(REFRESH_REQUIRED == RegRefreshFlags.lastVersion){
		RegReader reader;

		// grab the new value from the registry now.
		reader = createRegReader();
		initRegReaderEx(reader, "%s\\%s", CRYPTIC_REG_KEY, projectName);

		if(!rrReadString(reader, LAST_VER, lastVersion, MAX_PATH))
			valueExists = 0;
		else
			valueExists = 1;
			
		destroyRegReader(reader);
		RegRefreshFlags.lastVersion = 1;
	}

	if(valueExists)
		return lastVersion;
	else
		return NULL;
}

void regSetInstallationDir(const char* installDir){
	int opResult;
	DWORD disposition;
	HKEY hKey;	
	char subkey[1000];

	// Write the new user setting to the registry.
	//	Open the key to write to first.
	STR_COMBINE_SS(subkey, "SOFTWARE\\Cryptic\\", regGetAppName());
	opResult = RegCreateKeyExA(
		HKEY_CURRENT_USER,			// handle to open key
		subkey,	// subkey name
		0,						// reserved
		NULL,						// class string
		REG_OPTION_NON_VOLATILE,	// special options
		KEY_READ | KEY_WRITE ,		// desired security access
		NULL,						// inheritance
		&hKey,						// key handle 
		&disposition				// disposition value buffer
		);
				
	assert(ERROR_SUCCESS == opResult);
	
	opResult = RegSetValueExA(
		hKey,							// handle to key
		"Installation Directory",		// value name
		0,								// reserved
		REG_SZ,							// value type
		installDir,						// value data
		(DWORD)strlen(installDir)		// size of value data
		);
	
	assert(ERROR_SUCCESS == opResult);
}

/* Function regGetInstallationDir()
 *	Extracts the game installation directory from the registry.
 *
 *
 *
 */
const char* regGetInstallationDir(void){
	HKEY hKey;
	DWORD disposition;
	LONG regCreateResult;
	static char installationDir[MAX_PATH] = "";
	DWORD  installationDirSize = MAX_PATH;

	// If the installation directory has already been determined before,
	// just return it.
	if(installationDir[0])
		return installationDir;

	// Try to open a the key where City of Heroes info is stored.
	// If the key does not already exist, create it.
	{
		char	subkey[1000];

		STR_COMBINE_SS(subkey, "SOFTWARE\\Cryptic\\", regGetAppName());
		regCreateResult = RegCreateKeyExA(
							HKEY_CURRENT_USER,			// handle to open key
							subkey,	// subkey name
							0,						// reserved
							NULL,						// class string
							REG_OPTION_NON_VOLATILE,	// special options
							KEY_READ | KEY_WRITE ,		// desired security access
							NULL,						// inheritance
							&hKey,						// key handle 
							&disposition				// disposition value buffer
							);
		
		
		// If the key cannot be opened/created, the program cannot proceed in any way.
		if(ERROR_SUCCESS != regCreateResult){
			assert(regCreateResult);
			return NULL;
		}
		
		// If the create key call caused a new key to be created, no installation directory data
		// is available for retrieval.
		if(REG_CREATED_NEW_KEY == disposition)
			return NULL;
	}

	{
		DWORD valueType;
		int regQueryResult;

		regQueryResult = RegQueryValueExA(
							hKey,						// handle to key
							"Installation Directory",	// value name
							NULL,						// reserved
							&valueType,					// type buffer
							installationDir,			// data buffer
							&installationDirSize		// size of data buffer
							);

		// The key exists, but there is no value named "Installation Directory".
		if(ERROR_SUCCESS != regQueryResult){
			return NULL;
		}
		else{
			// If the value stored as "Installation Directory" is not a string, the value
			// cannot be used by the patcher client as intended.
			if(REG_SZ != valueType){
				LONG regDeleteResult;

				// Delete the useless value.
				regDeleteResult = RegDeleteValueA(
									hKey,						// handle to key
									"Installation Directory"	// value name
									);
				assert(ERROR_SUCCESS == regDeleteResult);
				return NULL;
			}
			else
				return (char*)installationDir;
		}
	}
}


static RegReader rr = 0;
static void initRR() {
	if (!rr) {
		rr = createRegReader();
		initRegReaderEx(rr, "%s", regGetAppKey());
	}
}

char *regGetAppString(const char *key, const char *deflt, char *dest, size_t dest_size)
{
	initRR();
	strcpy_s(dest, dest_size, deflt);
	rrReadString(rr, key, dest, (int)dest_size);
	return dest;
}

void regPutAppString(const char *key, const char *value)
{
	initRR();
	rrWriteString(rr, key, value);
}

int regGetAppInt(const char *key, int deflt)
{
	int value = deflt;
	initRR();
	if (!rrReadInt(rr, key, &value))
		return deflt;
	return value;
}

void regPutAppInt(const char *key, int value)
{
	initRR();
	rrWriteInt(rr, key, value);
}



#else

// A few xbox stubs, this should be replaced by a more generic function eventually


char *regGetAppString(const char *key, const char *deflt, char *dest, size_t dest_size)
{
	strcpy_s(dest, dest_size, deflt);
	return dest;
}

void regPutAppString(const char *key, const char *value)
{
}

int regGetAppInt(const char *key, int deflt)
{
	return deflt;
}

void regPutAppInt(const char *key, int value)
{
}


const char* regGetAppName(void)
{
	static char name[32];
	strcpy(name,"FIGHTCLUB");
	return name;
}

const char *regGetAppKey(void)
{
	static char name[32];
	strcpy(name,"FIGHTCLUB");
	return name;
}

const char* regGetCurrentVersion(const char* projectName)
{
	static char name[32];
	strcpy(name,"0.1");
	return name;
}

const char* regGetInstallationDir(void)
{
	static char name[32];
	strcpy(name,"FIGHTCLUB");
	return name;
}

void regSetAppName(const char* appName)
{

}

#endif






const char *regGetPatchValue(void)
{
	static bool gotPatchValue=false;
	static char patchValue[1024];

	if (!gotPatchValue) {
		regGetAppString("PatchValue", "", SAFESTR(patchValue));
		gotPatchValue = true;
	}
	return patchValue;
}

const char *regGetPatchBandwidth(void)
{
	static bool gotPatchBandwidth=false;
	static char patchBandwidth[1024];

	if (!gotPatchBandwidth) {
		regGetAppString("TransferRate", "", SAFESTR(patchBandwidth));
		gotPatchBandwidth = true;
	}
	return patchBandwidth;
}