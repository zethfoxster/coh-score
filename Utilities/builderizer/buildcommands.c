#include "utils.h"
#include "builderizer.h"
#include "buildcommands.h"
#include "builderizerfunctions.h"
#include "wininclude.h"
#include "psapi.h"
#include <direct.h>
#include <sys/stat.h>

#include "processes.h"
#include "fileutil2.h"

const char *lastProgramOutputFile = "lastout";


void runBuildCommandSystem( BuildCommand *command )
{
	S32 paramCount = eaSize(&command->params);
	char *cmdLine = NULL;
	int i;
	char *fileToRun, *procName = NULL, *newParam = doVariableReplacement(command->params[0]);
	int isList;

	if ( newParam[0] == '$' )
	{
		procName = newParam;
		newParam = doVariableReplacement(command->params[1]);
	}

	fileToRun = newParam;
	
	for(i = procName?2:1; i < eaSize(&command->params); i++){
		newParam = doVariableReplacementEx( command->params[i], &isList);
		if ( !isList && buildParamHasSpaces(newParam) && newParam[i] != '\"' )
			estrConcatf(&cmdLine, "%s\"%s\"", i ? " " : "", newParam);
		else
			estrConcatf(&cmdLine, "%s%s", i ? " " : "", newParam);
		estrDestroy(&newParam);
	}
	
	backSlashes(cmdLine);
	statePrintf( 1, 0, "Running Command: \"%s %s\".\n", fileToRun, cmdLine);
	
	if(g_BuildState->flags.doRunCommands && !g_BuildState->flags.simulate){
		// Run Command
		//system(cmdLine);
		if ( !isFullPath(fileToRun) )
		{
			char cwd[256];
			int len;
			_getcwd(cwd, 256);
			len = (int)strlen(cwd);
			if ( len > 255 )
				assert("Cwd is too large!" && 0);
			// add a slash to the end of the cwd
			cwd[len++] = '\\';
			cwd[len] = 0;
			estrInsert(&fileToRun, 0, cwd, (int)strlen(cwd));
		}

		runExternalCommand(fileToRun, cmdLine, procName, procName?1:0);
	}
	
	estrDestroy(&cmdLine);
	estrDestroy(&fileToRun);
}


void runBuildCommandRun( BuildCommand *command )
{
	S32 paramCount = eaSize(&command->params);
	char *cmdLine = NULL;
	int i;
	char *fileToRun, *procName = NULL, *newParam = doVariableReplacement(command->params[0]);
	int isList;
	char *fullcmdLine = NULL;
	int errorlevel = 0;

	if ( newParam[0] == '$' )
	{
		procName = newParam;
		newParam = doVariableReplacement(command->params[1]);
	}

	fileToRun = newParam;

	for(i = procName?2:1; i < eaSize(&command->params); i++){
		newParam = doVariableReplacementEx( command->params[i], &isList);
		if ( !isList && buildParamHasSpaces(newParam) && newParam[i] != '\"' )
			estrConcatf(&cmdLine, "%s\"%s\"", i ? " " : "", newParam);
		else
			estrConcatf(&cmdLine, "%s%s", i ? " " : "", newParam);
		estrDestroy(&newParam);
	}

	estrConcatf(&fullcmdLine, "%s%s", fileToRun, cmdLine);

	//backSlashes(cmdLine);
	//statePrintf( 1, 0, "Running Command: \"%s %s\".\n", fileToRun, cmdLine);
	statePrintf( 1, 0, "Running Command: \"%s\".\n", fullcmdLine);

	if(g_BuildState->flags.doRunCommands && !g_BuildState->flags.simulate){
		// Run Command
		errorlevel = system(fullcmdLine);
		setBuildStateVar("ERRORLEVEL", STACK_SPRINTF("%d", errorlevel));

		if(errorlevel){
			builderizerError("Process exit code: %d\n", errorlevel);
		}

#if 0
		if ( !isFullPath(fileToRun) )
		{
			char cwd[256];
			int len;
			_getcwd(cwd, 256);
			len = (int)strlen(cwd);
			if ( len > 255 )
				assert("Cwd is too large!" && 0);
			// add a slash to the end of the cwd
			cwd[len++] = '\\';
			cwd[len] = 0;
			estrInsert(&fileToRun, 0, cwd, (int)strlen(cwd));
		}

		runExternalCommand(fileToRun, cmdLine, procName, procName?1:0);
#endif
	}

	estrDestroy(&fullcmdLine);
	estrDestroy(&cmdLine);
	estrDestroy(&fileToRun);
}

void runBuildCommandPushDir( BuildCommand *command )
{
	if ( eaSize(&command->params) != 1 )
		statePrintf( 1, COLOR_BRIGHT|COLOR_RED, "Wrong Number of parameters passed to Cmd_PushDir\n");
	else
	{
		char* newParam = doVariableReplacement( command->params[0]);
		statePrintf( 1, 0, "Pushing Dir: \"%s\".\n", newParam);
		if ( !g_BuildState->dirStack )
		{
			// store the current dir on the stack
			char *cwd;
			estrCreate(&cwd);
			estrSetSize(&cwd, MAX_PATH + 1);
			_getcwd(cwd, estrLength(&cwd));
			eaCreate(&g_BuildState->dirStack);
			eaPush(&g_BuildState->dirStack, cwd);
		}
		_chdir(newParam);
		eaPush(&g_BuildState->dirStack, newParam);
	}
}

void runBuildCommandPopDir( BuildCommand *command )
{
	if ( eaSize(&command->params) != 0 )
		statePrintf( 1, COLOR_BRIGHT|COLOR_RED, "Wrong Number of parameters passed to Cmd_PopDir\n");
	else
	{
		char *dir = g_BuildState->dirStack ? eaPop(&g_BuildState->dirStack) : NULL;
		char *cwd = g_BuildState->dirStack ? g_BuildState->dirStack[eaSize(&g_BuildState->dirStack)-1] : NULL;
		statePrintf( 1, 0, "Popping Dir: \"%s\".  New Working Dir: %s.\n", dir, cwd);
		_chdir(cwd);
		estrDestroy(&dir);
	}
}

