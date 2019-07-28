#include "builderizer.h"
#include "stdtypes.h"
#include "utils.h"
#include <conio.h>
#include <direct.h>
#include <io.h>

#include "wininclude.h"
#include "timing.h"
#include "buildcommands.h"
#include "builderizerclient.h"
#include "builderizerserver.h"
#include "builderizerfunctions.h"
#include "builderizerconfig.h"
#include "buildstatus.h"
#include "fileutil2.h"


#define TOK_RSTRUCT		TOK_REDUNDANTNAME | TOK_STRUCT_NORECURSE | TOK_STRUCT
#define TOK_STRINGPARAM	"", TOK_STRUCTPARAM | TOK_STRING
#define STANDARD_START_END	{"{", TOK_START}, {"}", TOK_END}
#define STANDARD_TERMINATOR	{0},

static ParseTable tpiBuildInputVarChoice[] = {
	{					TOK_STRINGPARAM(BuildVarChoice, displayName, 0)},
	{					TOK_STRINGPARAM(BuildVarChoice, value, 0)},
	{"\n",				TOK_END},
	STANDARD_TERMINATOR
};

static ParseTable tpiBuildInputVar[] = {
	STANDARD_START_END,
	{"Name:",			TOK_STRING(BuildVar, name, 0)},
	{"Value:",			TOK_STRING(BuildVar, value, 0)},
	{"Prompt:",			TOK_STRING(BuildVar, prompt, 0)},
	{"Choice:",			TOK_STRUCT(BuildVar, choices, tpiBuildInputVarChoice) },
	STANDARD_TERMINATOR
};

static ParseTable tpiBuildVar[] = {
	{					TOK_STRINGPARAM(BuildVar, name, 0)},
	{					TOK_STRINGPARAM(BuildVar, value, 0)},
	{"\n",				TOK_END},
	STANDARD_TERMINATOR
};


static ParseTable tpiBuildList[] = {
	{					TOK_STRINGPARAM(BuildList, name, 0)},
	{"",				TOK_STRUCTPARAM | TOK_STRINGARRAY(BuildList, values), },
	{"\n",				TOK_END},
	STANDARD_TERMINATOR
};

#define CONDITION_CONTENTS(type)																\
	{"Type:",			TOK_INT(BuildCondition, ifType, type)},									\
	{"",				TOK_STRUCT(BuildCondition, conditions, tpiBuildCondition)},				\
	{"if",				TOK_RSTRUCT(BuildCondition, conditions, tpiBuildConditionIfBlock)},		\
	{"if:",				TOK_RSTRUCT(BuildCondition, conditions, tpiBuildConditionIfLine)},		\
	{"&&",				TOK_RSTRUCT(BuildCondition, conditions, tpiBuildConditionAndBlock)},	\
	{"&&:",				TOK_RSTRUCT(BuildCondition, conditions, tpiBuildConditionAndLine)},		\
	{"||",				TOK_RSTRUCT(BuildCondition, conditions, tpiBuildConditionOrBlock)},		\
	{"||:",				TOK_RSTRUCT(BuildCondition, conditions, tpiBuildConditionOrLine)}

extern ParseTable tpiBuildCondition[];
extern ParseTable tpiBuildConditionIfBlock[];
extern ParseTable tpiBuildConditionIfLine[];
extern ParseTable tpiBuildConditionAndBlock[];
extern ParseTable tpiBuildConditionAndLine[];
extern ParseTable tpiBuildConditionOrBlock[];
extern ParseTable tpiBuildConditionOrLine[];

#define MAKE_BLOCK(name, type)				\
	static ParseTable name[] = {	\
		STANDARD_START_END,					\
		CONDITION_CONTENTS(type),			\
		STANDARD_TERMINATOR					\
	}
	
MAKE_BLOCK(tpiBuildConditionIfBlock,	IFTYPE_IF_BLOCK);
MAKE_BLOCK(tpiBuildConditionAndBlock,	IFTYPE_AND_BLOCK);
MAKE_BLOCK(tpiBuildConditionOrBlock,	IFTYPE_OR_BLOCK);

#undef MAKE_BLOCK

#define MAKE_LINE(name, type)																		\
	static ParseTable name[] = {															\
		{"Type:",			TOK_INT(BuildCondition, ifType, type)},									\
		{"",				TOK_STRUCTPARAM | TOK_STRINGARRAY(BuildCondition, params),		0},		\
		{"\n",				TOK_END},																\
		STANDARD_TERMINATOR																			\
	}

MAKE_LINE(tpiBuildConditionIfLine,	IFTYPE_IF_LINE);
MAKE_LINE(tpiBuildConditionAndLine, IFTYPE_AND_LINE);
MAKE_LINE(tpiBuildConditionOrLine,	IFTYPE_OR_LINE);

#undef MAKE_LINE

static ParseTable tpiBuildCondition[] = {
	STANDARD_START_END,

	CONDITION_CONTENTS(IFTYPE_IF_BLOCK),
	//
	{"Params:",			TOK_STRINGARRAY(BuildCondition, params)},
	//
	STANDARD_TERMINATOR
};

#define MAKE_COMMAND_BEGIN(name, type)															\
	static ParseTable name[] = {														\
		{"Type:",			TOK_INT(BuildCommand, commandType,	type)},

#define MAKE_COMMAND_END()																		\
		STANDARD_TERMINATOR																		\
	}

#define MAKE_COMMAND(name, type)																\
	MAKE_COMMAND_BEGIN(name, type)																\
		{"",				TOK_STRUCTPARAM | TOK_STRINGARRAY(BuildCommand, params)},			\
		{"\n",				TOK_END},															\
	MAKE_COMMAND_END()

MAKE_COMMAND_BEGIN(tpiBuildCommandStoreCmd, 0)
	STANDARD_START_END,
	{"Type:",			TOK_INT(BuildCommand, commandType, 0)},
	{"Commands:",		TOK_STRINGARRAY(BuildCommand, params)},
	{"FileName:",		TOK_STRING(BuildCommand, writeLogFile.fileName, 0)},
	{"Text:",			TOK_STRINGARRAY(BuildCommand, writeLogFile.textToWrite)},
	{"\n",				TOK_END},
MAKE_COMMAND_END();

MAKE_COMMAND(tpiBuildCommandSystem,			BUILDCMDTYPE_SYSTEM);
MAKE_COMMAND(tpiBuildCommandRun,			BUILDCMDTYPE_RUN);
MAKE_COMMAND(tpiBuildCommandPushDir,		BUILDCMDTYPE_PUSHDIR);
MAKE_COMMAND(tpiBuildCommandPopDir,			BUILDCMDTYPE_POPDIR);
MAKE_COMMAND(tpiBuildCommandKillProcesses,	BUILDCMDTYPE_KILL_PROCESSES);
MAKE_COMMAND(tpiBuildCommandWaitForProcs,	BUILDCMDTYPE_WAIT_FOR_PROCS);
MAKE_COMMAND(tpiBuildCommandDeleteFiles,	BUILDCMDTYPE_DELETE_FILES);
MAKE_COMMAND(tpiBuildCommandCopy,			BUILDCMDTYPE_COPY);
MAKE_COMMAND(tpiBuildCommandRename,			BUILDCMDTYPE_RENAME);
MAKE_COMMAND(tpiBuildCommandGimme,			BUILDCMDTYPE_GIMME);
MAKE_COMMAND(tpiBuildCommandDeleteFolder,	BUILDCMDTYPE_DELETE_FOLDER);
MAKE_COMMAND(tpiBuildCommandMakeFolder,		BUILDCMDTYPE_MAKE_FOLDER);
MAKE_COMMAND(tpiBuildCommandSVN,			BUILDCMDTYPE_SVN);
MAKE_COMMAND(tpiBuildCommandTool,			BUILDCMDTYPE_TOOL);
MAKE_COMMAND(tpiBuildCommandCompile,		BUILDCMDTYPE_COMPILE);
MAKE_COMMAND(tpiBuildCommandBuild,			BUILDCMDTYPE_BUILD);
MAKE_COMMAND(tpiBuildCommandSrcExe,			BUILDCMDTYPE_SRCEXE);
MAKE_COMMAND(tpiBuildCommandFunction,		BUILDCMDTYPE_FUNCTION);
MAKE_COMMAND(tpiBuildCommandPrint,			BUILDCMDTYPE_PRINT);
MAKE_COMMAND(tpiBuildCommandError,			BUILDCMDTYPE_ERROR);
MAKE_COMMAND(tpiBuildCommandSleep,			BUILDCMDTYPE_SLEEP);
MAKE_COMMAND(tpiBuildProcCommandKill,		BUILDCMDTYPE_PROC_KILL);
MAKE_COMMAND(tpiBuildListCommandSort,		BUILDCMDTYPE_LIST_SORT);
MAKE_COMMAND(tpiBuildVarCommandAdd,			BUILDCMDTYPE_VAR_ADD);
MAKE_COMMAND(tpiBuildVarCommandSub,			BUILDCMDTYPE_VAR_SUB);
MAKE_COMMAND(tpiBuildVarCommandMul,			BUILDCMDTYPE_VAR_MUL);
MAKE_COMMAND(tpiBuildVarCommandDiv,			BUILDCMDTYPE_VAR_DIV);




MAKE_COMMAND_BEGIN(tpiBuildCommandWriteLogFile, BUILDCMDTYPE_WRITELOGFILE)
	STANDARD_START_END,
	{"FileName:",		TOK_STRING(BuildCommand, writeLogFile.fileName, 0)},
	{"Text:",			TOK_STRINGARRAY(BuildCommand, writeLogFile.textToWrite)},
MAKE_COMMAND_END();

#undef MAKE_COMMAND

		///* Configuration */																					\
		//{"Configuration",		TOK_STRUCT,		STRUCT_INFO(BuildStep, configurations, tpiBuildConfiguration)},	\

