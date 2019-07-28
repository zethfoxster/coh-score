// serialize.h - provides simple binary serialization functions for storing structs
// these functions allow for a simple binary format: hierarchical, self describing,
// with traversal.  64 bytes overhead per struct

#ifndef __SERIALIZE_H
#define __SERIALIZE_H

#include <stdio.h>
#include "endian.h"

///////////////////////////////////////// simple buffer files - just preloading entire file
// SimpleBuf functions just let you treat a file read to and from a large
// memory buffer as a normal file.  Actual file I/O happens on Open on a 
// Read file and Close on a Write file.  The semantics of all functions are
// the same as standard lib functions

typedef struct _SimpleBufHandle *SimpleBufHandle;

SimpleBufHandle SimpleBufOpenWrite(const char* filename, int forcewrite);
SimpleBufHandle SimpleBufOpenRead(const char* filename);
void SimpleBufClose(SimpleBufHandle handle);
int SimpleBufWrite(const void* data, int size, SimpleBufHandle handle);
int SimpleBufRead(void* data, int size, SimpleBufHandle handle);
int SimpleBufSeek(SimpleBufHandle handle, long offset, int origin);
int SimpleBufTell(SimpleBufHandle handle);

// extra non file-io functions to avoid double mallocs
void *SimpleBufGetData(SimpleBufHandle handle,unsigned int *len);
SimpleBufHandle SimpleBufSetData(void *data,unsigned int len);
void SimpleBufGrow(SimpleBufHandle handle, unsigned int len); // grow to at least current size + len, to prepare for lots of little writes

// endian-aware i/o - always assums that file should be little-endian
static INLINEDBG int SimpleBufWriteU16(U16 val, SimpleBufHandle handle) { U16 x = endianSwapIfBig(U16, val); return SimpleBufWrite(&x, sizeof(U16), handle); }
static INLINEDBG int SimpleBufWriteU32(U32 val, SimpleBufHandle handle) { U32 x = endianSwapIfBig(U32, val); return SimpleBufWrite(&x, sizeof(U32), handle); }
static INLINEDBG int SimpleBufWriteU64(U64 val, SimpleBufHandle handle) { U64 x = endianSwapIfBig(U64, val); return SimpleBufWrite(&x, sizeof(U64), handle); }
static INLINEDBG int SimpleBufWriteF32(F32 val, SimpleBufHandle handle) { F32 x = endianSwapIfBig(F32, val); return SimpleBufWrite(&x, sizeof(F32), handle); }

static INLINEDBG int SimpleBufReadU16(U16* val, SimpleBufHandle handle) { U16 x; int ret = SimpleBufRead(&x, sizeof(U16), handle); *val = endianSwapIfBig(U16, x); return ret; }
static INLINEDBG int SimpleBufReadU32(U32* val, SimpleBufHandle handle) { U32 x; int ret = SimpleBufRead(&x, sizeof(U32), handle); *val = endianSwapIfBig(U32, x); return ret; }
static INLINEDBG int SimpleBufReadU64(U64* val, SimpleBufHandle handle) { U64 x; int ret = SimpleBufRead(&x, sizeof(U64), handle); *val = endianSwapIfBig(U64, x); return ret; }
static INLINEDBG int SimpleBufReadF32(F32* val, SimpleBufHandle handle) { F32 x; int ret = SimpleBufRead(&x, sizeof(F32), handle); *val = endianSwapIfBig(F32, x); return ret; }

////////////////////////////////////// structured files
// Serialize functions encapsulate a structured binary data file.
// Data is stored in a series of named structure segments.  Segments
// may be included in each other heirarchically.  Each Struct is
// a Header followed by Data of any length.  Data length is recorded
// in the Header.

#define MAX_FILETYPE_LEN	4096
#define MAX_STRUCTNAME_LEN	4096

// read or write
void SerializeClose(SimpleBufHandle file);

// pascal strings
int WritePascalString(SimpleBufHandle file, const char* str); // write a string to a file, with padding for DWORD lines, returns bytes written
int ReadPascalString(SimpleBufHandle file, char* str, int strsize); // read a corresponding string, returns bytes read

// write functions
SimpleBufHandle SerializeWriteOpen(const char* filename, const char* filetype, int build);		
int SerializeWriteStruct(SimpleBufHandle sfile, char* structname, int size, void* structptr);	// returns length written
int SerializeWriteHeader(SimpleBufHandle sfile, char* structname, int size, long* loc);		// returns length written
int SerializeWriteData(SimpleBufHandle sfile, int size, void* dataptr);				// returns length written
int SerializePatchHeader(SimpleBufHandle sfile, int size, long loc);					// use loc returned by SerializeWriteHeader

// read functions
SimpleBufHandle SerializeReadOpen(const char* filename, const char* filetype, int build, int ignore_crc_difference);
int SerializeNextStruct(SimpleBufHandle sfile, char* structname, int namesize, int* size); // loads name, size.  returns success
int SerializeSkipStruct(SimpleBufHandle sfile);			// returns length skipped
int SerializeReadStruct(SimpleBufHandle sfile, char* structname, int size, void* structptr);	// returns length read
int SerializeReadHeader(SimpleBufHandle sfile, char* structname, int namesize, int* size); // loads name, size. returns success
int SerializeReadData(SimpleBufHandle sfile, int size, void* dataptr);					// returns length read

#endif // __SERIALIZE_H