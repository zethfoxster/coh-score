#ifndef __texture_tools_h__
#define __texture_tools_h__

#ifdef __cplusplus
	extern "C" {
#endif

#include "gettex.h"

	// Call this to prepare for a new operation
void textureTools_Start( void );

	// Call this to configure the processor
bool textureTools_AddOption( tGetTexOpt optType, const char* argData );

	// Perform texture processing
bool textureTools_Finish( void );

	// Returns NULL if no error was detected; otherwise, an error string.
const char* textureTools_GetLastError( void );

#ifdef __cplusplus
	}
#endif

#endif // __texture_tools_h__