#define MAKE_BUILDSTEP_BEGIN(name, isElseInfo)															\
	ParseTable name[] = {																		\
		STANDARD_START_END,																				\
		{"",						TOK_INT(BuildStep, isElse,				isElseInfo)},				\
		/* Internal Variables. */																		\
		{"DisplayName:",			TOK_REDUNDANTNAME|TOK_STRUCTPARAM|TOK_STRING(BuildStep, displayName, 0)},								\
		{"DisplayName:",			TOK_STRING(BuildStep, displayName, 0)},								\
		{"ContinueOnError:",		TOK_BOOL(BuildStep, continueOnError,		0)},					\
		{"Simulate:",				TOK_BOOL(BuildStep, simulate,		0)},							\
		{"ForceRun:",				TOK_BOOL(BuildStep, forceRun,		0)},							\
		/* Variables. */																				\
		{"InputVar",				TOK_STRUCT(BuildStep, vars, tpiBuildInputVar)},						\
		{"Var:",					TOK_RSTRUCT(BuildStep, vars, tpiBuildVar)},							\
		{"List:",					TOK_RSTRUCT(BuildStep, lists, tpiBuildList)},						\
		/* Conditions. */																				\
		{"Condition",				TOK_STRUCT(BuildStep, conditions, tpiBuildCondition)},				\
		{"Condition:",				TOK_RSTRUCT(BuildStep, conditions, tpiBuildConditionIfLine)},		\
		{"Requirement",				TOK_STRUCT(BuildStep, requirements, tpiBuildCondition)},			\
		{"Requirement:",			TOK_RSTRUCT(BuildStep, requirements, tpiBuildConditionIfLine)},		\
		{"For",						TOK_STRINGARRAY(BuildStep, forLoopValues)},							\
		/* Commands. */																					\
		{"Cmd_System:",				TOK_RSTRUCT(BuildStep, commands, tpiBuildCommandSystem)},			\
		{"Cmd_Run:",				TOK_RSTRUCT(BuildStep, commands, tpiBuildCommandRun)},			\
		{"Cmd_PushDir:",			TOK_RSTRUCT(BuildStep, commands, tpiBuildCommandPushDir)},			\
		{"Cmd_PopDir",				TOK_RSTRUCT(BuildStep, commands, tpiBuildCommandPopDir)},			\
		{"Cmd_StoreCmd:",			TOK_STRUCT(BuildStep, commands, tpiBuildCommandStoreCmd)},			\
		{"Cmd_KillProcesses:",		TOK_RSTRUCT(BuildStep, commands, tpiBuildCommandKillProcesses)},	\
		{"Cmd_WaitForProcesses:",	TOK_RSTRUCT(BuildStep, commands, tpiBuildCommandWaitForProcs)},		\
		{"Cmd_DeleteFiles:",		TOK_RSTRUCT(BuildStep, commands, tpiBuildCommandDeleteFiles)},		\
		{"Cmd_Copy:",				TOK_RSTRUCT(BuildStep, commands, tpiBuildCommandCopy)},				\
		{"Cmd_Rename:",				TOK_RSTRUCT(BuildStep, commands, tpiBuildCommandRename)},			\
		{"Cmd_Gimme:",				TOK_RSTRUCT(BuildStep, commands, tpiBuildCommandGimme)},			\
		{"Cmd_DeleteFolder:",		TOK_RSTRUCT(BuildStep, commands, tpiBuildCommandDeleteFolder)},		\
		{"Cmd_MakeFolder:",			TOK_RSTRUCT(BuildStep, commands, tpiBuildCommandMakeFolder)},		\
		{"Cmd_SVN:",				TOK_RSTRUCT(BuildStep, commands, tpiBuildCommandSVN)},				\
		{"Cmd_Tool:",				TOK_RSTRUCT(BuildStep, commands, tpiBuildCommandTool)},				\
		{"Cmd_Compile:",			TOK_RSTRUCT(BuildStep, commands, tpiBuildCommandCompile)},			\
		{"Cmd_Build:",				TOK_RSTRUCT(BuildStep, commands, tpiBuildCommandBuild)},			\
		{"Cmd_SrcExe:",				TOK_RSTRUCT(BuildStep, commands, tpiBuildCommandSrcExe)},			\
		{"Cmd_WriteLogFile:",		TOK_RSTRUCT(BuildStep, commands, tpiBuildCommandWriteLogFile)},		\
		{"Cmd_Function:",			TOK_RSTRUCT(BuildStep, commands, tpiBuildCommandFunction)},			\
		/* Misc Commands */																				\
		{"Print:",					TOK_RSTRUCT(BuildStep, commands, tpiBuildCommandPrint)},			\
		{"Error:",					TOK_RSTRUCT(BuildStep, commands, tpiBuildCommandError)},			\
		{"Sleep:",					TOK_RSTRUCT(BuildStep, commands, tpiBuildCommandSleep)},			\
		/* List  and Variable Commands */																\
		{"List_Sort:",				TOK_RSTRUCT(BuildStep, commands, tpiBuildListCommandSort)},			\
		{"Var_Add:",				TOK_RSTRUCT(BuildStep, commands, tpiBuildVarCommandAdd)},			\
		{"Var_Sub:",				TOK_RSTRUCT(BuildStep, commands, tpiBuildVarCommandSub)},			\
		{"Var_Mul:",				TOK_RSTRUCT(BuildStep, commands, tpiBuildVarCommandMul)},			\
		{"Var_Div:",				TOK_RSTRUCT(BuildStep, commands, tpiBuildVarCommandDiv)},			\
		/* Process Commands */																			\
		{"Proc_Kill:",				TOK_RSTRUCT(BuildStep, commands, tpiBuildProcCommandKill)},			\
		/* BuildSteps */																				\
		{"BuildStep",				TOK_STRUCT(BuildStep, buildSteps, tpiBuildStep)},					\
		{"BuildStepIf:",			TOK_RSTRUCT(BuildStep, buildSteps, tpiBuildStepIf)},				\
		{"ElseBuildStep",			TOK_RSTRUCT(BuildStep, buildSteps, tpiElseBuildStep)},

#define MAKE_BUILDSTEP_END()																			\
		STANDARD_TERMINATOR																				\
	}
	
#define MAKE_BUILDSTEP(name, isElseInfo) MAKE_BUILDSTEP_BEGIN(name, isElseInfo) MAKE_BUILDSTEP_END();
	

extern ParseTable tpiBuildStepIf[];
extern ParseTable tpiElseBuildStep[];

MAKE_BUILDSTEP(tpiBuildStep, 0);
MAKE_BUILDSTEP(tpiElseBuildStep, 1);

MAKE_BUILDSTEP_BEGIN(tpiBuildStepIf, 0)
	{"",					TOK_STRUCTPARAM | TOK_RSTRUCT(BuildStep, conditions, tpiBuildConditionIfLine)},
MAKE_BUILDSTEP_END();

#undef MAKE_BUILDSTEP_BEGIN
#undef MAKE_BUILDSTEP_END
#undef MAKE_BUILDSTEP

static ParseTable tpiBuildScript[] = {
	{"BuildStep",			TOK_STRUCT(BuildScript, buildSteps, tpiBuildStep) },
	{"ElseBuildStep",		TOK_RSTRUCT(BuildScript, buildSteps, tpiElseBuildStep)},
	STANDARD_TERMINATOR
};


BuildState *g_BuildState = NULL;
BuildScript *g_BuildScript;

static char buildScriptDir[MAX_PATH] = "c:/game/tools/build";
static char buildScriptPath[MAX_PATH] = "./build_script.buildscript";
static char buildConfigPath[MAX_PATH] = {0};
static char profilePath[MAX_PATH] = "logs/profile.txt";
char log_file_path[MAX_PATH+1] = {0};
FILE *builderizer_log_file = NULL;

static U32 timer;
static U32 *timerStack = NULL;
static S32 curTimer = 0, nextTimer = 0;
static bool waitOnExit = 0;
static bool disable_logging = 0;
static bool server_mode = 1;
static bool client_mode = 0;


// -------------------------------------------------------
// build error level functions
//
// the error level is set through different threads, so 
// all changes to the error level must go through these
// functions.
// -------------------------------------------------------

// this is to prevent unsafe changes to the error level
static int lastErrorLevel = 0;

void setErrorLevel(int level)
{
	buildEnterCritSection();
	if ( lastErrorLevel != g_BuildState->errorLevel )
		assert("Error Level being changed outside of thread safe functions" == 0);
	g_BuildState->errorLevel = level;
	lastErrorLevel = g_BuildState->errorLevel;
	buildLeaveCritSection();
}

void incErrorLevel()
{
	buildEnterCritSection();
	if ( lastErrorLevel != g_BuildState->errorLevel )
		assert("Error Level being changed outside of thread safe functions" == 0);
	g_BuildState->errorLevel++;
	lastErrorLevel = g_BuildState->errorLevel;
	buildLeaveCritSection();
}

void decErrorLevel()
{
	buildEnterCritSection();
	if ( lastErrorLevel != g_BuildState->errorLevel )
		assert("Error Level being changed outside of thread safe functions" == 0);
	g_BuildState->errorLevel--;
	lastErrorLevel = g_BuildState->errorLevel;
	buildLeaveCritSection();
}

int getErrorLevel()
{
	buildEnterCritSection();
	if ( lastErrorLevel != g_BuildState->errorLevel )
		assert("Error Level being changed outside of thread safe functions" == 0);
	lastErrorLevel = g_BuildState->errorLevel;
	buildLeaveCritSection();
	return lastErrorLevel;
}

// -------------------------------------------------------
// END build error level functions
// -------------------------------------------------------


