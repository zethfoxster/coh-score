#include "DebugState.h"
//#include "cmdoldparse.h"

DbgState dbg_state;

// CmdOld dbg_cmds[] = {
// #define PARSE_TEST_VAR(i) \
// 	{ 9, "test" #i, 0, {{ TYPE_FLOAT, &dbg_state.test##i }}, 0,				\
// 						"temporary debugging params odds and ends" },		\
// 	{ 9, "test" #i "flicker", 0, {{ TYPE_INT, &dbg_state.test##i##flicker }}, 0,	\
// 						"temporary debugging params odds and ends" },
// 	PARSE_TEST_VAR(1)
// 	PARSE_TEST_VAR(2)
// 	PARSE_TEST_VAR(3)
// 	PARSE_TEST_VAR(4)
// 	PARSE_TEST_VAR(5)
// 	PARSE_TEST_VAR(6)
// 	PARSE_TEST_VAR(7)
// 	PARSE_TEST_VAR(8)
// 	PARSE_TEST_VAR(9)
// 	PARSE_TEST_VAR(10)
// 	{ 9, "testInt1", 0, {{ CMDINT(dbg_state.testInt1) }}, 0,
// 						"temporary debugging params odds and ends" },
// 	{ 9, "testDisplay", 0, {{ CMDINT(dbg_state.testDisplayValues) }},0,
// 						"temporary debugging params odds and ends" },
// 	{ 0 },
// };

int dbgCommandParse(Cmd *cmd, CmdContext *cmd_context)
{
	//// Command is good
	//switch(cmd->num)
	//{
	//}
	return 1;
}

//CmdList dbg_cmdlist = {
//	{{ dbg_cmds},
//	{ 0 }},&dbgCommandParse
//};

void dbgOncePerFrame(void)
{
	// Do flickering
#define DOFLICKER(var)	\
	if (dbg_state.test##var##flicker) dbg_state.test##var = (F32)!dbg_state.test##var;
	DOFLICKER(1);
	DOFLICKER(2);
	DOFLICKER(3);
	DOFLICKER(4);
	DOFLICKER(5);
	DOFLICKER(6);
	DOFLICKER(7);
	DOFLICKER(8);
	DOFLICKER(9);
	DOFLICKER(10);
}
