#ifndef _XBOX

#include "Breakpoint.h"

#include <stdio.h>
#include <assert.h>
#include <stddef.h>
#include "wininclude.h"



//-------------------------------------------------------------------------------
// Debug control register
//-------------------------------------------------------------------------------
/* Debug control register
*	Break down of register DR7 (debug register 7).
*	Assumes that the compiler will align first bit field as the least significant bit.
*/
typedef struct{
	unsigned int l0		: 1;	// bit 0		// local breakpoint enable
	unsigned int g0		: 1;					// global breakpoint enable
	unsigned int l1		: 1;	// bit 2
	unsigned int g1		: 1;
	unsigned int l2		: 1;	// bit 4
	unsigned int g2		: 1;
	unsigned int l3		: 1;	// bit 6
	unsigned int g3		: 1;
	unsigned int le		: 1;	// bit 8		// local exact breakpoint (break on the exact instruction)
	unsigned int ge		: 1;
	unsigned int unused : 3;
	unsigned int gd		: 1;	// bit 13		// general detection (break when debug register changes)
	unsigned int unused2: 2;
	unsigned int rw0	: 2;	// bit 16
	unsigned int len0	: 2;
	unsigned int rw1	: 2;	// bit 20
	unsigned int len1	: 2;
	unsigned int rw2	: 2;	// bit 24
	unsigned int len2	: 2;
	unsigned int rw3	: 2;	// bit 28
	unsigned int len3	: 2;
} DebugControlRegister;

// Valid values for DebugControlRegister::lenX fields
#define DB_DATA_LEN_1	0
#define DB_DATA_LEN_2	1
#define DB_DATA_LEN_4	3
#define DB_DATA_LEN(x)	DB_DATA_LEN_##x


#define DB_BREAK_ON(x)	DB_BREAK_ON_##x

// General bit field mutator macros
#define DB_SET_BITS(bitarray, bitloc, val) (*(int*)bitarray |= (val << bitloc))
#define DB_CLEAR_BITS(bitarray, bitloc, val) (*(int*)bitarray &= ~(val << bitloc))

// Debug control register bit locators
#define DB_GET_LOCAL_ENABLE_BIT(breakpoint) (breakpoint << 1)
#define DB_GET_GLOBAL_ENABLE_BIT(breakpoint) ((breakpoint << 1) + 1)
#define DB_GET_LOCAL_EXACT_BIT() (8)
#define DB_GET_GLOBAL_EXACT_BIT() (9)
#define DB_GET_GENERAL_DETECTION_BIT() (13)
#define DB_GET_DATA_LEN_BIT(breakpoint) ((breakpoint << 1) + 18)
#define DB_GET_BREAK_CONDITION_BIT(breakpoint) ((breakpoint << 1) + 16)



//-------------------------------------------------------------------------------
// Debug Status Register
//-------------------------------------------------------------------------------
/* Debug Status Register
*	Break down of register DR6 (debug register 6).
*	Assumes that the compiler will align first bit field as the least significant bit.
*/
typedef struct{
	unsigned int b0		: 1;	// Break point condition detected bits
	unsigned int b1		: 1;
	unsigned int b2		: 1;
	unsigned int b3		: 1;
	unsigned int unused : 9;
	unsigned int bd		: 1;	// Debug register access.  Next instruction accesses a debug register.
	unsigned int bs		: 1;	// Single step debug exception.
	unsigned int bt		: 1;	// Task switch debug exception.
} DebugStatusRegister;



//-------------------------------------------------------------------------------
// Setting Debug registers
//-------------------------------------------------------------------------------
unsigned int debugRegisterOffsets[] =
{
	offsetof(CONTEXT, Dr0),	// for BreakPoint1
	offsetof(CONTEXT, Dr1), // for BreakPoint2
	offsetof(CONTEXT, Dr2), // for BreakPoint3
	offsetof(CONTEXT, Dr3), // for BreakPoint4
};

/* Function debugSetDebugRegister
*	Sets the register associated with the specified breakpoint to the specified data.
*	
*/
static INLINEDBG void debugSetRegister(CONTEXT* context, BreakPointID breakpoint, uintptr_t data)
{
	// cast mania
	*(uintptr_t*)(((unsigned char*)context) + debugRegisterOffsets[breakpoint]) = data;
}



