#ifndef _FPMACROS_H
#define _FPMACROS_H

#include <float.h>

extern U32 oldControlState;
#if defined(_M_X64) || defined(BEACONIZER)
#	define SET_FP_CONTROL_WORD_DEFAULT 	_controlfp_s(&oldControlState, _RC_NEAR | _MCW_EM, _MCW_RC | _MCW_EM);
#else
#	define SET_FP_CONTROL_WORD_DEFAULT 	_controlfp_s(&oldControlState,_RC_NEAR | _PC_24 | _MCW_EM, _MCW_RC | _MCW_PC | _MCW_EM);
#endif
	// MAK - 64-bit doesn't support setting different precisions, and _controlfp actually asserts if
	// you pass _MCW_PC.  I'm just changing things to 64-bit precision for either target, if this
	// causes a performance problem, we can change back to 32-bit precision for x86 targets

	// 10-25-2006 AB - going back to 24 bit precision. caused a bunch of UI issues. (COH-2373)

	// 08-14-2008 GG - going platform dependent for x64

	// JP - 24-bit precision may hamper consistency of beaconizer checksums (unverified)
#endif