U32 getNewTimer()
{
	if ( !timerStack )
		ea32Create(&timerStack);

	curTimer = nextTimer;

	if ( curTimer >= ea32Size(&timerStack) )
	{
		int newTimer = timerAlloc();
		ea32Push(&timerStack, newTimer);
	}

	++nextTimer;
	return timerStack[curTimer];
}

U32 getLastTimer()
{
	if ( !timerStack || ea32Size(&timerStack) <= 0 )
		return getNewTimer();

	--nextTimer;
	--curTimer;

	if ( curTimer < 0 )
		curTimer = 0;
	if ( nextTimer < 0 )
		nextTimer = 0;

	return timerStack[curTimer];
}


BuildState *getBuildState(void)
{
	return g_BuildState;
}

char *getDisplayName( BuildStep *step )
{
	//static char *newDisplayName = NULL;
	//char *tmp = NULL;

	if ( !step->displayName )
		return "(null)";

	//tmp = doVariableReplacement(step->displayName);
	//estrClear(&newDisplayName);
	//estrCopy(&newDisplayName, &tmp);
	//estrDestroy(&tmp);
	return step->displayName;
}


const char* getCommandTypeName( BuildCommandType cmdType )
{
	switch(cmdType){
		#define CASE(x, y) xcase x: return y
		CASE(BUILDCMDTYPE_SYSTEM,			"System");
		CASE(BUILDCMDTYPE_RUN,				"Run");
		CASE(BUILDCMDTYPE_PUSHDIR,			"PushDir");
		CASE(BUILDCMDTYPE_POPDIR,			"PopDir");
		CASE(BUILDCMDTYPE_KILL_PROCESSES,	"KillProcesses");
		CASE(BUILDCMDTYPE_WAIT_FOR_PROCS,	"WaitForProcesses");
		CASE(BUILDCMDTYPE_DELETE_FILES,		"DeleteFiles");
		CASE(BUILDCMDTYPE_COPY,				"Copy");
		CASE(BUILDCMDTYPE_RENAME,			"Rename");
		CASE(BUILDCMDTYPE_GIMME,			"Gimme");
		CASE(BUILDCMDTYPE_DELETE_FOLDER,	"DeleteFolder");
		CASE(BUILDCMDTYPE_MAKE_FOLDER,		"MakeFolder");
		CASE(BUILDCMDTYPE_SVN,				"SVN");
		CASE(BUILDCMDTYPE_TOOL,				"Tool");
		CASE(BUILDCMDTYPE_COMPILE,			"Compile");
		CASE(BUILDCMDTYPE_BUILD,			"Build");
		CASE(BUILDCMDTYPE_SRCEXE,			"SrcExe");
		CASE(BUILDCMDTYPE_FUNCTION,			"Function");
		CASE(BUILDCMDTYPE_PRINT,			"Print");
		CASE(BUILDCMDTYPE_ERROR,			"Error");
		CASE(BUILDCMDTYPE_SLEEP,			"Sleep");
		CASE(BUILDCMDTYPE_WRITELOGFILE,		"WriteLogFile");
		CASE(BUILDCMDTYPE_PROC_KILL,		"ProcKill");
		CASE(BUILDCMDTYPE_LIST_SORT,		"ListSort");
		CASE(BUILDCMDTYPE_VAR_ADD,			"VarAdd");
		CASE(BUILDCMDTYPE_VAR_SUB,			"VarSub");
		CASE(BUILDCMDTYPE_VAR_MUL,			"VarMul");
		CASE(BUILDCMDTYPE_VAR_DIV,			"VarDiv");
		xdefault:{
			assert(0);
		}
		#undef CASE
	}

	return NULL;
}

S32 isValidBuildVarName( const char *name )
{
	for(; name && *name; name++){
		if(	!(	tolower(*name) >= 'a' &&
				tolower(*name) <= 'z'
				||
				*name == '_'
				||
				*name >= '0' &&
				*name <= '9')
			)
		{
			return 0;
		}
	}
	
	return 1;
}

S32 setBuildStateVar( const char *name, const char *value )
{
	char* tempValue;
	
	if(!isValidBuildVarName(name))
	{
		return 0;
	}
	
	if(!g_BuildState->stVariables)
	{
		g_BuildState->stVariables = stashTableCreateWithStringKeys(100, 0);
	}
	
	if(!g_BuildState->stringTableVarNames)
	{
		g_BuildState->stringTableVarNames = strTableCreate(StrTableDefault, 128);
	}
	
	name = strTableAddString(g_BuildState->stringTableVarNames, name);
	
	if(stashFindPointer(g_BuildState->stVariables, name, &tempValue))
	{
		SAFE_FREE(tempValue);
	}
	
	tempValue = value ? strdup(value) : NULL;
	
	if(!stashAddPointer(g_BuildState->stVariables, name, tempValue, true))
	{
		assert(0);
	}
	
	return 1;
}


typedef struct
{
	char **values;
} StoredStateList;

int isBuildStateList( const char *name )
{
	StoredStateList *temp;

	if(!isValidBuildVarName(name) || !g_BuildState->stLists || !g_BuildState->stringTableVarNames)
		return 0;

	if(!stashFindPointer(g_BuildState->stLists, name, &temp))
		return 0;

	return 1;
}

int clearBuildStateListInternal(StashElement element)
{
	eaSetSize(&((StoredStateList*)stashElementGetPointer(element))->values, 0);
	return 1;
}

void clearAllBuildStateLists()
{
	stashForEachElement(g_BuildState->stLists, clearBuildStateListInternal);
}

S32 clearBuildStateList( const char *name )
{
	StoredStateList *temp;
	
	if(!isValidBuildVarName(name))
	{
		return 0;
	}
	
	if(!g_BuildState->stLists)
	{
		g_BuildState->stLists = stashTableCreateWithStringKeys(100, 0);
	}
	
	if(!g_BuildState->stringTableVarNames)
	{
		g_BuildState->stringTableVarNames = strTableCreate(StrTableDefault, 128);
	}

	name = strTableAddString(g_BuildState->stringTableVarNames, name);

	if(!stashFindPointer(g_BuildState->stLists, name, &temp))
	{
		temp = (StoredStateList*)calloc(1, sizeof(StoredStateList));

		if(!stashAddPointer(g_BuildState->stLists, name, temp, true))
		{
			assert(0);
		}
	}

	eaSetSize(&temp->values, 0);
	return 1;
}

S32 addToBuildStateList( const char *name, const char *value )
{
	StoredStateList *temp;
	
	if(!isValidBuildVarName(name))
	{
		return 0;
	}
	
	if(!g_BuildState->stLists)
	{
		g_BuildState->stLists = stashTableCreateWithStringKeys(100, 0);
	}
	
	if(!g_BuildState->stringTableVarNames)
	{
		g_BuildState->stringTableVarNames = strTableCreate(StrTableDefault, 128);
	}
	
	name = strTableAddString(g_BuildState->stringTableVarNames, name);
	
	if(!stashFindPointer(g_BuildState->stLists, name, (void**)&temp))
	{
		temp = (StoredStateList*)calloc(1, sizeof(StoredStateList));

		if(!stashAddPointer(g_BuildState->stLists, name, temp, true))
		{
			assert(0);
		}
	}
	
	if ( value )
	{
		eaPush(&temp->values,strdup(value));
	}
	
	return 1;
}


void builderizerLogv(const char *format, va_list va)
{
	if ( disable_logging )
		return;

	if (!builderizer_log_file)
	{
		// no log file specified
		if ( log_file_path[0] == 0 )
			return;
		makeDirectoriesForFile(log_file_path);
		builderizer_log_file = fopen(log_file_path, "wt");
		if ( !builderizer_log_file )
			return;
	}
	vfprintf(builderizer_log_file, format, va);
}

void builderizerLogf(char *format, ...)
{
	VA_START(va, format);
	builderizerLogv(format, va);
	VA_END();
}


void printIndent( S32 indent )
{
	S32 i;
	
	for(i = 0; i < indent; i++){
		printf("    ");
		if ( !g_BuildState->flags.disableLogging )
			builderizerLogf("    ");
	}
}

void printStateIndentPlus( S32 extraIndent )
{
	printIndent(g_BuildState->stepDepth + extraIndent);
}

void printStateIndent(void){
	printStateIndentPlus( 0);
}

void statePrintv( S32 extraIndent, U32 color, const char *format, va_list va)
{
	if(color & ~COLOR_BRIGHT){
		consoleSetColor(color, 0);
	}
	
	printStateIndentPlus(extraIndent);
	
	if(format){
		vprintf(format, va);
		if ( !g_BuildState->flags.disableLogging )
			builderizerLogv(format, va);
	}
	
	if(color & ~COLOR_BRIGHT){
		consoleSetDefaultColor();
	}
}

void statePrintf( S32 extraIndent, U32 color, const char* format, ... )
{
	VA_START(va, format);
	statePrintv( extraIndent, color | COLOR_BRIGHT, format, va);
	VA_END();
}

void statePrintfDim( S32 extraIndent, U32 color, const char* format, ... )
{
	VA_START(va, format);
	statePrintv( extraIndent, color & ~COLOR_BRIGHT, format, va);
	VA_END();
}

void statePrintfNoLog( S32 extraIndent, U32 color, const char* format, ... )
{
	g_BuildState->flags.disableLogging = 1;
	VA_START(va, format);
	statePrintv( extraIndent, color | COLOR_BRIGHT, format, va);
	VA_END();
	g_BuildState->flags.disableLogging = 0;
}

void statePrintfDimNoLog( S32 extraIndent, U32 color, const char* format, ... )
{
	g_BuildState->flags.disableLogging = 1;
	VA_START(va, format);
	statePrintv( extraIndent, color & ~COLOR_BRIGHT, format, va);
	VA_END();
	g_BuildState->flags.disableLogging = 0;
}

