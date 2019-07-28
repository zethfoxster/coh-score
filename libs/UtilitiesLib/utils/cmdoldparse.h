#ifdef _CMDPARSE_H
#error "can't include old and new cmdparse in same translation unit"
#endif

// define _CMDOLDPARSE_H
#ifndef _CMDOLDPARSE_H
#define _CMDOLDPARSE_H 1


#include "network\netio.h"
#include "timing.h"
#include "cmdoldparsetypes.h"

C_DECLARATIONS_BEGIN

#define CMDMAXARGS 11 // Raise this if we need more arguments per command
#define CMDMAXLENGTH 10000 // The longest command we can successfully parse. This is hardcoded for now
#define CMDMAXDATASWAPS 4 // Raise this if we need more

// These macros are used for the old argument passing that needs explicit paramaters
#define CMDINT(x) \
	PARSETYPE_S32, (sizeof(x)==sizeof(int))?&x:&x+(5/0)
#define CMDSTR(x) PARSETYPE_STR, x, sizeof(x)
#define CMDSENTENCE(x) PARSETYPE_SENTENCE, x, sizeof(x)

// These are for the new argument passing that creates the paramaters for you.
#define ARGMAT4 {PARSETYPE_MAT4}
#define ARGVEC3 {PARSETYPE_VEC3}
#define ARGF32 {PARSETYPE_FLOAT}
#define ARGU32 {PARSETYPE_U32}
#define ARGS32 {PARSETYPE_S32}
#define ARGSTR(size) {PARSETYPE_STR, 0, (size)}
#define ARGSENTENCE(size) {PARSETYPE_SENTENCE, 0, (size)}
#define ARGSTRUCT(str,def) {PARSETYPE_TEXTPARSER, def, sizeof(str)}


typedef enum CmdFlag
{
	CMDF_HIDEVARS = 1,		 // If you have a one-parameter commands in the explicit parameter style, set this to not have it printed when 0 args are passed
	CMDF_APPLYOPERATION = 2, // For one-parameter old-style commands, allows the user to call with a & or | to modify the value
	CMDF_RETURNONERROR = 4,  // Causes cmdRead to return the command even if the number of parameters was not correct
	CMDF_HIDEPRINT = 8,		 // don't show command in 'cmdlist'
} CmdFlag;

typedef enum CmdOperation
{
	CMD_OPERATION_NONE = 0,
	CMD_OPERATION_AND,
	CMD_OPERATION_OR,
} CmdOperation;

// The union of possible argument types. Add them here as needed
typedef struct CmdArg
{
	union
	{	
		S32 iArg;
		U32 uiArg;
		F32 fArg;
		void *pArg;
	};
} CmdArg;

typedef struct CmdDataSwap
{
	void *source;
	intptr_t size;
	void *dest;
} CmdDataSwap;

typedef struct CmdServerContext CmdServerContext;

typedef struct CmdContext
{
	CmdOperation	op;
	int				orig_value;
	char			msg[1000]; // An error or general output message
	bool			found_cmd; // set to true if a command was found (but not necessarily executed, i.e. if it had the wrong number of parameters)
	int				access_level; // The access level of the person executing this
	CmdArg			args[CMDMAXARGS]; // Where the arg and arg pointers are stored for new-style calls
	//CmdDataSwap		**data_swap;
	CmdDataSwap		data_swap[CMDMAXDATASWAPS];
	int				numDataSwaps;

	// union for ease of use
	union{
		void		*data; // Arbitrary data that may be needed by some handler functions
		CmdServerContext *svr_context;
	};
	char const *cmdstr; // the original command string passed in.
} CmdContext;

typedef struct DataDesc
{
	CmdParseType type;
	void	*ptr;
	int		maxval;
} DataDesc;

typedef struct PerfInfoStaticData PerfInfoStaticData;
typedef struct PerformanceInfo PerformanceInfo;
typedef struct StashTableImp *StashTable;
typedef const struct StashTableImp *cStashTable;
typedef struct Cmd Cmd;

typedef void (*CmdHandler)(Cmd *c, CmdContext *o);

