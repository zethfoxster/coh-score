/* File Breakpoint.h
 *	This module provides an interface to programmatically set data breakpoints on modern CPUs.
 *
 *	Future enhancements:
 *		It may be possible to integrate program symbol info to allow setting breakpoints on a
 *		source line level.
 *	
 */

#ifndef BREAKPOINT_H
#define BREAKPOINT_H

// There are only 4 hardware breakpoints available.
typedef enum
{
	BreakPoint1,
	BreakPoint2,
	BreakPoint3,
	BreakPoint4
} BreakPointID;

// Valid values for DebugControlRegister::rwX fields
typedef enum
{
	DB_BREAK_ON_EXECUTE	=			0,
	DB_BREAK_ON_WRITE =				1,
	DB_BREAK_ON_READWRITEFETCH =	2,
	DB_BREAK_ON_READWRITE =			3,
} BreakCondition;

void debugSetDataBreakPoint(BreakPointID breakpoint, void* dataAddress);
void debugSetDataBreakPointEx(BreakPointID breakpoint, void* dataAddress, BreakCondition condition);
void debugClearDataBreakPoint(BreakPointID breakpoint);

void testBreakpoint(void);
#endif