void runBuildCommandKillProcesses( BuildCommand *command )
{
	S32		count = eaSize(&command->params);
	int i;
	
	if(!count){
		statePrintf(1, COLOR_RED|COLOR_GREEN, "WARNING: No parameters for Cmd_KillProcesses: %s\n",
					getDisplayName(g_BuildState->curBuildStep));
		return;
	}

	//killProcessName = doVariableReplacement( command->params[0]);

	//statePrintf( 1, 0, "Killing processes that contain: \"%s\"\n", killProcessName);
	//
	//if(g_BuildState->flags.doRunCommands && !g_BuildState->flags.simulate){
	//	HANDLE *procs = NULL;
	//	int i;

	//	procFindProcesses(killProcessName, &procs);
	//	for ( i = 0; i < eaSizeUnsafe(&procs); ++i )
	//	{
	//		int startTime;
	//		statePrintf( 2, 0, "Killing process %5d: %s...", GetProcessId(procs[i]), killProcessName/*pe32.szExeFile*/);
	//		startTime = timeGetTime();
	//		procKillHandle(procs[i]);
	//		printf("Done (%dms)!\n", timeGetTime() - startTime);
	//	}
	//}

	for ( i = 0; i < count; ++i )
	{
		int exitCode;
		char *killProcessName = doVariableReplacement( command->params[i]);
		
		if(g_BuildState->flags.doRunCommands){
			// is the param a handle that we have stored?
			if ( killProcessName[0] == '$' )
			{
				PROCESS_INFORMATION *pi = procGetInfo(killProcessName);
				if ( procIsRunning(killProcessName) )
				{
					if ( pi ) {
						statePrintf( 2, 0, "Killing process %5d: %s...\n", pi->dwProcessId, killProcessName);
						if ( !g_BuildState->flags.simulate )
							procKillHandle(pi->hProcess);
					}
					else
						statePrintf( 2, COLOR_RED|COLOR_GREEN, "No process named %s...\n", killProcessName);
				}
				if ( !g_BuildState->flags.simulate ) {
					if ( !procGetExitCode(killProcessName, &exitCode) )
						statePrintf( 2, 0, "Failed to get exit code for %5d: %s\n", pi ? pi->dwProcessId : 0, killProcessName);
					else {
						if ( exitCode ) {
							builderizerError("Process %s(%5d) exited with %d\n", 
								killProcessName, pi ? pi->dwProcessId : 0, exitCode);
						}
					}
				}
			}
			// it isnt a stored handle, its a process name
			else
			{
				HANDLE *procs = NULL;
				int j;
				procFindProcesses(killProcessName, &procs);

				for ( j = 0; j < eaSizeUnsafe(&procs); ++j )
				{
					statePrintf( 2, 0, "Killing process %5d: %s...\n", GetProcessId(procs[j]), killProcessName);
					if ( !g_BuildState->flags.simulate )
						procKillHandle(procs[j]);
				}
			}
		}

		estrDestroy(&killProcessName);
	}
}

void runBuildCommandWaitForProcs( BuildCommand *command )
{
	S32		count = eaSize(&command->params);
	int i;
	
	if(!count){
		statePrintf(1, COLOR_RED|COLOR_GREEN, "WARNING: No parameters for Cmd_WaitForProcesses: %s\n",
					getDisplayName(g_BuildState->curBuildStep));
		return;
	}

	for ( i = 0; i < count; ++i )
	{
		int exitCode;
		char *waitForProcName = doVariableReplacement( command->params[i]);
		
		if(g_BuildState->flags.doRunCommands && !g_BuildState->flags.simulate){
			// is the param a handle that we have stored?
			if ( waitForProcName[0] == '$' )
			{
				PROCESS_INFORMATION *pi = procGetInfo(waitForProcName);
				if ( procIsRunning(waitForProcName) )
				{
					if ( pi ) {
						statePrintf( 2, 0, "Waiting for process %5d: %s...\n", pi->dwProcessId, waitForProcName);
						WaitForSingleObject(pi->hProcess, INFINITE);
					}
					else
						statePrintf( 2, COLOR_RED|COLOR_GREEN, "No process named %s...\n", waitForProcName);
				}
				if ( !procGetExitCode(waitForProcName, &exitCode) )
					statePrintf( 2, 0, "Failed to get exit code for %5d: %s\n", pi ? pi->dwProcessId : 0, waitForProcName);
				else {
					if ( exitCode ) {
						builderizerError("Process %s(%5d) exited with %d\n", 
							waitForProcName, pi ? pi->dwProcessId : 0, exitCode);
					}
				}
			}
			// it isnt a stored handle, its a process name
			else
			{
				HANDLE *procs = NULL;
				int j;
				procFindProcesses(waitForProcName, &procs);

				for ( j = 0; j < eaSizeUnsafe(&procs); ++j )
				{
					statePrintf( 2, 0, "Waiting for process %5d: %s...\n", GetProcessId(procs[j]), waitForProcName);
					procWait(procs[j]);
				}
			}
		}

		estrDestroy(&waitForProcName);
	}
}

typedef struct FileCallbackData 
{
	BuildState*		buildState;
	BuildCommand*	command;
	U32				recursive;
	U32				quiet;
} FileCallbackData;