void builderizerError( const char *format, ... )
{
	char *newFormat = NULL;
	estrConcat(&newFormat, "ERROR: ", (int)strlen("ERROR: "));
	estrConcat(&newFormat, format, (int)strlen(format));

	VA_START(va, format);
	statePrintv( 1, COLOR_RED | COLOR_BRIGHT, newFormat, va );
	VA_END();

	estrDestroy(&newFormat);
	
	incErrorLevel();
	if ( g_BuildState->curBuildStep )
	{
		buildStatusUpdateStep(getDisplayName(g_BuildState->curBuildStep), g_BuildState->curBuildStep->timeSpent, 0, 1);
		if ( !g_BuildState->curBuildStep->continueOnError )
			buildStatusUpdateStatus(1);
	}
	if ( builderizer_log_file )
		fflush(builderizer_log_file);
}


void clearAllErrorsRecurse(BuildStep **steps)
{
	int i;
	for ( i = 0; i < eaSize(&steps); ++i )
	{
		if ( steps[i]->buildSteps )
			clearAllErrorsRecurse(steps[i]->buildSteps);
		steps[i]->failed = 0;
	}
}

void clearAllErrors()
{
	setErrorLevel(0);
	clearAllErrorsRecurse(g_BuildScript->buildSteps);
}

int recoverFromError()
{
	int i;

	if ( getErrorLevel() == 0 )
		return 1;

	setErrorLevel(0);

	// find the step with the error and run all force-run steps before it
	for ( i = 0; i < eaSize(&g_BuildScript->buildSteps); ++i )
	{
		if ( g_BuildScript->buildSteps[i]->forceRun )
			runBuildStep(g_BuildScript->buildSteps[i]);
		else if ( g_BuildScript->buildSteps[i]->failed )
		{
			g_BuildScript->buildSteps[i]->failed = 0;
			if ( g_BuildScript->buildSteps[i]->buildSteps )
				clearAllErrorsRecurse( g_BuildScript->buildSteps[i]->buildSteps );
			g_BuildState->curBuildStep = g_BuildScript->buildSteps[i];
			g_BuildState->curBuildStepIdx = i;
			break;
		}
	}

	return 1;
}


const char* getBuildStateVar( const char *name )
{
	const char* value;
	
	if(!g_BuildState->stVariables){
		return NULL;
	}
	
	if(stashFindPointerConst(g_BuildState->stVariables, name, &value)){
		return value;
	}
	
	return NULL;
}

char** getBuildStateList( const char *name )
{
	StoredStateList *list;
	
	if(!g_BuildState->stLists){
		return NULL;
	}
	
	if(stashFindPointerConst(g_BuildState->stLists, name, &list)){
		return list->values;
	}
	
	return NULL;
}


char *escapeVar(char *v)
{
	char *orig = v;
	while ( v && *v )
	{
		if ( *v == '\\' )
		{
			++v;
			switch (*v)
			{
			case '\\':
			case '-':
				memcpy(v-1, v, strlen(v)+1); //+1 to include NULL
				break;
			default:
				builderizerError("Unknown escape character %c in %s", *v, orig);
				return orig;
			}
		}
		else
			++v;
	}
	return orig;
}


//
// Function: doVariableReplacement
//
// Returns: An EString, so called must call estrDestroy(&retval) on it.
//
// This basically treats surrounding '%' as a dereference.  If a value has no valid
// dereference (such as a list without an operator) it will fail.
//
char* doVariableReplacementEx( const char *input, int *isList )
{
	const char* originalInput = input;
	char* output;
	int varIsList = 0;
	
	estrCreate(&output);
	
	for(; input && *input; input++)
	{
		if(*input == '%')
		{
			char token[1000];
			char* tokenEnd = token;
			int numClosedBrackets = 0;
			int numClosedParens = 0;
			int numClosedBraces = 0;
			int charCount = 0;
			
			for(input++; *input && (*input != '%' || numClosedBrackets || numClosedParens || numClosedBraces ); input++)
			{
				if ( *input == '[' )
					numClosedBrackets++;
				else if ( *input == '(' )
					numClosedParens++;
				else if ( *input == '{' )
					numClosedBraces++;
				else if ( *input == ']' )
					numClosedBrackets--;
				else if ( *input == ')' )
					numClosedParens--;
				else if ( *input == '}' )
					numClosedBraces--;

				if ( numClosedBrackets < 0 || numClosedParens < 0 || numClosedBraces < 0 )
				{
					builderizerError("Invalid number of brackets or parenthesis in %s", originalInput);
					return NULL;
				}

				*tokenEnd++ = *input;
				++charCount;
			}
			
			*tokenEnd = 0;
			
			if(*input)
			{
				// There is a token that ended with "%".

				--tokenEnd;

				// there were 0 characters in between the first % and the last %.
				// this means that they are trying to just output a single %
				if ( charCount == 0 )
				{
					estrConcatChar(&output, '%');
				}
				// this token has a parenthesized value at the end of it, which represents
				// how many characters from the beginning or end of the string to use
				// eg: (2) would get the first 2 characters of the value,
				// (-2) would get the last 2 characters of the value
				//
				// on the string 'abcde', (2) would get 'ab' and (-2) would get 'de'
				//
				// since it is difficult to recover from script errors, instead of erroring if the value is
				// too large, i just return the entire string
				else if ( *tokenEnd == ')' )
				{
					int numToGet = 0;
					char *tmp;
					// find the bracketed value
					*tokenEnd = 0;
					while ( *tokenEnd != '(' ) --tokenEnd;
					*tokenEnd = 0;
					tokenEnd++;
					tmp = doVariableReplacement(tokenEnd);

					{
						const char* value = getBuildStateVar( token);
						if ( value )
						{
							int len = (int)strlen(value);

							numToGet = atoi(tmp);
							if ( abs(numToGet) > len )
								estrAppend2(&output, value);
							else
							{
								if ( numToGet < 0 )
									estrAppend2(&output, &value[len + numToGet]);
								else
									estrConcat(&output, value, numToGet);
							}
							
						}
						else
						{
							const char **values = getBuildStateList(token);
							if ( values )
							{
								varIsList = 1;
								builderizerError( "No operator () defined for list: \"%s\" used in \"%s\"\n", token, originalInput);
							}
							else
								builderizerError( "Undefined variable \"%s\" used in \"%s\"\n", token, originalInput);
						}
					}
				}
				// this token has a bracketed value at the end of it, which represents
				// which character from the beginning or end of the string to use
				// eg: [2] would get the character at index 2 (the third character) from the start of the value,
				// [-2] would get the 2nd to last characters of the value
				// 
				// on the string 'abcde', [2] would get 'c' and [-2] would get 'd'
				//
				// since it is difficult to recover from script errors, insted of erroring if the value is
				// too large or small, i use a mod system, so in the above string, [-6] would become [-1]
				// and [6] would become [1]
				//
				// for a list, it returns the value at the given index
				else if (*tokenEnd == ']')
				{
					int idxToGet = 0;
					char *tmp;
					// find the bracketed value
					*tokenEnd = 0;
					while ( *tokenEnd != '[' ) --tokenEnd;
					*tokenEnd = 0;
					tokenEnd++;
					tmp = doVariableReplacement(tokenEnd);
					idxToGet = atoi(tmp); 
					
					{
						const char* value = getBuildStateVar( token);

						if ( value )
						{
							int len = (int)strlen(value);
							while ( idxToGet < 0 )
								idxToGet += len;
							while ( idxToGet >= len )
								idxToGet -= len;
							estrConcatChar(&output, value[idxToGet]);
						}
						else
						{
							const char **values = getBuildStateList(token);
							if ( values )
							{
								int len = eaSize(&values);
								while ( idxToGet < 0 )
									idxToGet += len;
								while ( idxToGet >= len )
									idxToGet -= len;
								estrAppend2(&output, values[idxToGet]);
								varIsList = 1;
							}
							else
							{
								builderizerError( "Undefined variable \"%s\" used in \"%s\"\n", token, originalInput);
							}
						}
					}
				}
				// braces handle finding the string up to the first occurance of a character or string
				// eg: {.} would return the string up until the first '.', 
				// {-.} returns the end of the string past the last '.'
				//
				// '\' is an escape character, so to search for the first '-', you would use {\-}, 
				// and similarly, to search for the last '-', you would use {-\-}. 
				// to search for a '\', you would use {\\}
				else if ( *tokenEnd == '}' )
				{
					int numCharsToGet = 0;
					char *tmp;
					// find the bracketed value
					*tokenEnd = 0;
					while ( *tokenEnd != '{' ) --tokenEnd;
					*tokenEnd = 0;
					tokenEnd++;
					tmp = doVariableReplacement(tokenEnd);

					{
						const char* value = getBuildStateVar( token);
						if ( value )
						{
							int fromEnd = (*tmp == '-' ? 1 : 0);
							if ( fromEnd ) ++tmp;
							escapeVar(tmp);
							if ( fromEnd )
							{
								char *newValue = estrCreateFromStr( value );
								char *lastMatch = strstri(newValue, tmp);
								char *c = strstri((lastMatch+1), tmp);
								while ( c )
								{
									lastMatch = c;
									c = strstr((lastMatch+1), tmp);
								}
								lastMatch += strlen(tmp);
								estrAppend2(&output,lastMatch);
								estrDestroy(&newValue);
							}
							else
							{
								char *newValue = estrCreateFromStr( value );
								char *c = strstr(newValue, tmp);
								if ( c ) *c = 0;
								estrAppend2(&output,newValue);
								estrDestroy(&newValue);
							}
						}
						else
						{
							const char **values = getBuildStateList(token);
							if ( values )
							{
								builderizerError( "No operator {} defined for list: \"%s\" used in \"%s\"\n", token, originalInput);
								varIsList = 1;
							}
							else
								builderizerError( "Undefined variable \"%s\" used in \"%s\"\n", token, originalInput);
						}
					}
				}
				// a question mark returns the length of the variable.
				// if the variable is a list, it returns the length of the list
				else if ( *tokenEnd == '?' )
				{
					*tokenEnd = 0;
					{
						const char* value = getBuildStateVar( token);
						if ( value )
						{
							estrConcatf(&output, "%d", strlen(value));
						}
						else
						{
							const char **values = getBuildStateList(token);
							if ( values )
							{
								estrConcatf(&output, "%d", eaSize(&values));
								varIsList = 1;
							}
							else
								builderizerError( "Undefined variable \"%s\" used in \"%s\"\n", token, originalInput);
						}
					}
				}
				else
				{
					const char* value = getBuildStateVar( token);
					if ( value )
						estrAppend2(&output, value);
					else
					{
						const char **values = getBuildStateList(token);
						if ( values )
						{
							int i;

							// in this context, a list returns its size
							//estrConcatf(&output, "%d", eaSize(&values));
							for ( i = 0; i < eaSize(&values)-1; ++i )
							{
								estrConcatf(&output, "%s ", doVariableReplacement(values[i]));
							}
							estrConcatf(&output, "%s", doVariableReplacement(values[i]));
							varIsList = 1;

						}
						else
							builderizerError( "Undefined variable \"%s\" used in \"%s\"\n", token, originalInput);
					}
				}
			}
		}
		else
		{
			char str[2] = {*input, 0};
			
			estrAppend2(&output, str);
		}
	}
	
	if ( isList )
		*isList = varIsList;
	return output;
}

