/****************************************************************************
	rendercgfx.h
	
	Main header for CGFX support.
	
	Author: LStLezin, 01/2009
****************************************************************************/
#ifndef __rendercgfx_h__
#define __rendercgfx_h__

// For incomplete work / open questions, search on comments marked "CGFX_TBD"

typedef enum {
		// Everything is dandy.
	kCgFxError_NONE,
		// Can't run this effect on the current hardware.
	kCgFxError_NoValidTechnique,
		// The effect instance (material) doesn't match the effect
		// format (probably a typo in the param names).
	kCgFxError_BadInstanceFormat,
		// We don't know how to handle a parameter's data type.
	kCgFxError_UnsupportedDataType,
		// Bad Cg format
	kCgFxError_BadEffectFormat,
		// General bad thing happened.
	kCgFxError_GENERAL_FAILURE
} tCgFxError;


#endif // __rendercgfx_h__