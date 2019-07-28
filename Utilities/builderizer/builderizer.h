#include "StashTable.h"
#include "earray.h"
#include "EString.h"
#include "StringTable.h"
#include "file.h"
#include "textparser.h"

typedef struct BuildStep		BuildStep;
typedef struct BuildScript		BuildScript;

typedef struct BuildVarChoice {
	char*					displayName;
	char*					value;
} BuildVarChoice;

typedef struct BuildVar {
	char*					name;
	char*					value;
	
	char*					prompt;
	BuildVarChoice**		choices;
} BuildVar;

typedef struct BuildList {
	char *name;
	char **values;
} BuildList;

typedef enum BuildConditionIfType {
	IFTYPE_IF_BLOCK,
	IFTYPE_IF_LINE,
	IFTYPE_AND_BLOCK,
	IFTYPE_AND_LINE,
	IFTYPE_OR_BLOCK,
	IFTYPE_OR_LINE,
} BuildConditionIfType;

typedef struct BuildCondition BuildCondition;
typedef struct BuildCondition {
	char**					params;
	
	BuildConditionIfType	ifType;
	
	BuildCondition**		conditions;
} BuildCondition;

typedef enum BuildCommandType {
	BUILDCMDTYPE_SYSTEM,
	BUILDCMDTYPE_RUN,
	BUILDCMDTYPE_PUSHDIR,
	BUILDCMDTYPE_POPDIR,
	BUILDCMDTYPE_KILL_PROCESSES,
	BUILDCMDTYPE_WAIT_FOR_PROCS,
	BUILDCMDTYPE_DELETE_FILES,
	BUILDCMDTYPE_COPY,
	BUILDCMDTYPE_RENAME,
	BUILDCMDTYPE_GIMME,
	BUILDCMDTYPE_DELETE_FOLDER,
	BUILDCMDTYPE_MAKE_FOLDER,
	BUILDCMDTYPE_SVN,
	BUILDCMDTYPE_TOOL,
	BUILDCMDTYPE_COMPILE,
	BUILDCMDTYPE_BUILD,
	BUILDCMDTYPE_SRCEXE,
	BUILDCMDTYPE_FUNCTION,
	BUILDCMDTYPE_WRITELOGFILE,
	BUILDCMDTYPE_PRINT,
	BUILDCMDTYPE_ERROR,
	BUILDCMDTYPE_SLEEP,
	BUILDCMDTYPE_PROC_KILL,
	BUILDCMDTYPE_LIST_SORT,
	BUILDCMDTYPE_VAR_ADD,
	BUILDCMDTYPE_VAR_SUB,
	BUILDCMDTYPE_VAR_MUL,
	BUILDCMDTYPE_VAR_DIV,
	//----------------
	BUILDCMDTYPE_COUNT,
} BuildCommandType;

typedef struct BuildCommand {
	BuildCommandType		commandType;
	
	char**					params;
	
	struct{
		char*				fileName;
		char**				textToWrite;
	} writeLogFile;
} BuildCommand;

typedef struct BuildStep {
	S32						isElse;
	
	char*					displayName;
	F32						timeSpent;

	bool					continueOnError;
	bool					simulate;
	bool					failed;
	bool					forceRun;  // if a step is set as ran but forceRun is set, the step will be run again

	BuildStep**				buildSteps;
	//BuildConfiguration**	configurations;
	BuildVar**				vars;
	BuildList**				lists;
	BuildCondition**		conditions; // if a condition is false, the build step is skipped
	BuildCondition**		requirements; // if a requirement is false, the step is delayed until the requirement is true
	char**					forLoopValues;
	BuildCommand**			commands;
} BuildStep;

typedef struct BuildScript {
	BuildStep**				buildSteps;
} BuildScript;

typedef struct BuildState {
	S32				stepDepth;
	
	StringTable		stringTableVarNames;
	StringArray		dirStack;
	StashTable		stVariables;
	StashTable		stLists;
	BuildStep*		curBuildStep;
	U32				curBuildStepIdx;
	S32				errorLevel;
	
	struct {
		U32			doRunCommands			: 1;
		U32			doPrintConditions		: 1;
		U32			doPrintExternalCommands	: 1;
		U32			doApproveEachStep		: 1;
		U32			doSkipApproval			: 1;
		U32			simulate				: 1;
		U32			disableLogging			: 1;
		U32			profile					: 1;
	} flags;
} BuildState;

extern BuildState *g_BuildState;
extern ParseTable tpiBuildStep[];

void setErrorLevel(int level);
void incErrorLevel();
void decErrorLevel();
int getErrorLevel();

BuildState *getBuildState(void); // WARNING: not thread safe
char *getDisplayName( BuildStep *step ); // WARNING: not thread safe
const char* getCommandTypeName( BuildCommandType cmdType );
S32 isValidBuildVarName( const char *name );
S32 setBuildStateVar( const char *name, const char *value );
S32 addToBuildStateList( const char *name, const char *value );
void clearAllBuildStateLists( void );
S32 clearBuildStateList( const char *name );
int isBuildStateList( const char *name );
void builderizerLogf(char *format, ...);
void printIndent( S32 indent );
void printStateIndentPlus( S32 extraIndent );
void printStateIndent( void );
void statePrintv( S32 extraIndent, U32 color, const char *format, va_list va);
void statePrintf( S32 extraIndent, U32 color, const char *format, ... );
void statePrintfNoLog( S32 extraIndent, U32 color, const char* format, ... ); // same as above except doesnt write to log
void statePrintfDim( S32 extraIndent, U32 color, const char* format, ... );
void statePrintfDimNoLog( S32 extraIndent, U32 color, const char* format, ... ); // same as above except doesnt write to log
void builderizerError( const char *format, ... );
int recoverFromError();
const char* getBuildStateVar( const char *name );
char** getBuildStateList( const char *name );
char* doVariableReplacementEx( const char *input, int *isList );
#define doVariableReplacement(input) doVariableReplacementEx(input,NULL)
void readScriptFiles( char *scriptPath );
void parseCommandLine( S32 argc, char **argv );
S32 conditionIsBlock( BuildConditionIfType ifType );
const char* getIfTypeToken( BuildConditionIfType ifType );
BuildState* buildStateCreate( void );
void buildStateDestroy( BuildState **buildState );
S32 evaluateCondition( BuildCondition *condition );
S32 checkBuildConditions( BuildCondition **conditions );
void runBuildVarsHelper( const char *varName, const char* value );
void runBuildVars( BuildVar **vars );
char* getFileOrFolderVar( const char *varName, S32 isFolder );
char* getFileVar( const char *varName );
char* getFolderVar( const char *varName );
int buildParamHasSpaces(char *param);
void printExternalMessage( char *message, S32 *useSameLine );
void runBuildCommand( BuildCommand *command );
void runBuildCommands( BuildCommand **commands );
void runBuildSteps( BuildStep **steps, int startIdx );
S32 runBuildStep( BuildStep *step );
void runBuildScript();
unsigned int __stdcall runBuildStepThread(void *);
unsigned int __stdcall runBuildScriptThread(void *);
unsigned int __stdcall resumeBuildScriptThread(void *args);
void runBuilderizer(S32 argc, char **argv );