void initBuildScript()
{
	if ( g_BuildScript )
		StructDestroy(tpiBuildScript, g_BuildScript);
	ParserSetTableInfo(tpiBuildScript, sizeof(*g_BuildScript), "BuildScript", NULL, __FILE__);
	g_BuildScript = StructAllocRaw(tpiBuildScript);	
	ZeroStruct(g_BuildScript);
}

void readScriptFiles( char *scriptPath )
{
	initBuildScript();
	g_BuildState->curBuildStepIdx = 0;
	if ( !ParserLoadFiles(NULL, scriptPath, NULL, 0, tpiBuildScript, g_BuildScript) )
		Errorf("Error loading buildscript file %s", scriptPath);
}

void parseCommandLine( S32 argc, char **argv )
{
	S32 i;

	if ( g_BuildState )
	{
		g_BuildState->flags.doRunCommands = 1;
		//g_BuildState->flags.doPrintConditions = 1;
		//g_BuildState->flags.doApproveEachStep = 1;
		//g_BuildState->flags.doPrintExternalCommands = 1;
		g_BuildState->flags.profile = 0;
	}
	
	for(i = 0; i < argc; i++){
		const char* cmd = argv[i];
		
		if(cmd[0] != '-'){
			continue;
		}
		
		cmd++;
		
		#define BEGIN_CMDS() if(0);
		#define IF_CMD(x) else if(!_stricmp(cmd, x))
		#define HAS_PARAM() (i + 1 < argc && argv[i + 1][0] != '-')
		#define GET_PARAM() (HAS_PARAM() ? argv[++i] : "")
		#define END_CMDS()
		
		BEGIN_CMDS()
		IF_CMD("build") {
			Strncpyt(buildScriptDir, GET_PARAM());
			
			printf("Overriding build dir: %s\n", buildScriptDir);
		}
		IF_CMD("script") {
			Strncpyt(buildScriptPath, GET_PARAM());
			printf("Using script: %s\n", buildScriptPath);
			client_mode = server_mode = 0;
		}
		IF_CMD("log") {
			char *logPath = GET_PARAM();
			if ( strlen(logPath) >= MAX_PATH )
				printf("The value specified for -log (%s) is too long. Using default...\n", logPath);
			else
				Strncpyt(log_file_path, logPath);
		}
		IF_CMD("dontruncommands") {
			g_BuildState->flags.doRunCommands = 0;
		}
		IF_CMD("printexternalcommands") {
			g_BuildState->flags.doPrintExternalCommands = 1;
		}
		IF_CMD("waitonexit") {
			waitOnExit = 1;
		}
		IF_CMD("server") {
			char *ipstr = GET_PARAM();
			setBuildServerIP(ipstr);
			client_mode = 1;
		}
		IF_CMD("var") {
			char *varName = GET_PARAM();
			char *varVal = GET_PARAM();
			setBuildStateVar(varName, varVal);
		}
		IF_CMD("console") {
			newConsoleWindow();
			consoleInit(140, 9999);
		}
		IF_CMD("config") {
			strcpy_s(buildConfigPath, MAX_PATH, GET_PARAM());
		}
		IF_CMD("profile") {
			g_BuildState->flags.profile = 1;
		}
		else if (!stricmp(cmd, "nologs") || !stricmp(cmd, "disablelogs") || !stricmp(cmd, "disablelogging")){
			disable_logging = 1;
		}
		END_CMDS()
	}
}

S32 conditionIsBlock( BuildConditionIfType ifType )
{
	switch(ifType){
		xcase IFTYPE_IF_BLOCK:
		case IFTYPE_AND_BLOCK:
		case IFTYPE_OR_BLOCK:
		{
			return 1;
		}
	}
	
	return 0;
}

const char* getIfTypeToken( BuildConditionIfType ifType )
{
	switch(ifType){
		xcase IFTYPE_IF_BLOCK:
			return "if:";
		xcase IFTYPE_IF_LINE:
			return "if:";
		xcase IFTYPE_AND_BLOCK:
			return "&&";
		xcase IFTYPE_AND_LINE:
			return "&&:";
		xcase IFTYPE_OR_BLOCK:
			return "||";
		xcase IFTYPE_OR_LINE:
			return "||:";
		xdefault:
			return "ERROR!";
	}
}

#if 0
void printCondition( BuildCondition *cond, S32 indent )
{
	S32 i;
	
	if(conditionIsBlock(cond->ifType)){
		printIndent(indent);

		switch(cond->ifType){
			xcase IFTYPE_IF_BLOCK:
				printfColor(COLOR_RED|COLOR_BLUE, "if\n");
			xcase IFTYPE_AND_BLOCK:
				printfColor(COLOR_RED|COLOR_BLUE, "&&\n");
			xcase IFTYPE_OR_BLOCK:
				printfColor(COLOR_RED|COLOR_BLUE, "||\n");
		}
		
		printIndent(indent);
		printfColor(COLOR_RED|COLOR_BLUE, "{\n");
		
		for(i = 0; i < eaSize(&cond->conditions); i++){
			printCondition(cond->conditions[i], indent + 1);
		}
		
		printIndent(indent);
		printfColor(COLOR_RED|COLOR_BLUE, "}\n");
	}else{
		printIndent(indent);
		
		switch(cond->ifType){
			xcase IFTYPE_IF_LINE:
				printfColor(COLOR_RED|COLOR_BLUE, "if:");
			xcase IFTYPE_AND_LINE:
				printfColor(COLOR_RED|COLOR_BLUE, "&&:");
			xcase IFTYPE_OR_LINE:
				printfColor(COLOR_RED|COLOR_BLUE, "||:");
		}
		
		for(i = 0; i < eaSize(&cond->params); i++){
			printfColor((i & 1 ? COLOR_BRIGHT : 0) | COLOR_GREEN | COLOR_BLUE, " %s", cond->params[i]);
		}
		
		printf("\n");
	}
}

void printBuildSteps( BuildStep *step, S32 indent )
{
	S32 i;
	S32 j;
	
	printIndent(indent);
	
	printfColor(COLOR_GREEN, "- ");

	printfColor(step->displayName ? COLOR_BRIGHT|COLOR_GREEN : COLOR_BRIGHT|COLOR_RED,
				"%s%s:\n",
				step->isElse ? "ELSE: " : "",
				step->displayName ? step->displayName : "(NO NAME)");
				
	for(i = 0; i < eaSize(&step->conditions); i++){
		printCondition(step->conditions[i], indent + 1);
	}

	for(i = 0; i < eaSize(&step->vars); i++){
		BuildVar* var = step->vars[i];
		
		printIndent(indent + 1);

		printfColor(COLOR_GREEN|COLOR_BLUE,
					"%s = \"%s\"\n",
					var->name,
					var->value);
					
		if(var->prompt){
			printIndent(indent + 2);
			
			printfColor(COLOR_BRIGHT|COLOR_BLUE, "%s\n", var->prompt);
			
			for(j = 0; j < eaSize(&var->choices); j++){
				printIndent(indent + 3);
				
				printfColor(COLOR_BLUE,
							"%2d. %-20s",
							j + 1,
							var->choices[j]->displayName);
							
				if(var->choices[j]->value){
					printfColor(COLOR_BLUE, "%s", var->choices[j]->value);
				}
				
				printf("\n");
			}
		}
	}
			
	for(i = 0; i < eaSize(&step->commands); i++){
		BuildCommand* cmd = step->commands[i];
		
		printIndent(indent + 1);

		printfColor(COLOR_BRIGHT|COLOR_RED|COLOR_GREEN,
					"%s: ",
					getCommandTypeName(cmd->commandType));
		
		switch(cmd->commandType){
			xcase BUILDCMDTYPE_WRITELOGFILE:{
				printfColor(COLOR_RED|COLOR_GREEN,
							"\"%s\" <",
							cmd->writeLogFile.fileName);
							
				for(j = 0; j < eaSize(&cmd->writeLogFile.textToWrite); j++){
					printfColor(COLOR_RED|COLOR_GREEN,
								" %s",
								cmd->writeLogFile.textToWrite[j]);
				}
				
				printf("\n");
			}

			xdefault:{
				for(j = 0; j < eaSize(&cmd->params); j++){
					printfColor(COLOR_RED|COLOR_GREEN, "%s%s", j ? " " : "", cmd->params[j]);
				}
				
				printf("\n");
			}
		}

	}
	
	for(i = 0; i < eaSize(&step->buildSteps); i++){
		printBuildSteps(step->buildSteps[i], indent + 1);
	}
}