//-------------------------------------------------------------------------------
// Setting/Clearing breakpoints
//-------------------------------------------------------------------------------
void debugSetDataBreakPoint(BreakPointID breakpoint, void* dataAddress)
{
	CONTEXT threadContext = {0};

	DebugControlRegister* control;
	DebugStatusRegister* status;

	assert(breakpoint >= BreakPoint1 && breakpoint <= BreakPoint4);

	// Grab the current state of the debug registers
	threadContext.ContextFlags = CONTEXT_DEBUG_REGISTERS;
	GetThreadContext(GetCurrentThread(), &threadContext);
	control = (DebugControlRegister*)&(threadContext).Dr7;
	status = (DebugStatusRegister*)&(threadContext).Dr6;

	DB_SET_BITS(control, DB_GET_LOCAL_ENABLE_BIT(breakpoint),		1);					// Enable breakpoint
	DB_SET_BITS(control, DB_GET_DATA_LEN_BIT(breakpoint),			DB_DATA_LEN_4);		// Always watch 4 bytes
	DB_SET_BITS(control, DB_GET_BREAK_CONDITION_BIT(breakpoint),	DB_BREAK_ON_WRITE);	// detect data write
	debugSetRegister(&threadContext, breakpoint, (uintptr_t)dataAddress);				// detection address


	SetThreadContext(GetCurrentThread(), &threadContext);
}

void debugSetDataBreakPointEx(BreakPointID breakpoint, void* dataAddress, BreakCondition condition)
{
	CONTEXT threadContext = {0};

	DebugControlRegister* control;
	DebugStatusRegister* status;

	assert(breakpoint >= BreakPoint1 && breakpoint <= BreakPoint4);

	// Grab the current state of the debug registers
	threadContext.ContextFlags = CONTEXT_DEBUG_REGISTERS;
	GetThreadContext(GetCurrentThread(), &threadContext);
	control = (DebugControlRegister*)&(threadContext).Dr7;
	status = (DebugStatusRegister*)&(threadContext).Dr6;

	DB_SET_BITS(control, DB_GET_LOCAL_ENABLE_BIT(breakpoint),		1);					// Enable breakpoint
	DB_SET_BITS(control, DB_GET_DATA_LEN_BIT(breakpoint),			DB_DATA_LEN_4);		// Always watch 4 bytes
	DB_SET_BITS(control, DB_GET_BREAK_CONDITION_BIT(breakpoint),	condition);	// detect data write
	debugSetRegister(&threadContext, breakpoint, (uintptr_t)dataAddress);				// detection address


	SetThreadContext(GetCurrentThread(), &threadContext);
}


void debugClearDataBreakPoint(BreakPointID breakpoint)
{
	CONTEXT threadContext = {0};

	DebugControlRegister* control;
	DebugStatusRegister* status;

	assert(breakpoint >= BreakPoint1 && breakpoint <= BreakPoint4);

	// Grab the current state of the debug registers
	threadContext.ContextFlags = CONTEXT_DEBUG_REGISTERS;
	GetThreadContext(GetCurrentThread(), &threadContext);
	control = (DebugControlRegister*)&(threadContext).Dr7;
	status = (DebugStatusRegister*)&(threadContext).Dr6;

	DB_CLEAR_BITS(control, DB_GET_LOCAL_ENABLE_BIT(breakpoint),		1);		// Disable breakpoint
	DB_CLEAR_BITS(control, DB_GET_DATA_LEN_BIT(breakpoint),			3);		// Clear data length field
	DB_CLEAR_BITS(control, DB_GET_BREAK_CONDITION_BIT(breakpoint),	3);		// Clear break condition field
	debugSetRegister(&threadContext, breakpoint, 0);							// Clear detection address

	SetThreadContext(GetCurrentThread(), &threadContext);
}



//-------------------------------------------------------------------------------
// Test program
//-------------------------------------------------------------------------------
void testBreakpoint(void)
{

	char a;

	debugSetDataBreakPoint(BreakPoint1, &a);

	printf("hello world\n");
	printf("hello again\n");
	

	printf("I wonder...\n");
	printf("where \"a\"...\n");

	a = 1;

	printf("is going to...\n");
	printf("get modified...\n");

	printf("Only if...\n");
	printf("there is some way to tell...\n");

	printf("my life would be be...\n");
	printf("so much easier\n");

	if(a)
	{
		printf("read a\n");
	}
}

#endif