FileScanAction deleteFileCallback( FileScanContext *context )
{
	FileCallbackData* data = context->userData;
	BuildState* buildState = data->buildState;
	
	if ( data->command->commandType == BUILDCMDTYPE_DELETE_FOLDER && (context->fileInfo->attrib & _A_SUBDIR)) {
		char *dirname = NULL;
		struct _finddata32_t* oldFileInfo = context->fileInfo;
		char *oldDir = context->dir;
		
		estrCreate(&dirname );
		estrConcatf(&dirname, "%s\\%s", context->dir, context->fileInfo->name);
		backSlashes(dirname);

		// clear out sub directories and files
		fileScanDirRecurseContext(dirname, context);
		context->fileInfo = oldFileInfo;
		context->dir = oldDir;

		if ( !data->quiet )
			statePrintfNoLog( 2, 0, "Deleting: %s\n", dirname);
		if(buildState->flags.doRunCommands && !buildState->flags.simulate){
			_rmdir(dirname);
		}
	}
	if(!(context->fileInfo->attrib & _A_SUBDIR)){
		char *fname = NULL;
		estrCreate(&fname);
		estrConcatf(&fname, "%s\\%s", context->dir, context->fileInfo->name);
		if ( !data->quiet )
			statePrintfNoLog( 2, 0, "Deleting: %s\n", fname);
		backSlashes(fname);

		if(buildState->flags.doRunCommands && !buildState->flags.simulate){
			// Make sure the file is there.
			if (fileExists(fname))
			{
				chmod(fname,_S_IREAD | _S_IWRITE);
				// Delete the file.
				if (unlink(fname) != 0)
				{
					builderizerError("could not delete File \"%s\".\n", context->dir);
				}
			}
		}
		estrDestroy(&fname);
	}
		
	return data->recursive ? FSA_EXPLORE_DIRECTORY : 0;
}

void runBuildCommandDeleteFiles( BuildCommand *command )
{
	S32		recursive = 0;
	char*	fileSpec = NULL;
	S32		i;
	int		quiet = 0;
	
	for(i = 0; i < eaSize(&command->params); i++){
		char* param = doVariableReplacement( command->params[i]);
		
		#define BEGIN_FLAGS if(1){if(0);
		#define IF_FLAG(x)	else if(!_stricmp(param, x))
		#define END_FLAGS	}
		
		BEGIN_FLAGS
			IF_FLAG("/s"){
				recursive = 1;
			}
			IF_FLAG("/q"){
				quiet = 1;
			}
			else if(param[0] == '/'){
				builderizerError("Invalid Cmd_DeleteFiles flag: %s\n", param);
			}
			else if(param[0]){
				if(fileSpec){
					statePrintf( 1, COLOR_RED|COLOR_GREEN, "WARNING: Found multiple file specs, ignoring second one:\n");
					statePrintf( 2, COLOR_RED|COLOR_GREEN, "Spec1: \"%s\"\n", fileSpec);
					statePrintf( 2, COLOR_RED|COLOR_GREEN, "Spec2: \"%s\"\n", param);
				}else{
					estrCopy2(&fileSpec, param);
				}
			}
		END_FLAGS
		
		#undef BEGIN_FLAGS
		#undef IF_FLAG
		#undef END_FLAGS
		
		estrDestroy(&param);
	}
	
	if(!fileSpec){
		builderizerError("No filespec specified for deletion!\n");
	}else{
		statePrintf( 1, 0, "Deleting files: %s\n", fileSpec);
		
		if(!fileIsAbsolutePath(fileSpec)){
			builderizerError("Not an absolute path: %s\n", fileSpec);
		}else{
			statePrintf( 2, 0, "Recursive: %s\n", recursive ? "ON" : "OFF");
			
			if(g_BuildState->flags.doRunCommands){
				char folderName[1000];
				FileScanContext context = {0};
				FileCallbackData data;
				
				strcpy_s(folderName, 1000, fileSpec);
				
				getDirectoryName(folderName);
				
				data.buildState = g_BuildState;
				data.command = command;
				data.recursive = recursive;
				data.quiet = quiet;
				
				context.userData = &data;
				context.processor = deleteFileCallback;
				
				fileScanDirRecurseContext(fileSpec, &context);
			}
		}

		estrDestroy(&fileSpec);
	}
}

void runBuildCommandDeleteFolder( BuildCommand *command )
{
	char*	folderToDelete = NULL;
	S32		i;
	int		quiet = 0;
	
	for(i = 0; i < eaSize(&command->params); i++){
		char* param = doVariableReplacement( command->params[i]);
		
		#define BEGIN_FLAGS if(1){if(0);
		#define IF_FLAG(x)	else if(!_stricmp(param, x))
		#define END_FLAGS	}
		
		BEGIN_FLAGS
			IF_FLAG("/q"){
				quiet = 1;
			}
			else if(param[0] == '/'){
				builderizerError("Invalid Cmd_DeleteFolder flag: %s\n", param);
			}
			else if(param[0]){
				if(folderToDelete){
					statePrintf( 1, COLOR_RED|COLOR_GREEN, "WARNING: Found multiple folders, ignoring second one:\n");
					statePrintf( 2, COLOR_RED|COLOR_GREEN, "Folder1: %s\n", folderToDelete);
					statePrintf( 2, COLOR_RED|COLOR_GREEN, "Folder2: %s\n", param);
				}else{
					estrCopy2(&folderToDelete, param);
				}
			}
		END_FLAGS
		
		#undef BEGIN_FLAGS
		#undef IF_FLAG
		#undef END_FLAGS
		
		estrDestroy(&param);
	}
	
	if(!folderToDelete){
		builderizerError("No folder specified for deletion!\n");
	}else{
		statePrintf( 1, 0, "Deleting folder: %s\n", folderToDelete);
		
		if(!fileIsAbsolutePath(folderToDelete)){
			builderizerError("Not an absolute path: %s\n", folderToDelete);
		}else{
			if(g_BuildState->flags.doRunCommands && !g_BuildState->flags.simulate){
				char folderName[1000];
				FileScanContext context = {0};
				FileCallbackData data;
				
				strcpy_s(folderName, 1000, folderToDelete);
				
				getDirectoryName(folderName);
				
				data.buildState = g_BuildState;
				data.command = command;
				data.quiet = quiet;
				
				context.userData = &data;
				context.processor = deleteFileCallback;
				
				fileScanDirRecurseContext(folderToDelete, &context);

				if(g_BuildState->flags.doRunCommands && !g_BuildState->flags.simulate)
					_rmdir(folderToDelete);
			}
		}

		estrDestroy(&folderToDelete);
	}
}