void printBuildScriptSteps( BuildScript *buildScript )
{
	S32 i;
	
	for(i = 0; i < eaSize(&buildScript->buildSteps); i++){
		printBuildSteps(buildScript->buildSteps[i], 0);
	}
}
#endif

BuildState* buildStateCreate(void)
{
	BuildState* buildState = calloc(sizeof(*buildState), 1);
	
	return buildState;
}

void buildStateDestroy( BuildState **buildState )
{
	if(!*buildState){
		return;
	}
	
	SAFE_FREE(*buildState);
}

// evaluates the parameters between start and end
char *evaluateParameters(char **params, int start, int end)
{
	char* functionPrefix = "Function::";
	char* firstParam = params[start];
	S32 invert = 0;
	S32 i;
	char *ret = NULL;

	estrClear(&ret);

	if(firstParam[0] == '!')
	{
		invert = 1;
		firstParam++;
	}

	if(!strnicmp(firstParam, functionPrefix, strlen(functionPrefix)))
	{
		char* functionName = firstParam + strlen(functionPrefix);
		S32 result = 0;
		StringArray args = NULL;
		
		if(g_BuildState->flags.doPrintConditions){
			statePrintf( 1, COLOR_RED|COLOR_GREEN, "Function: %s(", functionName);

			for(i = start+1; i <= end; i++){
				printfColor(COLOR_RED|COLOR_GREEN, " \"%s\"", params[i]);
			}
			
			printfColor(COLOR_BRIGHT|COLOR_RED|COLOR_GREEN, " )\n");
		}

		for(i = start+1; i <= end; i++){
			eaPush(&args,doVariableReplacement(params[i]));
		}

		result = bfCallFunc(functionName, args);
		for ( i = 0; i < eaSize(&args); ++i )
		{
			estrDestroy(&(args[i]));
		}
		eaDestroy(&args);
		
		result = invert ? !result : result;

		estrConcatf(&ret, "%d", result);
	}
	else
	{
		char *newVal = NULL;
		char *val = firstParam;
		newVal = doVariableReplacement(val);
		if(!newVal)
			builderizerError("Invalid value: %s\n", newVal);
		if ( strIsNumeric(newVal) )
		{
			int num = atoi(newVal);
			if ( invert )
				num = !num;
			estrConcatf(&ret, "%d", num);
		}
		else
		{
			if ( invert ) // this will probably always be 1
				estrConcatf(&ret, "%d", !atoi(newVal));
			else
				estrCopy(&ret, &newVal);
			estrDestroy(&newVal);
		}
	}
	return ret;
}

int findConditional(char **params, int numParams)
{
	int i;
	for ( i = 0; i < numParams; ++i )
	{
		if ( !stricmp(params[i], "==") || !stricmp(params[i], "!=") || !stricmp(params[i], "<") ||
			!stricmp(params[i], ">") || !stricmp(params[i], "<=") || !stricmp(params[i], ">=") )
			return i;
	}
	return -1;
}

S32 evaluateCondition( BuildCondition *condition )
{
	if(conditionIsBlock(condition->ifType)){
		S32 condCount = eaSize(&condition->conditions);
		
		if(!condCount){
			builderizerError("Empty condition block.\n");
			
			return 0;
		}else{
			S32 result = 1;
			S32 i;
			S32 newAndGroup = 1;
			
			for(i = 0; i < condCount; i++){
				BuildCondition* subCond = condition->conditions[i];
				S32 doEvaluate = result;
				
				switch(subCond->ifType){
					xcase IFTYPE_IF_LINE:
					case IFTYPE_IF_BLOCK:
					{
						if(i){
							builderizerError( "\"%s\" used as non-first clause.\n", getIfTypeToken(subCond->ifType));
						}
					}
					
					xcase IFTYPE_AND_LINE:
					case IFTYPE_AND_BLOCK:
					{
						if(!i){
							builderizerError( "\"%s\" used as first clause.\n", getIfTypeToken(subCond->ifType));
						}
					}
					
					xcase IFTYPE_OR_LINE:
					case IFTYPE_OR_BLOCK:
					{
						if(!i){
							builderizerError( "\"%s\" used as first clause.\n", getIfTypeToken(subCond->ifType));
							result = 0;
						}
						
						if(result){
							return 1;
						}
						
						doEvaluate = 1;
					}
				}
				
				if(doEvaluate){
					result = evaluateCondition( subCond);
				}
			}
			
			return result;
		}
	}else{
		S32 i;
		char** params = condition->params;
		S32 paramCount = eaSize(&params);
		
		if(g_BuildState->flags.doPrintConditions){
			statePrintf( 1, COLOR_RED|COLOR_GREEN, "evaluate:");
			
			for(i = 0; i < paramCount; i++){
				printfColor(COLOR_RED|COLOR_GREEN, " %s", params[i]);
			}
			
			printf("\n");
		}
				
		if(!paramCount){
			builderizerError("No parameters to \"%s\".\n", getIfTypeToken(condition->ifType));
			
			return 0;
		}else{
			int conditional = findConditional(params, paramCount);

			if ( conditional == -1 )
				return atoi(evaluateParameters(params, 0, paramCount-1));
			else
			{
				char* op = params[conditional];
				char *lvalue = evaluateParameters(params, 0, conditional-1);
				char *rvalue = evaluateParameters(params, conditional+1, paramCount);
				int result;

				#define BEGIN_OPS		if(1){if(0);
				#define IF_OP(x)		else if(!stricmp(op, x))
				#define END_OPS			}
				
				BEGIN_OPS
					IF_OP("=="){
						result = !stricmp(lvalue, rvalue);
					}
					IF_OP("!="){
						result = !!stricmp(lvalue, rvalue);
					}
					IF_OP(">"){
						result = atoi(lvalue) > atoi(rvalue);
					}
					IF_OP("<"){
						result = atoi(lvalue) < atoi(rvalue);
					}
					IF_OP(">="){
						result = atoi(lvalue) >= atoi(rvalue);
					}
					IF_OP("<="){
						result = atoi(lvalue) <= atoi(rvalue);
					}
					else{
						builderizerError("Invalid condition operator: \"%s\"\n", op);
					}
				END_OPS
				
				#undef BEGIN_OPS
				#undef IF_OP
				#undef END_OPS

				estrDestroy(&lvalue);
				estrDestroy(&rvalue);
				return result;
			}

			//else if(paramCount == 3){
			//	const char* lValue = params[0];
			//	const char* op = params[1];
			//	const char* rValue = params[2];
			//	const char* lValueValue;
			//	
			//	//lValueValue = getBuildStateVar( lValue);
			//	lValueValue = doVariableReplacement( lValue);
			//	
			//	if(!lValueValue){
			//		builderizerError("Invalid LValue: %s\n", lValue);
			//		
			//		return 0;
			//	}else{
			//		char*	rValueValue = doVariableReplacement( rValue);
			//		S32		result = 0;
			//		
			//		#define BEGIN_OPS		if(1){if(0);
			//		#define IF_OP(x)		else if(!stricmp(op, x))
			//		#define END_OPS			}
			//		
			//		BEGIN_OPS
			//			IF_OP("=="){
			//				result = !stricmp(lValueValue, rValueValue);
			//			}
			//			IF_OP("!="){
			//				result = !!stricmp(lValueValue, rValueValue);
			//			}
			//			IF_OP(">"){
			//				result = atoi(lValueValue) > atoi(rValueValue);
			//			}
			//			IF_OP("<"){
			//				result = atoi(lValueValue) < atoi(rValueValue);
			//			}
			//			IF_OP(">="){
			//				result = atoi(lValueValue) >= atoi(rValueValue);
			//			}
			//			IF_OP("<="){
			//				result = atoi(lValueValue) <= atoi(rValueValue);
			//			}
			//			else{
			//				builderizerError("Invalid condition operator: \"%s\"\n", op);
			//			}
			//		END_OPS
			//		
			//		#undef BEGIN_OPS
			//		#undef IF_OP
			//		#undef END_OPS
			//		
			//		estrDestroy(&rValueValue);
			//		estrDestroy(&lValueValue);

			//		return result;
			//	}
			//}
			//else if(paramCount == 1){
			//	const char* lValue = params[0];
			//	S32			invert = 0;
			//	const char*	lValueValue;
			//	
			//	if(lValue[0] == '!'){
			//		lValue++;
			//		invert = 1;
			//	}
			//	
			//	lValueValue = getBuildStateVar( lValue);
			//	
			//	if(!lValueValue){
			//		builderizerError("Invalid LValue: %s\n", lValue);
			//		
			//		return invert;
			//	}
			//	
			//	return atoi(lValueValue) ? !invert : invert;
			//}
			//else{
			//	builderizerError("Can't evaluate condition with %d params: ", paramCount);

			//	for(i = 0; i < paramCount; i++){
			//		printf(" %s", params[i]);
			//	}
			//	
			//	printf("\n");
			//	
			//	return 0;
			//}
		}
	}
}

S32 checkBuildConditions( BuildCondition **conditions )
{
	S32 condCount = eaSize(&conditions), i;
	
	if(!condCount){
		return 1;
	}

	for ( i = 0; i < eaSize(&conditions); ++i )
	{
		if ( !evaluateCondition(conditions[i]) )
			return 0;
	}
	
	return 1;
}

int buildParamHasSpaces(char *param)
{
	char *c = param;
	while ( c && *c )
	{
		if ( *c == ' ' || *c == '\t' || *c == '\n' )
			return 1;
		++c;
	}
	return 0;
}

