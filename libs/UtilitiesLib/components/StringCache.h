#ifndef STRINGCACHE_H
#define STRINGCACHE_H

#include "stdtypes.h" // to make VS2005 not complain about stricmp

C_DECLARATIONS_BEGIN

typedef const char *constCharPtr;

void stringReorder(void);
int numIndexedStringsInStringTable(void);
void stringCachePreAlloc(int size);

// Generic string table
const char* allocAddString( const char * s );
const char* allocFindString(const char * s);
const char* allocAddString_checked(const char *s); // passes NULLs through, devassert if string isn't already cached

// Shared memory string table
void sharedStringReserve(int count);
const char* allocAddSharedString(const char * s);

// Indexed string table (only used by FX - for network communication)
const char* allocAddIndexedString( const char * s );
int stringToReference(const char* str);
const char* stringFromReference(int ref);

// First tries the string cache for a simple cmp, if that fails, does a stricmp...
// AB: this function is stupid stupid stupid
// __forceinline static bool stringCacheCompareString(const char* pcKnownStringPtr, const char* pcLookupString)
// {
// 	const char* pcLookedUp = allocFindString(pcLookupString);
// 	if (!pcLookedUp)
// 	{
// 		return (stricmp(pcKnownStringPtr, pcLookupString) == 0);
// 	}
// 	return pcKnownStringPtr == pcLookedUp;
// }

C_DECLARATIONS_END

#endif // STRINGCACHE_H
