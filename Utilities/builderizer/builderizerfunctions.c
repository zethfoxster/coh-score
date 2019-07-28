#include "builderizerfunctions.h"
#include "builderizer.h"
#include "functionlibrary.h"
#include "processes.h"
#include "utils.h"
#include "simpleparser.h"

#include <direct.h>

#define CHECKPARAM(p, t) {if((p)->type != (t))assert("Bad Param"==0);}

int bfFileExists( Parameter **args )
{
	char *str;
	int ret;
	CHECKPARAM(args[0], PARAMTYPE_STRING);
	str = doVariableReplacement(args[0]->sParam);
	ret = fileExists(str);
	estrDestroy(&str);
	return ret;
}

int bfFolderExists( Parameter **args )
{
	char *str;
	int ret;
	CHECKPARAM(args[0], PARAMTYPE_STRING);
	str = doVariableReplacement(args[0]->sParam);
	ret = dirExists(str);
	estrDestroy(&str);
	return ret;
}


int bfVarDefined( Parameter **args )
{
	char *str;
	int ret;
	CHECKPARAM(args[0], PARAMTYPE_STRING);
	str = doVariableReplacement(args[0]->sParam);
	ret = getBuildStateVar(str) ? 1 : 0;
	ret |= getBuildStateList(str) ? 1 : 0;
	estrDestroy(&str);
	return ret;
}

int bfProcIsRunning( Parameter **args )
{
	char *str;
	CHECKPARAM(args[0], PARAMTYPE_STRING);
	str = args[0]->sParam;
	return procIsRunning(str);
}

int bfListSizeEquals( Parameter **args )
{
	char *list, *val;
	char **listVals;
	CHECKPARAM(args[0], PARAMTYPE_STRING);
	CHECKPARAM(args[1], PARAMTYPE_INT);
	list = args[0]->sParam;
	val = args[1]->sParam;
	listVals = getBuildStateList(list);
	if ( eaSize(&listVals) == atoi(val) )
		return 1;
	return 0;
}

int bfListSize( Parameter **args )
{
	char *list;
	char **listVals;
	CHECKPARAM(args[0], PARAMTYPE_STRING);
	list = args[0]->sParam;
	listVals = getBuildStateList(list);
	return eaSize(&listVals);
}

int bfStripTrailingWhitespace( Parameter **args )
{
	const char *const_str;
	char *str;
	CHECKPARAM(args[0], PARAMTYPE_STRING);
	const_str = getBuildStateVar(args[0]->sParam);
	if ( !const_str )
		return 0;
	str = strdup(const_str);
	removeTrailingWhiteSpaces(str);
	setBuildStateVar(args[0]->sParam, str);
	free(str);
	return 1;
}

int bfStripLeadingWhitespace( Parameter **args )
{
	const char *const_str;
	char *str;
	CHECKPARAM(args[0], PARAMTYPE_STRING);
	const_str = getBuildStateVar(args[0]->sParam);
	if ( !const_str )
		return 0;
	const_str = removeLeadingWhiteSpaces(const_str);
	setBuildStateVar(args[0]->sParam, str = strdup(const_str));
	free(str);
	return 1;
}

int bfBackSlashes( Parameter **args )
{
	const char *const_str;
	char *str;
	CHECKPARAM(args[0], PARAMTYPE_STRING);
	const_str = getBuildStateVar(args[0]->sParam);
	if ( !const_str )
		return 0;
	str = strdup(const_str);
	str = backSlashes(str);
	setBuildStateVar(args[0]->sParam, str);
	free(str);
	return 1;
}

int bfMegabytesFreeOnDisk( Parameter **args )
{
	U64 bytes;
	char *str;
	CHECKPARAM(args[0], PARAMTYPE_STRING);
	str = args[0]->sParam;

	bytes = fileGetFreeDiskSpace(str);
	return (bytes / 1024) / 1024;
}


// the handler function for all builderizerfunction calls
int bfCallFunc( char *name, char **args )
{
	int ret = flibCallFunc(name, args);
	if ( ret == INVALID_FUNCTION_CALL )
	{
		builderizerError("function %s", name);
	}
	return ret;
}

void bfRegisterAllFunctions()
{
	flibRegisterFunc("FileExists", bfFileExists, 1, PARAMTYPE_STRING);
	flibRegisterFunc("FolderExists", bfFolderExists, 1, PARAMTYPE_STRING);
	flibRegisterFunc("VarDefined", bfVarDefined, 1, PARAMTYPE_STRING);
	flibRegisterFunc("ProcIsRunning", bfProcIsRunning, 1, PARAMTYPE_STRING);
	flibRegisterFunc("ListSizeEquals", bfListSizeEquals, 2, PARAMTYPE_STRING, PARAMTYPE_INT);
	flibRegisterFunc("ListSize", bfListSize, 1, PARAMTYPE_STRING);
	flibRegisterFunc("MegabytesFreeOnDisk", bfMegabytesFreeOnDisk, 1, PARAMTYPE_STRING);

	// string functions
	flibRegisterFunc("StripLeadingWhitespace", bfStripLeadingWhitespace, 1, PARAMTYPE_STRING);
	flibRegisterFunc("StripTrailingWhitespace", bfStripTrailingWhitespace, 1, PARAMTYPE_STRING);
	flibRegisterFunc("BackSlashes", bfBackSlashes, 1, PARAMTYPE_STRING);
}