void runBuildCommandCopy( BuildCommand *command )
{
	int i;
	S32 paramCount = eaSize(&command->params);

	if ( paramCount < 2 )
		builderizerError("Cmd_Copy must specify at least 2 parameters (<src_file_or_folder> <dest_file_or_folder>).  Only %d parameters given\n", paramCount);
	else
	{
		int srcIsDir = 0, destIsDir = 0, overwriteExisting = 0;
		char cwd[MAX_PATH] = {0};
		char *dest = NULL, *src = NULL, *fileSpec = NULL, *fileList = NULL, *excludeList = NULL, 
			*recurseFlag = "/E", *otherParams = NULL, *newParam;

		// get source folder/file
		newParam = doVariableReplacement(command->params[0]);
		if ( strStartsWith(newParam, "/FILELIST:") )
		{
			char *f_list = strchr(newParam, ':');
			estrCreate(&fileList);
			estrConcatf(&fileList, "%s", ++f_list);
		}
		else if ( !isFullPath(newParam) )
		{
			getcwd(cwd, MAX_PATH);
			estrConcat(&src, cwd, (int)strlen(cwd));
			estrAppend(&src, &newParam);
		}
		else
			estrCopy(&src, &newParam);

		if ( !fileExists(src) )
			srcIsDir = 1;
		estrDestroy(&newParam);

		// get destination folder/file
		newParam = doVariableReplacement(command->params[1]);
		if ( !isFullPath(newParam) )
		{
			if ( !cwd[0] )
				getcwd(cwd, MAX_PATH);
			estrConcat(&dest, cwd, (int)strlen(cwd));
			estrAppend(&dest, &newParam);
		}
		else
			estrCopy(&dest, &newParam);

		if ( srcIsDir || dirExists(dest) )
			destIsDir = 1;
		estrDestroy(&newParam);

		for(i = 2; i < paramCount; i++)
		{
			newParam = doVariableReplacement(command->params[i]);
			//if ( buildParamHasSpaces(newParam) && newParam[i] != '\"' )
			//	estrConcatf(&cmdLine, "%s\"%s\"", i ? " " : "", newParam);
			//else
			//	estrConcatf(&cmdLine, "%s%s", i ? " " : "", newParam);
			if ( strStartsWith(newParam, "/FILELIST:") )
			{
				if ( fileList )
				{
					builderizerError("More than one file list specified in Cmd_Copy.\n");
					return;
				}
				else
				{
					char *f_list = strchr(newParam, ':');
					estrCreate(&fileList);
					estrConcatf(&fileList, "%s", ++f_list);
				}
			}
			else if ( strchr(newParam, '*') )
				estrCopy(&fileSpec, &newParam);
			else if ( strStartsWith(newParam, "/EXCLUDE:") )
			{
				if ( excludeList )
				{
					builderizerError("More than one exclude list specified in Cmd_Copy.\n");
					return;
				}
				else
				{
					char *e_list;
					estrCreate(&excludeList);
					estrConcatf(&excludeList, "%s", newParam);
					// set the slashes after the /EXCLUDE: to back slashes
					e_list = strchr(excludeList, ':');
					backSlashes(++e_list);

				}
			}
			else if (stricmp(newParam, "/Y") == 0 ||
				stricmp(newParam, "/E") == 0 ||
				stricmp(newParam, "/S") == 0 ||
				stricmp(newParam, "/I") == 0 ||
				stricmp(newParam, "/U") == 0 ||
				stricmp(newParam, "/A") == 0 ||
				stricmp(newParam, "/M") == 0 ||
				stricmp(newParam, "/P") == 0 ||
				stricmp(newParam, "/Q") == 0 ||
				stricmp(newParam, "/H") == 0 ||
				stricmp(newParam, "/R") == 0 ||
				stricmp(newParam, "/K") == 0 ||
				stricmp(newParam, "/V") == 0 ||
				stricmp(newParam, "/D") == 0)
			{
				if ( stricmp(newParam, "/Y") == 0 )
					overwriteExisting = 1;

				if ( !otherParams )
					estrCreate(&otherParams);
				estrConcatf(&otherParams, "%s ", newParam);
			}
			else
			{
				builderizerError("Unknown parameter in Cmd_Copy: %s\n", newParam);
				return;
			}
			estrDestroy(&newParam);
		}

		// do the copy
		if ( srcIsDir && !destIsDir )
		{
			builderizerError("Something (probably very stupid) happened and I think I am supposed to be copying a directory to a file.\
							 src = %s\tdest = %s\n", src, dest);
			return;
		}
		else if ( !srcIsDir )
		{
			if ( destIsDir )
				estrConcatf(&dest, strEndsWith(dest, "\\") || strEndsWith(dest, "/") ? "%s" : "/%s", getFileName(src));
			if ( !fileExists(dest) || overwriteExisting )
			{
				statePrintf(2, 0, "Copying %s to %s\n", src, dest);
				if(g_BuildState->flags.doRunCommands && !g_BuildState->flags.simulate)
					fileCopy(src, dest);
			}
			else
				statePrintf(2, 0, "File %s already exists\n", dest);

		}
		else if ( srcIsDir && destIsDir )
		{
			char *cmdLine = NULL;
			// if src exists, but there is a file list, this will hold <src>/<file_from_filelist>
			char *srcFull = NULL;
			if ( fileSpec )
			{
				if ( !(strEndsWith(src, "/") || strEndsWith(src, "\\")) )
					estrConcatf(&src, "/");
				estrAppend(&src, &fileSpec);
			}
			backSlashes(src);
			backSlashes(dest);
			makeDirectories(dest);
			if ( fileList )
			{
				char line[256];
				FILE *fl = fopen(fileList, "rt");
				if ( !fl )
				{
					builderizerError("File list \"%s\" does not exist!", fileList);
					return;
				}
				while ( fgets(line, 256, fl) )
				{
					int len = (int)strlen(line);
					if ( line[len-1] == '\n' )
						line[len-1] = 0;
					// append the cwd if necessary
					if ( !isFullPath(line) )
					{
						if ( !cwd[0] || !src )
						{
							getcwd(cwd, MAX_PATH);
							estrConcat(&src, cwd, (int)strlen(cwd));
							if ( !(strEndsWith(src, "/") || strEndsWith(src, "\\")) )
								estrConcatf(&src, "/");
							estrConcat(&srcFull, src, (int)strlen(src));
							estrConcat(&srcFull, line, (int)strlen(line));
							//estrConcat(&src, line, (int)strlen(line));
						}
						else if ( src )
						{
							// this should have been handled before, but i check it again just in case
							if ( !isFullPath(src) )
							{
								if ( !cwd[0] )
									getcwd(cwd, MAX_PATH);
								estrConcat(&src, cwd, (int)strlen(cwd));
							}
							if ( !(strEndsWith(src, "/") || strEndsWith(src, "\\")) )
								estrConcatf(&src, "/");
							estrConcat(&srcFull, src, (int)strlen(src));
							estrConcat(&srcFull, line, (int)strlen(line));
						}
					}
					backSlashes(srcFull);
					estrConcatf(&cmdLine, "xcopy \"%s\" \"%s\" %s %s", srcFull, dest, excludeList?excludeList:"", otherParams?otherParams:"");
					statePrintf(2, 0, "%s\n", cmdLine);
					if(g_BuildState->flags.doRunCommands && !g_BuildState->flags.simulate)
						system(cmdLine);
					estrClear(&cmdLine);
					estrClear(&srcFull);
				}
			}
			else
			{
				estrConcatf(&cmdLine, "xcopy \"%s\" \"%s\" %s %s", src, dest, excludeList?excludeList:"", otherParams?otherParams:"");
				statePrintf(2, 0, "%s\n", cmdLine);
				if(g_BuildState->flags.doRunCommands && !g_BuildState->flags.simulate)
					system(cmdLine);
			}
		}

		estrDestroy(&src);
		estrDestroy(&dest);
		estrDestroy(&fileSpec);
	}
 
}


