#include "Version/AppVersion.h"
#include "Version/AppRegCache.h"
#include "sysutil.h"
#include <assert.h>
#include "utils.h"
#include "file.h"
#include <string.h>
#include <time.h>
#include "../network/crypt.h"

char app_client_name[128] = "CityOfHeroes.exe";

void appSetClientName(const char* appName)
{
	Strncpyt(app_client_name, appName);
}

const char* appGetClientName(void)
{
	return app_client_name;
}

static char override_version[256] = "";

void setExecutablePatchVersionOverride(const char *_override_version) // Used for TestClient if it knows it's running a different version than what the registry says
{
	strcpy(override_version, _override_version);
}

/* Function isExecutableInInstallDir()
 *	Returns:
 *		1 - if this executable is in the same directory as the CoH installation directory
 *		0 - otherwise.
 */
int isExecutableInInstallDir(void)
{
	static int result = -1;

	if(result == -1)
	{
		// Find the directory where the game is installed.
		
		const char* installationDirConst = regGetInstallationDir();
		char		installationDir[MAX_PATH];
		char*		executableDir;
		
		if(!installationDirConst)
			return 0;
			
		strcpy(installationDir, installationDirConst);

		// Find the directory where this executable is.
		executableDir = getExecutableName();
		assert(executableDir);
		getDirectoryName(executableDir);

		fileFixName(installationDir, installationDir);
		fileFixName(executableDir, executableDir);
		if(stricmp(installationDir, executableDir) == 0)
			result = 1;
		else
			result = 0;
	}
	return result;
}

const char *getDevVersion(void)
{
	static	char version[100];
	char	name[1000];
	char	timestr[26];
	SYSTEMTIME sys;

	getExecutableDir(name);
	strcatf(name,"/%s", appGetClientName());
	fileLastChangedWindows(name, &sys);
	sprintf_s(SAFESTR(timestr),"%04hd-%02hd-%02hd %02hd:%02hd:%02hd",sys.wYear,sys.wMonth,sys.wDay,sys.wHour,sys.wMinute,sys.wSecond);
	sprintf_s(SAFESTR(version),"dev: %s",timestr);
	return version;
}

static const char *getVersionIniInternal(void)
{
	static char version[100];
	static bool bFirstTime = true;
	static const char* pVersion = NULL;

	// try to read file just once first time thru
	if(bFirstTime)
	{
		char directoryName[MAX_PATH];
		char versionIniFileName[MAX_PATH];
		char *buffer;

		GetCurrentDirectory(sizeof(directoryName), directoryName);
		sprintf(versionIniFileName, "%s/version.ini", directoryName);
		buffer = fileAlloc(versionIniFileName, 0);
		if (buffer)
		{
			// Read version number
			char *cursor = strstr(buffer, "Version=");
			if (cursor != NULL && sscanf_s(cursor, "Version=%s", version, sizeof(version)) > 0)
			{
				// assign to static ptr
				pVersion = version;
			}
			free(buffer);
		}		
		bFirstTime = false;
	}

	return pVersion;
}

static const char* getExecutablePatchVersionInternal(const char* projectName)
{
	char *no_version_return="Could not find registry key";
	const char* ret;

	if (override_version[0])
		return override_version;

	if(!isExecutableInInstallDir() && fileIsUsingDevData()){
		return getDevVersion();
	}

	// Try to get the version out of the version.ini file before resorting to checking the registry
	ret = getVersionIniInternal();
	if (ret)
		return ret;

	if(!projectName){
		projectName = regGetAppName();
	}

	ret = regGetCurrentVersion(projectName);
	return ret?ret:no_version_return;
}

/* Function getExecutablePatchVersion()
*	This function tells whether the executable appears to be a patched version, and if so,
*	retrives the version number.
 *
 *	Note that the function is really dumb right now.  It doesn't check if the version recorded
 *	in the manifest name is the same as the one recorded in the registry.  It also doesn't perform
 *	any sort of checking on the game data files.
 *	
 *	Returns:
 *		valid string - The current patch version as recorded in the registry.
 *		NULL -	The current executable is not in the CoH installation directory or
 *				if no current version is recorded in the registry.
 *				
 */
const char* getExecutablePatchVersion(const char* projectName)
{
	return getExecutablePatchVersionInternal(projectName);
}

const char* getCompatibleGameVersion(void)
{
	// Get the coh patch version.
	// This allows the game and server patch to be updated independently.
	return getExecutablePatchVersionInternal(regGetAppName());
}

int checksumFile(const char *fname,unsigned int checksum[4])
{
	U8	*mem;
	int	len;

	mem = fileAlloc(fname,&len);
	if (!mem)
		return 0;
	cryptMD5Update(mem,len);
	cryptMD5Final(checksum);
	free(mem);
	return 1;
}