typedef struct Cmd
{
	int					access_level;
	char				*name;
	int					num;
	DataDesc			data[CMDMAXARGS];
	int					flags;
	char				*comment;
	CmdHandler			handler;
	int					send;
	int					num_args;
	int					usage_count; // Make sure this is second to last?
	#if USE_NEW_TIMING_CODE
		PerfInfoStaticData*	perfInfo; // Make sure this is last.
	#else
		PerformanceInfo*	perfInfo; // Make sure this is last.
	#endif
} Cmd;

// The max length of a command module name
#define CMDMODULELEN 32

typedef struct CmdGroup
{
	Cmd	*cmds;	
	CmdHandler handler;
	char header[CMDMODULELEN];
} CmdGroup;

// The max number of cmd groups per list. Increase if you need more modules
#define CMDMAXGROUPS 32

typedef struct CmdList
{
	CmdGroup groups[CMDMAXGROUPS];
	CmdHandler handler;
	StashTable	name_hashes;
} CmdList;

// These macros are for easy access to the arglist in CmdContext
// All of these macros assume the args are named what they are in CMDARGS

#define CMDARGS Cmd *cmd, CmdContext *cmd_context

#define STARTARG int argIndex = 0;
#define GETARG(var,vartype) assertmsg(argIndex < cmd->num_args,"Trying to get " #var " from past end of arguments!");\
	switch(cmd->data[argIndex].type) {	\
case PARSETYPE_S32: var = *((vartype *)&cmd_context->args[argIndex].iArg);break;\
case PARSETYPE_U32: var = *((vartype *)&cmd_context->args[argIndex].uiArg);break;\
case PARSETYPE_FLOAT: case PARSETYPE_EULER: var = *((vartype *)&cmd_context->args[argIndex].fArg);break;\
case PARSETYPE_STR: case PARSETYPE_SENTENCE: case PARSETYPE_VEC3: case PARSETYPE_MAT4: case PARSETYPE_TEXTPARSER: var = *((vartype *)&cmd_context->args[argIndex].pArg);break;\
default: assertmsg(0,"Invalid argument type!"); var = 0; break;}argIndex++;

#define READARGS1(vartype1, var1) \
	vartype1 var1;STARTARG;\
	GETARG(var1,vartype1);

#define READARGS2( vartype1, var1, vartype2, var2) \
	vartype1 var1;vartype2 var2;STARTARG;\
	GETARG(var1,vartype1);GETARG(var2,vartype2);

#define READARGS3( vartype1, var1, vartype2, var2, vartype3, var3) \
	vartype1 var1;vartype2 var2;vartype3 var3;STARTARG;\
	GETARG(var1,vartype1);GETARG(var2,vartype2);\
	GETARG(var3,vartype3);

#define READARGS4( vartype1, var1, vartype2, var2, vartype3, var3, vartype4, var4) \
	vartype1 var1;vartype2 var2;vartype3 var3;vartype4 var4;STARTARG;\
	GETARG(var1,vartype1);GETARG(var2,vartype2);\
	GETARG(var3,vartype3);GETARG(var4,vartype4);

#define READARGS5( vartype1, var1, vartype2, var2, vartype3, var3, vartype4, var4, vartype5, var5) \
	vartype1 var1;vartype2 var2;vartype3 var3;vartype4 var4;vartype5 var5;STARTARG;\
	GETARG(var1,vartype1);GETARG(var2,vartype2);\
	GETARG(var3,vartype3);GETARG(var4,vartype4);\
	GETARG(var5,vartype5);

#define READARGS6( vartype1, var1, vartype2, var2, vartype3, var3, vartype4, var4, vartype5, var5,vartype6,var6) \
	vartype1 var1;vartype2 var2;vartype3 var3;vartype4 var4;vartype5 var5;vartype6 var6;STARTARG;\
	GETARG(var1,vartype1);GETARG(var2,vartype2);\
	GETARG(var3,vartype3);GETARG(var4,vartype4);\
	GETARG(var5,vartype5);GETARG(var6,vartype6);

#define READARGS7( vartype1, var1, vartype2, var2, vartype3, var3, vartype4, var4, vartype5, var5,vartype6,var6,vartype7,var7) \
	vartype1 var1;vartype2 var2;vartype3 var3;vartype4 var4;vartype5 var5;vartype6 var6;vartype7 var7STARTARG;\
	GETARG(var1,vartype1);GETARG(var2,vartype2);\
	GETARG(var3,vartype3);GETARG(var4,vartype4);\
	GETARG(var5,vartype5);GETARG(var6,vartype6);\
	GETARG(var7,vartype7);