void runBuildCommandRename( BuildCommand *command )
{
	S32 paramCount = eaSize(&command->params);
	int isList;

	if ( paramCount != 2 )
		builderizerError("Cmd_Rename must specify 2 parameters (<src_file_or_folder> <dest_file_or_folder>).  %d parameters given\n", paramCount);
	else
	{
		char *newParam;
		//char *cmdLine;
		char *oldName = NULL, *newName = NULL;

		//estrCreate(&cmdLine);
		//estrConcatf(&cmdLine, "rename ");

		newParam = doVariableReplacementEx( command->params[0], &isList );
		
		if ( !isList && buildParamHasSpaces(newParam) && newParam[0] != '\"' )
			estrConcatf(&oldName, "\"%s\"", newParam);
		else
			estrConcatf(&oldName, "%s", newParam);

		estrClear(&newParam);

		newParam = doVariableReplacementEx( command->params[1], &isList );

		if ( !isList && buildParamHasSpaces(newParam) && newParam[0] != '\"' )
			estrConcatf(&newName, "\"%s\"", newParam);
		else
			estrConcatf(&newName, "%s", newParam);

		estrClear(&newParam);

		//{
		//	char *c;
		//	// does this param have a full path specified?  if so, chop off all but the end
		//	if ( (c = strrchr(newParam, '/')) || (c = strrchr(newParam, '\\')) )
		//	{
		//		char buf[256] = {0};
		//		++c;
		//		strcpy_s(SAFESTR(buf), c);
		//		estrClear(&newParam);
		//		if ( buf[strlen(buf)-1] == '\"' )
		//			estrConcatf(&newParam, "\"");
		//		estrConcatf(&newParam, "%s", buf);
		//	}
		//}
		//
		//if ( !isList && buildParamHasSpaces(newParam) && newParam[0] != '\"' )
		//	estrConcatf(&cmdLine, "\"%s\"", newParam);
		//else
		//	estrConcatf(&cmdLine, "%s", newParam);

		estrDestroy(&newParam);

		if(g_BuildState->flags.doRunCommands && !g_BuildState->flags.simulate)
		{
			//backSlashes(cmdLine);
			//statePrintf( 1, 0, "\"%s\".\n", cmdLine);
			//system(cmdLine);
			statePrintf( 1, 0, "rename %s %s", oldName, newName);
			if ( rename(oldName, newName) != 0 )
				builderizerError("Could not rename folder %s %s", oldName, newName);
		}
	}
}

void runBuildCommandMakeFolder( BuildCommand *command )
{
	S32 paramCount = eaSize(&command->params);
	
	if(paramCount != 1){
		builderizerError("Cmd_MakeFolder takes 1 parameter, you gave %d.\n", paramCount);
	}
	else{
		char* folderToMake = doVariableReplacement( command->params[0]);
		
		if(!fileIsAbsolutePath(folderToMake)){
			builderizerError("Cmd_MakeFolder takes an absolute path, not this: \"%s\".\n", folderToMake);
		}
		else{
			statePrintf( 1, 0, "Making folder: \"%s\".\n", folderToMake);
			
			if(g_BuildState->flags.doRunCommands && !g_BuildState->flags.simulate){
				// Create the folder.
				
				makeDirectories(folderToMake);
			}
		}
		
		estrDestroy(&folderToMake);
	}
}

