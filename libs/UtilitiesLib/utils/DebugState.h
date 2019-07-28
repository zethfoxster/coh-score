#ifndef DEBUGSTATE_H
#define DEBUGSTATE_H

#include "cmdoldparse.h"

typedef struct DbgState {
	//Debug and Testing stuff

	F32		test1;  //a handful of floats to have around for off the cuff debugging
	F32		test2;
	F32		test3;
	F32		test4;
	F32		test5;
	F32		test6;
	F32		test7;
	F32		test8;
	F32		test9;
	F32		test10;
	int		testInt1;
	Vec3	testVec1;
	Vec3	testVec2;
	Vec3	testVec3;
	Vec3	testVec4;
	int		testDisplayValues;
	int		test1flicker;
	int		test2flicker;
	int		test3flicker;
	int		test4flicker;
	int		test5flicker;
	int		test6flicker;
	int		test7flicker;
	int		test8flicker;
	int		test9flicker;
	int		test10flicker;
} DbgState;

extern DbgState dbg_state;

typedef struct CmdList CmdList;
extern CmdList dbg_cmdlist;
extern Cmd dbg_cmds[];
int dbgCommandParse(CMDARGS);

void dbgOncePerFrame(void);

extern Cmd auto_cmds_UtilitiesLib[];


#endif