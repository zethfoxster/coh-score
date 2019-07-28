#include "relay_util.h"
#include "utils.h"
#include <sys/stat.h>
#include "sysutil.h"
#include "AppRegCache.h"
#include "file.h"



extern CmdRelayCmdStatus g_status;
extern char g_lastMsg[2000];
extern CRITICAL_SECTION g_criticalSection;

BOOL execAndQuit(char *exe_name, char * cmd)
{
	STARTUPINFO			si = {0};
	PROCESS_INFORMATION	pi = {0};
	int					result;
	si.cb				= sizeof(si);

	if(GetFileAttributes(exe_name) == INVALID_FILE_ATTRIBUTES)
		return FALSE;

	result = CreateProcess(exe_name, cmd,
		NULL,			NULL,			FALSE,			CREATE_NEW_CONSOLE | CREATE_NEW_PROCESS_GROUP,
		NULL,			NULL,			&si,			&pi);

	// TODO: Make sure that new process is running

	if(result)
	{
		exit(0);	// terminate this process
	}
	else
	{
		return FALSE;
	}
}



BOOL safeRenameFile(char *oldnamep,char *newnamep)
{
	char	oldname[MAX_PATH],newname[MAX_PATH];

	strcpy(oldname,oldnamep);
	strcpy(newname,newnamep);
	backSlashes(oldname);
	backSlashes(newname);
	chmod(oldname,_S_IREAD | _S_IWRITE);
	if (rename(oldname,newname) == 0)
		return TRUE;
	if (!fileExists(newname))
	{
		goto fail;
	}
	chmod(newname,_S_IREAD | _S_IWRITE);
	if (unlink(newname) != 0)
	{
		if (strEndsWith(newname,".exe"))
		{
			char	*s,new_exename[MAX_PATH];

			strcpy(new_exename,newname);
			s = strrchr(new_exename,'.');
			strcpy(s,".old");
			safeRenameFile(newname,new_exename);
		}
		else
			return FALSE;
	}
	if (rename(oldname,newname) == 0)
		return TRUE;
fail:
	return FALSE;
}


BOOL copyAndRestartInTempDir()
{
	char fullPath[] = "tmp\\CmdRelay.exe";

	mkdirtree(fullPath);

	printf("Renaming to '%s'\n", fullPath);

	if(fileCopy(getExecutableName(), fullPath))
	{
		printf("Error: Another instance appears to be running. Terminating program.");
		Sleep(2000);
		exit(0);
	}

	return execAndQuit(fullPath, GetCommandLine());	// will terminate inside this function on success
}