void runExternalCommand( const char *fileToRun, const char *cmdLine, char *procName, int detach )
{
	char fileToRunCopy[MAX_PATH], cwd[MAX_PATH];;
	strncpy_s(fileToRunCopy, MAX_PATH, fileToRun, strlen(fileToRun)+1);
	backSlashes(fileToRunCopy);

	if(!fileExists(fileToRunCopy)){
		builderizerError("Can't run: \"%s\".\n", fileToRunCopy);
	}
	else{
		STARTUPINFO			si = {0};
		char*				cmdLineCopy = NULL;
		PROCESS_INFORMATION pi = {0};
		
		estrPrintf(&cmdLineCopy, "\"%s\" %s", fileToRunCopy, cmdLine);

		if(g_BuildState->flags.doPrintExternalCommands){
			statePrintf( 1, COLOR_RED|COLOR_GREEN, "External: %s\n", cmdLineCopy);
		}

		if ( detach )
		{
			//spawnProcess( cmdLineCopy, _P_WAIT);
			si.cb = sizeof(si);
			// some programs (e.g. incredibuild) crash when run with DETACHED_PROCESS, since they dont make their own consoles.
			//CreateProcess(NULL, cmdLineCopy, NULL, NULL, TRUE, DETACHED_PROCESS|NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi);
			CreateProcess(NULL, cmdLineCopy, NULL, NULL, TRUE, CREATE_NEW_CONSOLE|NORMAL_PRIORITY_CLASS, NULL, _getcwd(cwd, MAX_PATH), &si, &pi);
			if ( procName )
				procStore(procName, &pi);
			else
			{
				CloseHandle(pi.hProcess);
				CloseHandle(pi.hThread);
			}
			estrDestroy(&cmdLineCopy);
			return;
		}
		else
		{
			HANDLE procOutputWrite;
			HANDLE procOutputRead;
			U32 exitCode = 0;
			SECURITY_ATTRIBUTES sa = {0};

			sa.nLength = sizeof(sa);
			sa.bInheritHandle = TRUE;
			sa.lpSecurityDescriptor = NULL;

			procOutputWrite = CreateFile(lastProgramOutputFile, GENERIC_WRITE, 
				FILE_SHARE_WRITE|FILE_SHARE_READ, &sa, OPEN_ALWAYS, 0, 0);
			procOutputRead = CreateFile(lastProgramOutputFile, GENERIC_READ, 
				FILE_SHARE_WRITE|FILE_SHARE_READ, &sa, OPEN_ALWAYS, 0, 0);

			si.cb = sizeof(si);
			si.dwFlags |= STARTF_USESTDHANDLES;
			si.hStdInput = stdin;
			si.hStdOutput = procOutputWrite;
			si.hStdError = procOutputWrite;
			
			if(!CreateProcess(NULL, cmdLineCopy, NULL, NULL, TRUE, 
				NORMAL_PRIORITY_CLASS /*0 ? 0 : 0 ? DETACHED_PROCESS : CREATE_NEW_CONSOLE*/, 
				NULL, _getcwd(cwd, MAX_PATH), &si, &pi))
			{
				builderizerError("Failed to start process: %s %s\n", fileToRunCopy, cmdLine ? cmdLine : "");
			}
			else
			{
				char buffer[10000] = "";
				DWORD bytesRead = 0, totalBytesRead = 0;
				S32 useSameLine = 0;
				DWORD size = 0;

				while ( WaitForSingleObject(pi.hProcess, 100) != WAIT_OBJECT_0)
				{
					if ( ReadFile(procOutputRead, buffer, sizeof(buffer), &bytesRead, NULL) )
					{
						totalBytesRead += bytesRead;
						printExternalMessage(buffer, &useSameLine);
					}
				}

				// read any of the file we might have missed
				while ( totalBytesRead < (size = GetFileSize(procOutputRead, NULL)) )
				{
					if ( ReadFile(procOutputRead, buffer, sizeof(buffer), &bytesRead, NULL) )
					{
						totalBytesRead += bytesRead;
						printExternalMessage(buffer, &useSameLine);
					}
					else
						break;
				}
			}

			if(GetExitCodeProcess(pi.hProcess, &exitCode))
			{
				setBuildStateVar("ERRORLEVEL", STACK_SPRINTF("%d", exitCode));

				if(exitCode){
					builderizerError("Process exit code: %d\n", exitCode);
				}
				else{
					statePrintf( 2, COLOR_GREEN, "Done!\n");
				}
			}
			else
			{
				builderizerError( "Can't get process exit code.\n");
			}
		
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
			CancelIo(procOutputWrite);
			CloseHandle(procOutputWrite);
			CloseHandle(procOutputRead);
		}

		estrDestroy(&cmdLineCopy);
	}
}

void runBuildCommandGimme( BuildCommand *command )
{
	const char* gimmePath = getBuildStateVar( "Path_Gimme");
	char *procName = NULL;
	int isList;
	
	if(!gimmePath){
		builderizerError("Path_Gimme not set.\n");
	}
	else if(!fileExists(gimmePath)){
		builderizerError("Path_Gimme doesn't exist: %s\n", gimmePath);
	}
	else{
		char* cmdLine = NULL;
		S32 i;

		for(i = 0; i < eaSize(&command->params); i++){
			char* newParam = doVariableReplacementEx( command->params[i], &isList);

			if ( newParam[0] == '$' )
				procName = newParam;
			else
			{
				if ( !isList && buildParamHasSpaces(newParam) && newParam[i] != '\"' )
					estrConcatf(&cmdLine, "%s\"%s\"", i ? " " : "", newParam);
				else
					estrConcatf(&cmdLine, "%s%s", i ? " " : "", newParam);
				estrDestroy(&newParam);
			}
		}
		
		statePrintf( 1, 0, "Running: %s %s\n", gimmePath, cmdLine);
		
		if(g_BuildState->flags.doRunCommands && !g_BuildState->flags.simulate){
			// Run the command.
			runExternalCommand( gimmePath, cmdLine, procName, procName?1:0);
		}
		
		estrDestroy(&cmdLine);
		if ( procName )
			estrDestroy(&procName);
	}
}