void runBuildVarsHelper( const char *varName, const char *value )
{
	char* newValue = doVariableReplacement( value);
	
	statePrintf( 1, COLOR_GREEN|COLOR_BLUE, "var: %s = \"%s\"\n", varName, newValue);
	
	if(strcmp(newValue, value)){
		printfColor(COLOR_GREEN|COLOR_BLUE, " (\"%s\")", value);
	}
	
	printf("\n");
	
	if(!setBuildStateVar( varName, newValue)){
		builderizerError("Invalid variable name: \"%s\"\n", varName);
	}
	
	estrDestroy(&newValue);
}

void runBuildVars( BuildVar **vars )
{
	S32 i;
	
	for(i = 0; i < eaSize(&vars); i++){
		BuildVar* var = vars[i];
		const char* name = var->name;
		const char* value = var->value;
		
		if(value){
			runBuildVarsHelper( name, value);
		}
		else if(var->choices){
			value = var->choices[0]->value ? var->choices[0]->value : var->choices[0]->displayName;
			
			if(value){
				runBuildVarsHelper( name, value);
			}
		}
	}
}


void runBuildListsHelper( const char *listName, const char **values )
{
	int i;

	statePrintf( 1, COLOR_GREEN|COLOR_BLUE, "list: %s = ", listName);
	for ( i = 0; i < eaSize(&values); ++i )
	{
		const char *value = values[i];
		char *newValue = doVariableReplacement(value);
		
		statePrintf( 1, COLOR_GREEN|COLOR_BLUE, "\"%s\",", newValue);
		
		if(!addToBuildStateList(listName, newValue)){
			builderizerError("Invalid variable name: \"%s\"\n", listName);
		}
		
		estrDestroy(&newValue);
	}
	printf("\n");
}

void runBuildLists( BuildList **lists )
{
	S32 i;
	
	for(i = 0; i < eaSize(&lists); i++)
	{
		BuildList* list = lists[i];
		const char *name = list->name;
		const char **values = list->values;
		
		if(values)
			runBuildListsHelper(name, values);
	}
}




char *getFileOrFolderVar( const char *varName, S32 isFolder )
{
	const char* value = getBuildStateVar( varName);
	
	if(!value){
		builderizerError("%s not set.\n", varName);
	}
	else if(!isFolder && !fileExists(value) ||
			isFolder && !dirExists(value))
	{
		builderizerError("%s specified in \"%s\" doesn't exist: \"%s\"\n", isFolder ? "Folder" : "File",
					varName, value);
	}
	else{
		char* valueCopy = NULL;
		
		estrCopy2(&valueCopy, value);
		
		forwardSlashes(valueCopy);
		
		return valueCopy;
	}
	
	return NULL;
}

char* getFileVar( const char *varName )
{
	return getFileOrFolderVar( varName, 0);
}

char* getFolderVar( const char *varName )
{
	return getFileOrFolderVar( varName, 1);
}

void printExternalMessage( char *message, S32 *useSameLine )
{
	char* token;
	char delim;
	//const char* cur;
	S32 dumpAll = 0;
	
	if(0){
		printf("message: ");
		//for(cur = message; *cur; cur++){
		//	if(*cur >= 32 && *cur <= 127){
		//		printfColor(COLOR_GREEN, "%c", *cur);
		//	}else{
		//		printfColor(COLOR_RED, "[%d]", *cur);
		//	}
		//}
		printfColor(COLOR_GREEN, "%s", message);
		printf("\n");
	}
		
	while(token = strsep2(&message, "\n", &delim)){
		if(dumpAll || !*useSameLine){
			statePrintfDim( 2, 0, NULL);
		}
		
		//for(cur = token; *cur; cur++){
		//	if(*cur >= 32 && *cur <= 127){
		//		printfColor(COLOR_GREEN, "%c", *cur);
		//	}else{
		//		printfColor(COLOR_RED, "[%d]", *cur);
		//	}
		//}
		printfColor(COLOR_GREEN, "%s", token);
		
		*useSameLine = delim != '\n';
		
		if(dumpAll || !*useSameLine){
			printf("\n");
		}
	}
}


void runBuildCommand( BuildCommand *command )
{
	switch(command->commandType){
		#define CASE(cmdType, handlerFunc) xcase cmdType:handlerFunc( command)
		CASE(BUILDCMDTYPE_SYSTEM,			runBuildCommandSystem);
		CASE(BUILDCMDTYPE_RUN,				runBuildCommandRun);
		CASE(BUILDCMDTYPE_PUSHDIR,			runBuildCommandPushDir);
		CASE(BUILDCMDTYPE_POPDIR,			runBuildCommandPopDir);
		CASE(BUILDCMDTYPE_KILL_PROCESSES,	runBuildCommandKillProcesses);
		CASE(BUILDCMDTYPE_WAIT_FOR_PROCS,	runBuildCommandWaitForProcs);
		CASE(BUILDCMDTYPE_DELETE_FILES,		runBuildCommandDeleteFiles);
		CASE(BUILDCMDTYPE_COPY,				runBuildCommandCopy);
		CASE(BUILDCMDTYPE_RENAME,			runBuildCommandRename);
		CASE(BUILDCMDTYPE_GIMME,			runBuildCommandGimme);
		CASE(BUILDCMDTYPE_DELETE_FOLDER,	runBuildCommandDeleteFolder);
		CASE(BUILDCMDTYPE_MAKE_FOLDER,		runBuildCommandMakeFolder);
		CASE(BUILDCMDTYPE_SVN,				runBuildCommandSVN);
		CASE(BUILDCMDTYPE_TOOL,				runBuildCommandTool);
		CASE(BUILDCMDTYPE_COMPILE,			runBuildCommandCompile);
		CASE(BUILDCMDTYPE_BUILD,			runBuildCommandBuild);
		CASE(BUILDCMDTYPE_SRCEXE,			runBuildCommandSrcExe);
		CASE(BUILDCMDTYPE_FUNCTION,			runBuildCommandFunction);
		CASE(BUILDCMDTYPE_WRITELOGFILE,		runBuildCommandWriteLogFile);
		CASE(BUILDCMDTYPE_PRINT,			runBuildCommandPrint);
		CASE(BUILDCMDTYPE_ERROR,			runBuildCommandError);
		CASE(BUILDCMDTYPE_SLEEP,			runBuildCommandSleep);
		CASE(BUILDCMDTYPE_PROC_KILL,		runBuildCommandProcKill);
		CASE(BUILDCMDTYPE_LIST_SORT,		runBuildCommandListSort);
		CASE(BUILDCMDTYPE_VAR_ADD,			runBuildCommandVarAdd);
		CASE(BUILDCMDTYPE_VAR_SUB,			runBuildCommandVarSub);
		CASE(BUILDCMDTYPE_VAR_MUL,			runBuildCommandVarMul);
		CASE(BUILDCMDTYPE_VAR_DIV,			runBuildCommandVarDiv);
		#undef CASE
		xdefault:{
			statePrintf( 1, COLOR_RED|COLOR_GREEN, "WARNING: Unknown command type: %d\n", command->commandType);
		}
	}
}

void runBuildCommands( BuildCommand **commands )
{
	S32 i;
	
	for(i = 0; i < eaSize(&commands); i++){
		runBuildCommand( commands[i]);
	}
}

S32 runBuildStep( BuildStep *step )
{
	S32 doRunStep;
	S32 resetSkipApproval = 0;
	g_BuildState->curBuildStep = step;
	
	statePrintf( 0, 0, "-");
	statePrintf( 0, step->simulate || g_BuildState->flags.simulate ? COLOR_BLUE : COLOR_GREEN, 
		"%s %s\n", step->isElse ? " ELSE:" : 
			(step->simulate || g_BuildState->flags.simulate ? "Simuated:" : ""), 
		getDisplayName(step));

	
	doRunStep = checkBuildConditions(step->conditions);
	
	if(	doRunStep &&
		g_BuildState->flags.doApproveEachStep &&
		!g_BuildState->flags.doSkipApproval)
	{
		S32 done = 0;

		statePrintf( 1, COLOR_RED|COLOR_GREEN, "Run this step? (y/n/a): ");
		
		while(!done){
			done = 1;
			switch(tolower(_getch())){
				xcase 'y':
					printfColor(COLOR_BRIGHT|COLOR_GREEN, "YES!\n");
				xcase 'n':
					printfColor(COLOR_BRIGHT|COLOR_RED, "NO!\n");
					doRunStep = 0;
				xcase 'a':
					printfColor(COLOR_BRIGHT|COLOR_GREEN, "ALL!\n");
					g_BuildState->flags.doSkipApproval = 1;
					resetSkipApproval = 1;
				xcase 0:
					_getch();
					done = 0;
				xdefault:
					done = 0;
			}
		}
	}
	
	if(!doRunStep){
		statePrintfDim( 1, COLOR_GREEN, "*** STEP SKIPPED! ***\n");
		return 0;
	}

	buildStatusUpdateStep(getDisplayName(step), 0.0f, 1, 0);
	timerStart(timer);

	// wait while the requirements of this step arent filled
	while ( !checkBuildConditions(step->requirements) )
		Sleep(100);
	
	if(step->commands){
		if(step->buildSteps){
			builderizerError("Can't have substeps within command steps!\n");
		}
		
		if(step->vars || step->lists){
			builderizerError( "Can't have variable changes within command steps!\n");
		}
		
		if ( step->simulate )
			g_BuildState->flags.simulate++;
		runBuildCommands( step->commands);
		if ( g_BuildState->flags.simulate > 0 && step->simulate )
			g_BuildState->flags.simulate--;

		// was there an error?
		if ( getErrorLevel() ) {
			step->failed = 1;
			return -1;
		}
	}
	else if(step->vars || step->lists){
		if(step->buildSteps){
			builderizerError("Can't have substeps within variable steps!\n");
		}

		if ( step->vars )
			runBuildVars(step->vars);
		if ( step->lists )
			runBuildLists(step->lists);
	}
	else if(step->buildSteps){
		int i;

		if ( step->simulate )
			g_BuildState->flags.simulate++;
		timer = getNewTimer();
		runBuildSteps( step->buildSteps, 0 );
		timer = getLastTimer();
		if ( g_BuildState->flags.simulate > 0 && step->simulate )
			g_BuildState->flags.simulate--;

		// if a child failed, fail the parent
		for ( i = 0; i < eaSize(&step->buildSteps); ++i ) {
			if ( step->buildSteps[i]->failed ) {
				step->failed = 1;
				break;
			}
		}
	}
	
	if(resetSkipApproval){
		g_BuildState->flags.doSkipApproval = 0;
	}

	step->timeSpent = timerElapsed(timer);
	buildStatusUpdateStep(getDisplayName(step), step->timeSpent, 0, 0);
	statePrintfDim(1, COLOR_BLUE|COLOR_GREEN, "Step completed in %f seconds\n", step->timeSpent);

	return 1;
}