#define READARGS8( vartype1, var1, vartype2, var2, vartype3, var3, vartype4, var4, vartype5, var5,vartype6,var6,vartype7,var7,vartype8,var8) \
	vartype1 var1;vartype2 var2;vartype3 var3;vartype4 var4;vartype5 var5;vartype6 var6;vartype7 var7STARTARG;\
	GETARG(var1,vartype1);GETARG(var2,vartype2);\
	GETARG(var3,vartype3);GETARG(var4,vartype4);\
	GETARG(var5,vartype5);GETARG(var6,vartype6);\
	GETARG(var7,vartype7);GETARG(var8,vartype8);


// Used to create shortcut bindings which are used to speed up network transfer of commands
void cmdOldAddShortcut(char *name);
void cmdOldSetShortcut(char *name,int idx);
char *cmdOldShortcutName(int idx);
int cmdOldShortcutIdx(char *name);

// Prints usage information for a command
void cmdOldPrint(Cmd *cmd,char *buf, int buflen, Cmd *cmds, CmdContext *output);

// Parses and executes all data setting in the command, returning so it can be processed later
Cmd *cmdOldRead(CmdList *cmd_list, const char *cmdstr,CmdContext* output);

// Cleans up the result of a cmdRead call, this should be called after the case callback
void cmdOldCleanup(Cmd *cmd, CmdContext* output);

// Sends a list of commands on the packet. For now only works for data setting commands
int cmdOldSend(Packet *pak,Cmd *cmds,CmdContext *output,int all);

// Clears the saved packet ids used for synch, in case of a player or map restart
void cmdOldResetPacketIds(CmdList *cmd_list);

// Receives a list of commands
void cmdOldReceive(Packet *pak,CmdList *cmd_list,CmdContext *output);

// Sets rather a list of commands should be sent on next send
void cmdOldSetSends(Cmd *cmds,int val);

// Initializes and error checks a command list
void cmdOldInit(Cmd *cmds);

// Initializes all cmds in a full command list
void cmdOldListInit(CmdList *list);

// Returns the number of commands in a list
int cmdOldCount(Cmd *cmds);

// Saves out the command list in a translation format
void cmdOldSaveMessageStore( CmdList *cmdlist, char * filename );

// Does a specified operation to the output
void cmdOldApplyOperation(int* var, int value, CmdContext* output);

// Adds a registry variable
void cmdOldVarsAdd(int i, char *s);

// Dispatches a command to the appropriate handler
int cmdOldExecute(CmdList *cmdlist, const char* cmdstr, CmdContext *context );

// Checks to see if this command could be executed, but don't actually do it.
int cmdOldCheckSyntax(CmdList *cmdlist, const char* cmdstr, CmdContext *context );

// Add or clear data swaps from a context. Data swaps are used to modify what object is changed
void cmdOldAddDataSwap(CmdContext *con, void *src, int size, void *dest);
void cmdOldClearDataSwap(CmdContext *con);

int cmdOldTabCompleteComand(const char* pcStart, const char** pcResult, int iSearchID, bool bSearchBackwards);

typedef int (*CmdParseFunc)(const char *cmd);

// void cmdOldSetGlobalCmdParseFunc(CmdParseFunc func);
// extern CmdParseFunc globCmdOldParse;
void cmdOldAddToGlobalCmdList(Cmd *cmds, CmdHandler handler, char *header);
extern CmdList globCmdOldList;
// void globCmdOldParsef(const char *fmt, ...);

void cmdOldListAddToHashes(CmdList *cmdlist, int index);

// No automatic handling of private commands, the app/library must have their own
void cmdOldAddToPrivateCmdList(Cmd *cmds, CmdHandler handler, char *header);
extern CmdList privCmdList;

// Find a particular command
Cmd *cmdOldListFind(CmdList *cmdlist, const char *name, const char *header, CmdHandler *handler);

C_DECLARATIONS_END

#endif // _CMDOLDPARSE_H