void runBuildCommandSVN( BuildCommand *command )
{
	char* svnPath = getFileVar( "Path_SVN");
	char *procName = NULL;
	int isList;
	
	if(svnPath){
		char*	cmdLine = NULL;
		S32		i;
		
		for(i = 0; i < eaSize(&command->params); i++){
			char* newParam = doVariableReplacementEx( command->params[i], &isList);

			if ( newParam[0] == '$' )
				procName = newParam;
			else
			{
				if ( !isList && buildParamHasSpaces(newParam) && newParam[i] != '\"' )
					estrConcatf(&cmdLine, "%s\"%s\"", i ? " " : "", newParam);
				else
					estrConcatf(&cmdLine, "%s%s", i ? " " : "", newParam);
				estrDestroy(&newParam);
			}
		}
		
		statePrintf( 1, 0, "Running: %s %s\n", svnPath, cmdLine);
		
		if(g_BuildState->flags.doRunCommands && !g_BuildState->flags.simulate){
			runExternalCommand( svnPath, cmdLine, procName, procName?1:0);
		}
		
		estrDestroy(&cmdLine);
		if ( procName )
			estrDestroy(&procName);
	}
	
	estrDestroy(&svnPath);
}

void runBuildCommandInFolder( BuildCommand *command, const char *cmdName, const char *folderVarName )
{
	char* folderToRunFrom = getFolderVar( folderVarName);
	char *procName = NULL;
	int isList;

	if(folderToRunFrom){
		S32 paramCount = eaSize(&command->params);
		
		if(!paramCount){
			builderizerError("%s: No params specified.\n", cmdName);
		}
		else{	
			char* toolPath = NULL;
			char* newParam = doVariableReplacement( command->params[0]);

			if ( newParam[0] == '$' )
			{
				procName = newParam;
				newParam = doVariableReplacement( command->params[1]);
			}
			
			estrPrintf(&toolPath, "%s/%s", folderToRunFrom, newParam);
			
			estrDestroy(&newParam);

			if(!fileExists(toolPath)){
				builderizerError("%s doesn't exist: %s\n", cmdName, toolPath);
			}else{
				char*	cmdLine = NULL;
				S32		i;
				
				for(i = procName?2:1; i < paramCount; i++){
					newParam = doVariableReplacementEx( command->params[i], &isList );
					
					if ( !isList && buildParamHasSpaces(newParam) && newParam[i] != '\"' )
						estrConcatf(&cmdLine, "%s\"%s\"", i ? " " : "", newParam);
					else
						estrConcatf(&cmdLine, "%s%s", i ? " " : "", newParam);
					
					estrDestroy(&newParam);
				}
				
				//forwardSlashes(toolPath);
				//forwardSlashes(cmdLine);
				backSlashes(toolPath);
				//backSlashes(cmdLine);
				
				statePrintf( 1, 0, "Running: %s %s\n", toolPath, cmdLine ? cmdLine : "");
				
				if(g_BuildState->flags.doRunCommands && !g_BuildState->flags.simulate){
					runExternalCommand( toolPath, cmdLine, procName, procName?1:0);
				}

				estrDestroy(&cmdLine);
				if ( procName )
					estrDestroy(&procName);
			}
				
			estrDestroy(&toolPath);
		}
	}
	
	estrDestroy(&folderToRunFrom);
}

void runBuildCommandTool( BuildCommand *command )
{
	runBuildCommandInFolder( command, "Cmd_Tool", "Folder_Tools");
}

void runBuildCommandCompile( BuildCommand *command )
{
	char* compilerPath = getFileVar( "Path_Compiler");
	//char* srcFolder = getFolderVar( "Folder_Src");
	char *procName = NULL;
	int isList;

	if(/*srcFolder &&*/ compilerPath)
	{
		S32 paramCount = eaSize(&command->params);
		
		if(!paramCount){
			statePrintf( 1, COLOR_RED, "No parameters to Cmd_Compile.\n");
		}
		else{
			char*	fileToBuild = doVariableReplacement( command->params[0]);
			char*	cmdLine = NULL;
			S32		i;
			
			if (fileToBuild[0] == '$')
			{
				procName = fileToBuild;
				fileToBuild = doVariableReplacement( command->params[1]);
			}
			// Make the file to build.
			
			//estrInsert(&fileToBuild, 0, "/", 1);
			//estrInsert(&fileToBuild, 0, srcFolder, (int)strlen(srcFolder));
			//forwardSlashes(fileToBuild);
			
			estrPrintf(&cmdLine, "\"%s\"", fileToBuild);
			
			// Get the parameters.
			
			for(i = procName?2:1; i < paramCount; i++){
				char* newParam = doVariableReplacementEx( command->params[i], &isList );

				if ( !isList && buildParamHasSpaces(newParam) )
					estrConcatf(&cmdLine, " \"%s\"", newParam);
				else
					estrConcatf(&cmdLine, " %s", newParam);
				
				estrDestroy(&newParam);
			}
			
			forwardSlashes(cmdLine);
			
			statePrintf( 1, 0, "Building: %s\n", cmdLine);
			
			if(g_BuildState->flags.doRunCommands && !g_BuildState->flags.simulate){
				// Run the build command.
				runExternalCommand( compilerPath, cmdLine, procName, procName?1:0);
			}
			
			estrDestroy(&fileToBuild);
			estrDestroy(&cmdLine);
			if ( procName )
				estrDestroy(&procName);
		}
	}
		
	//estrDestroy(&devEnvPath);
	//estrDestroy(&srcFolder);
}

void runBuildCommandBuild( BuildCommand *command )
{
	runBuildCommandInFolder( command, "Cmd_Build", "Folder_Build");
}

void runBuildCommandSrcExe( BuildCommand *command )
{
	runBuildCommandInFolder( command, "Cmd_SrcExe", "Folder_SrcExe");
}

void runBuildCommandFunction( BuildCommand *command )
{
	int i;
	char *func = NULL;
	char **args = NULL;

	func = doVariableReplacement( command->params[0] );

	for(i = 1; i < eaSize(&command->params); i++)
	{
		char* newParam = doVariableReplacement( command->params[i]);
		eaPush(&args, newParam);
	}

	bfCallFunc( func, args );
	estrDestroy(&func);
	for(i = 0; i < eaSize(&args); i++)
	{
		estrDestroy(&args[i]);
	}
	eaDestroy(&args);
}