void runBuildSteps( BuildStep **steps, int startIdx )
{
	BuildStep*	prevBuildStep = g_BuildState->curBuildStep;
	S32			prevRetVal = 0;
	S32			i, buildStepIdx;
	
	if(!steps){
		return;
	}
	
	g_BuildState->stepDepth++;

	for(buildStepIdx = startIdx; buildStepIdx < eaSize(&steps); buildStepIdx++){
		BuildStep* step = steps[buildStepIdx];

		step->failed = 0;
		
		if(step->isElse){
			if(!buildStepIdx){
				builderizerError("Else without preceding if: %s\n", getDisplayName(step));
			}
			
			if(prevRetVal){
				continue;
			}
		}

#define RUN_THIS_STEP(var,val)	{													\
	setBuildStateVar((var), (val));													\
	prevRetVal = runBuildStep(step);												\
	totalStepTime += step->timeSpent;												\
	if ( step->failed && !step->continueOnError ) {									\
		builderizerError("Step \"%s\" failed, Quitting...\n", getDisplayName(step));\
		goto build_step_failed;														\
	}																				\
	else if ( step->continueOnError )												\
	{																				\
		if ( getErrorLevel() )														\
			decErrorLevel();														\
		step->failed = 0;															\
	}	}

		setConsoleTitle(getDisplayName(step));
		
		// does the step use a for loop?
		if ( step->forLoopValues ) {
			char *varName = step->forLoopValues[0];
			float totalStepTime = 0.0f;
			// a for loop in the format "For [var] in [list] ..." or "For [var] in [filespec]"
			if ( !stricmp(step->forLoopValues[1], "in") ) {

				if ( strchr(step->forLoopValues[2], '*') &&
					eaSize(&step->forLoopValues) == 3 || eaSize(&step->forLoopValues) == 4 )
				{
					char fileDirectory[MAX_PATH];
					char fileFullSpec[MAX_PATH];
					char *newParam = doVariableReplacement(step->forLoopValues[2]);
					char **files = NULL;
					int count;

					strncpy_s(fileDirectory, MAX_PATH, newParam, MAX_PATH);
					backSlashes(fileDirectory);
					strcpy_s(SAFESTR(fileFullSpec),fileDirectory);
					getDirectoryName(fileDirectory);

					if (eaSize(&step->forLoopValues) == 4 && !stricmp(step->forLoopValues[3], "/r"))
						files = fileScanDir(fileDirectory, &count);
					else
						files = fileScanDirNoSubdirRecurse(fileDirectory, &count);

					
					for ( i = 0; i < count; ++i )
					{
						if ( simpleMatch(fileFullSpec,backSlashes(files[i])) )
						{
							RUN_THIS_STEP(varName,files[i]);
						}
					}
					
					fileScanDirFreeNames(files, count);
					estrDestroy(&newParam);
				}
				else
				{
					// iterate over each value in [list], setting [var] to the current value
					for ( i = 2; i < eaSize(&step->forLoopValues); ++i ) {
						char *curVal = step->forLoopValues[i];
						if ( isBuildStateList(curVal) )
						{
							int j;
							const char **list = getBuildStateList(curVal);
							for ( j = 0; j < eaSize(&list); ++j )
							{
								RUN_THIS_STEP(varName,list[j]);
							}
						}
						else
							RUN_THIS_STEP(varName,curVal);
					}
				}
			}
			// a for loop in the format "For [var] from [min] to [max]"  iterate over the range [min]-[max] setting [var] to the current value.
			// it is possible to make an infinite loop by not supplying the "to [max]" part.  
			else if ( !stricmp(step->forLoopValues[1], "from") ) {
				int min, max;
				char *newVal = doVariableReplacement(step->forLoopValues[2]);
				min = atoi(newVal);
				estrClear(&newVal);
				if ( eaSize(&step->forLoopValues) == 5 )
				{
					newVal = doVariableReplacement(step->forLoopValues[4]);
					max = atoi(newVal);
					estrDestroy(&newVal);
				}
				else {
					builderizerError("Bad for loop syntax in step \"%s\".  Should be \"For [variable] in [list] ...\" or \"For [variable] from [min] to [max]\"\n", getDisplayName(step));
					if ( !step->continueOnError )
						goto build_step_failed;
					else if ( getErrorLevel() )
						decErrorLevel();
				}
				for ( i = min; max ? i <= max : 1; ++i ) {
					char num[16];
					_itoa_s(i, SAFESTR(num), 10);
					RUN_THIS_STEP(varName,num);
				}
			}
			else {
				builderizerError("Bad for loop syntax in step \"%s\".  Should be \"For [variable] in [list] ...\" or \"For [variable] from [min] to [max]\"\n", getDisplayName(step));
				if ( !step->continueOnError )
					goto build_step_failed;
				else if ( getErrorLevel() )
					decErrorLevel();
			}

			step->timeSpent = totalStepTime;
			buildStatusUpdateStep(getDisplayName(step), step->timeSpent, 0, step->failed);
		}
		// not a for loop, just run once
		else {	
			prevRetVal = runBuildStep( step);

			if ( step->failed && !step->continueOnError ) {
				buildStatusUpdateStatus(1);
				builderizerError("Step \"%s\" failed, Quitting...\n", getDisplayName(step));
				goto build_step_failed;
			}
			else if ( step->continueOnError )
			{
				if ( getErrorLevel() )
					decErrorLevel();
				step->failed = 0;
			}
		}
		g_BuildState->curBuildStep = prevBuildStep;
	}

build_step_failed:

	g_BuildState->stepDepth--;
}

void initializeBuildStatus(BuildStep **steps)
{
	int i;
	for ( i = 0; i < eaSize(&steps); ++i )
	{
		buildStatusUpdateStep(getDisplayName(steps[i]), 0.0f, 0, 0);
		if ( steps[i]->buildSteps )
			initializeBuildStatus(steps[i]->buildSteps);
	}
}

static void profileBuild()
{
	// TODO:
	// print tree hierarchy to profilePath, sorted by steps that took the longest
}

void runBuildScript()
{
	initializeBuildStatus(g_BuildScript->buildSteps);
	runBuildSteps(g_BuildScript->buildSteps, g_BuildState->curBuildStepIdx);
}

unsigned int __stdcall runBuildStepThread(void *args)
{
	BuildStep *step = (BuildStep*)args;
	initBuildScript();
	eaPush(&g_BuildScript->buildSteps, step);
	runBuildScript();
	return 0;
}

unsigned int __stdcall runBuildScriptThread(void *args)
{
	if ( args )
		readScriptFiles((char*)args);
	setErrorLevel(0);
	runBuildScript();
	return 0;
}

unsigned int __stdcall resumeBuildScriptThread(void *args)
{
	recoverFromError();
	runBuildScript();
	return 0;
}

F32 getTotalBuildTime( BuildStep **steps )
{
	int i;
	F32 totalTime = 0.0f;

	for(i = 0; i < eaSize(&steps); i++){
		BuildStep *step = steps[i];
		totalTime += step->timeSpent;
	}

	return totalTime;
}

void runBuilderizer( S32 argc, char **argv )
{
	//BuildState* buildState = buildStateCreate();
	g_BuildState = buildStateCreate();
	timer = getNewTimer();
	
	bfRegisterAllFunctions();
	buildStatusInit();

	// let both the server and the builderizer threads use the text parser
	ParserLetMeCallYouInMultipleThreadsWithoutAssertingIKnowWhatImDoing();

	// set the default log file to be in builderizerdir/logs/build.log
	{
		int len;
		getcwd(log_file_path, MAX_PATH-1);
		len = (int)strlen(log_file_path);
		strncpyt(&log_file_path[len], "/logs/build.log", MAX_PATH - len);
		backSlashes(log_file_path);
	}
	
	parseCommandLine(argc, argv);

	if ( client_mode )
	{
		buildclientRun();
	}
	else if ( server_mode )
	{
		bcfgLoad(buildConfigPath[0] ? buildConfigPath : "./builderizer.cfg");
		buildserverInit();
		printf("Build server started...\n");
		buildserverRun();
	}
	else
	{
		if (!fileIsAbsolutePath(buildScriptPath))
		{
			char curdir[MAX_PATH];
			getcwd(curdir, MAX_PATH-1);
			strcatf(curdir, "/%s", buildScriptPath);
			strcpy(buildScriptPath, curdir);
		}
		readScriptFiles(buildScriptPath);
		
		//printBuildScriptSteps(buildScript);
		
		for (;;)
		{
			runBuildScript();

			if ( getErrorLevel() )
			{	
				printf( "Fix the error and press enter to continue.\n" );
				system("pause");
				if ( !recoverFromError() )
					break;
			}
			else
				break;
			
		}

		statePrintfDim(1, COLOR_BLUE|COLOR_GREEN, "%s completed in %f minutes\n", buildScriptPath, 
			getTotalBuildTime(g_BuildScript->buildSteps) / 60.0f );
		
		if ( waitOnExit )
		{
			printf("press a key to exit\n");
			_getch();
		}
	}
}