void runBuildCommandWriteLogFile( BuildCommand *command )
{
	char* newParam;
	if ( eaSize(&command->params) > 1 )
		builderizerError("Invalid number of commands passed to \"Cmd_WriteLogFile:\".  Format should be: Cmd_WriteLogFile: \"[message]\"");
	newParam = doVariableReplacement( command->params[0]);
	estrConcatChar(&newParam, '\n');
	builderizerLogf("%s", newParam);
	estrDestroy(&newParam);
}


/*****************************************************************************************
Misc Commands
*****************************************************************************************/


void runBuildCommandPrint( BuildCommand *command )
{
	char* newParam;
	if ( eaSize(&command->params) > 1 )
		builderizerError("Invalid number of commands passed to \"Print:\".  Format should be: Print: \"[message]\"");
	newParam = doVariableReplacement( command->params[0]);
	estrConcatChar(&newParam, '\n');
	statePrintf(1, 0, "%s", newParam);
	estrDestroy(&newParam);
}

void runBuildCommandError( BuildCommand *command )
{
	char* newParam;
	if ( eaSize(&command->params) > 1 )
		builderizerError("Invalid number of commands passed to \"Error:\".  Format should be: Error: \"[message]\"");
	newParam = doVariableReplacement( command->params[0]);
	estrConcatChar(&newParam, '\n');
	builderizerError("%s", newParam);
	estrDestroy(&newParam);
}

void runBuildCommandSleep( BuildCommand *command )
{
	char* newParam;
	int time = 0;
	if ( eaSize(&command->params) > 1 )
		builderizerError("Invalid number of commands passed to \"Sleep:\".  Format should be: Sleep: \"[time in seconds]\"");
	newParam = doVariableReplacement( command->params[0]);
	time = atoi(newParam);
	Sleep(time * 1000);
	estrDestroy(&newParam);
}


/*****************************************************************************************
List and Variable Commands
*****************************************************************************************/

int listSortCmp(const char**a, const char**b)
{
	if ( strIsNumeric(*a) && strIsNumeric(*b) )
	{
		int iA = atoi((*a));
		int iB = atoi((*b));

		return iA < iB ? -1 : (iA > iB ? 1 : 0);
	}
	else
		return strcmp((*a), (*b));
}

void runBuildCommandListSort( BuildCommand *command )
{
	char *listName = doVariableReplacement( command->params[0]);
	char **values;

	if ( !isBuildStateList(listName) )
	{
		builderizerError("Could not find list %s...", listName);
		return;
	}

	values = getBuildStateList(listName);

	eaQSort(values, listSortCmp);
}

void runBuildCommandVarAdd( BuildCommand *command )
{
	if ( eaSize(&command->params) != 2 )
	{
		builderizerError("Wrong number of params for Var_Add,  2 required, you gave %d", eaSize(&command->params) );
	}
	else
	{
		const char *val = getBuildStateVar(command->params[0]);
		char newVal[256];
		int a = atoi(val), b = atoi(command->params[1]);

		if ( !strIsNumeric(val) || !strIsNumeric(command->params[1]) )
		{
			builderizerError("Invalid syntax for Var_Add:  \"%s - %s\"", val, command->params[0] );
		}

		itoa(a+b, newVal, 10);
		setBuildStateVar(command->params[0], newVal);
	}
}

void runBuildCommandVarSub( BuildCommand *command )
{
	if ( eaSize(&command->params) != 2 )
	{
		builderizerError("Wrong number of params for Var_Sub,  2 required, you gave %d", eaSize(&command->params) );
	}
	else
	{
		const char *val = getBuildStateVar(command->params[0]);
		char newVal[256];
		int a = atoi(val), b = atoi(command->params[1]);

		if ( !strIsNumeric(val) || !strIsNumeric(command->params[1]) )
		{
			builderizerError("Invalid syntax for Var_Sub:  \"%s - %s\"", val, command->params[0] );
		}

		itoa(a-b, newVal, 10);
		setBuildStateVar(command->params[0], newVal);
	}
}

void runBuildCommandVarMul( BuildCommand *command )
{
	if ( eaSize(&command->params) != 2 )
	{
		builderizerError("Wrong number of params for Var_Mul,  2 required, you gave %d", eaSize(&command->params) );
	}
	else
	{
		const char *val = getBuildStateVar(command->params[0]);
		char newVal[256];
		int a = atoi(val), b = atoi(command->params[1]);

		if ( !strIsNumeric(val) || !strIsNumeric(command->params[1]) )
		{
			builderizerError("Invalid syntax for Var_Mul:  \"%s - %s\"", val, command->params[0] );
		}

		itoa(a*b, newVal, 10);
		setBuildStateVar(command->params[0], newVal);
	}
}

void runBuildCommandVarDiv( BuildCommand *command )
{
	if ( eaSize(&command->params) != 2 )
	{
		builderizerError("Wrong number of params for Var_Div,  2 required, you gave %d", eaSize(&command->params) );
	}
	else
	{
		const char *val = getBuildStateVar(command->params[0]);
		char newVal[256];
		int a = atoi(val), b = atoi(command->params[1]);

		if ( !strIsNumeric(val) || !strIsNumeric(command->params[1]) )
		{
			builderizerError("Invalid syntax for Var_Div:  \"%s - %s\"", val, command->params[0] );
		}

		itoa(a/b, newVal, 10);
		setBuildStateVar(command->params[0], newVal);
	}
}

/*****************************************************************************************
Proc Commands
*****************************************************************************************/

void runBuildCommandProcKill( BuildCommand *command )
{
	char *procName = doVariableReplacement( command->params[0]);
	PROCESS_INFORMATION *pi = NULL;

	//pi = procGetInfo(procName[0] == '$' ? &procName[1] : procName);
	pi = procGetInfo(procName);

	if ( !pi )
	{
		builderizerError("Could not find process %s...", procName);
		return;
	}

	if (g_BuildState->flags.doRunCommands && !g_BuildState->flags.simulate)
	{
		procKill(pi);
	}
}