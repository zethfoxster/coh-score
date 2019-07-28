/*\
 *
 *	structInternals.h - Copyright 2004 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	All the dirty details are here - beware!
 *
 */

#ifndef STRUCTINTERNALS_H
#define STRUCTINTERNALS_H

#include "file.h" // need weird FILE defines
#include "textparserUtils.h" // need DynOp enum, but can't forward declare enums
#include "textparser.h" // NUM_TOK_TYPE_TOKENS constant

typedef struct Packet Packet;

// access to textparser globals
extern int g_parseerr;
extern const char *parser_relpath;
extern const char *parser_structname;

// these are for getting info on fields in ParseTable that change depending on
// token type.  See interpretfield_f().
typedef enum ParseInfoField {
	ParamField,
	SubtableField,
} ParseInfoField;

typedef enum ParseInfoFieldUsage {
	// correct response for either field
	NoFieldUsage = 0,

	// param field
	SizeOfSubstruct,
	NumberOfElements,
	DefaultValue,
	EmbeddedStringLength,
	PointerToDefaultString,
	OffsetOfSizeField,
	SizeOfRawField,
	
	// subtable field
	StaticDefineList,
	PointerToSubtable,
	PointerToDynamicSubtable,
	PointerToLinkDictionary,
	PointerToDictionaryName,
} ParseInfoFieldUsage;

////////////////////////////////////////////////////////// Function prototypes for array and token handlers
typedef void (*preparse_f)(ParseTable tpi[], int column, void* structptr, TokenizerHandle tok);
typedef int (*parse_f)(TokenizerHandle tok, ParseTable tpi[], int column, void* structptr, int index, ParserTextCallback callback);
typedef void (*writetext_f)(FILE* out, ParseTable tpi[], int column, const void* structptr, int index, bool showname, int level, StructTypeField iOptionFlagsToMatch, StructTypeField iOptionFlagsToExclude);
typedef int (*writebin_f)(SimpleBufHandle file, SimpleBufHandle metafile, ParseTable tpi[], int column, void* structptr, int index, int* datasum, StructTypeField iOptionFlagsToMatch, StructTypeField iOptionFlagsToExclude);
typedef int (*readbin_f)(SimpleBufHandle file, SimpleBufHandle metafile, ParseTable tpi[], int column, void* structptr, int index, int* datasum);
typedef void (*initstruct_f)(ParseTable tpi[], int column, void* structptr, int index);
typedef void (*destroystruct_f)(ParseTable tpi[], int column, void* structptr, int index);
typedef void (*updatecrc_f)(ParseTable tpi[], int column, void* structptr, int index);
typedef int (*compare_f)(ParseTable tpi[], int column, void* lhs, void* rhs, int index);
typedef size_t (*memusage_f)(ParseTable tpi[], int column, void* structptr, int index);
typedef void (*copystruct_f)(ParseTable tpi[], int column, void* dest, void* src, int index, CustomMemoryAllocator memAllocator, void* customData);
typedef void (*copyfield_f)(ParseTable tpi[], int column, void* dest, void* src, int index, CustomMemoryAllocator memAllocator, void* customData, StructTypeField iOptionFlagsToMatch, StructTypeField iOptionFlagsToExclude);
typedef void (*senddiff_f)(Packet* pak, ParseTable tpi[], int column, void* oldstruct, void* newstruct, int index, bool sendAbsolute, bool forcePackAll, bool allowDiffs, StructTypeField iOptionFlagsToMatch, StructTypeField iOptionFlagsToExclude);
typedef void (*recvdiff_f)(Packet* pak, ParseTable tpi[], int column, void* structptr, int index, int absValues, void** pktidptr); // must deal with structptr == NULL by ignoring data
typedef void (*freepktids_f)(ParseTable tpi[], int column, void** pktidptr); 
typedef void (*endianswap_f)(ParseTable tpi[], int column, void* structptr, int index);
typedef void (*interp_f)(ParseTable tpi[], int column, void* structA, void* structB, void* destStruct, int index, F32 interpParam);
typedef void (*calcrate_f)(ParseTable tpi[], int column, void* structA, void* structB, void* destStruct, int index, F32 deltaTime);
typedef void (*integrate_f)(ParseTable tpi[], int column, void* valueStruct, void* rateStruct, void* destStruct, int index, F32 deltaTime);
typedef void (*calccyclic_f)(ParseTable tpi[], int column, void* valueStruct, void* ampStruct, void* freqStruct, void* cycleStruct, void* destStruct, int index, F32 fStartTime, F32 deltaTime);
typedef void (*applydynop_f)(ParseTable tpi[], int column, void* dstStruct, void* srcStruct, int index, DynOpType optype, const F32* values, U8 uiValuesSpecd, U32* seed);
typedef bool (*tosimple_f)(ParseTable tpi[], int column, void* structptr, int index, char* str, int str_size, bool prettyprint);
typedef bool (*fromsimple_f)(ParseTable tpi[], int column, void* structptr, int index, char* str);
typedef void (*calcoffset_f)(ParseTable tpi[], int column, size_t* size);
typedef size_t (*colsize_f)(ParseTable tpi[], int column);
typedef void (*addstringpool_f)(ParseTable tpi[], int column, void* structptr, int index, StructTypeField iOptionFlagsToMatch, StructTypeField iOptionFlagsToExclude);
typedef ParseInfoFieldUsage (*interpretfield_f)(ParseTable tpi[], int column, ParseInfoField field);

////////////////////////////////////////////////////////// Token array types
// g_arraytable provides array handler functions for each array type,
// these functions then call the base token handler to perform each operation
typedef struct TokenArrayInfo 
{
	initstruct_f	initstruct;
	destroystruct_f destroystruct;
	parse_f			parse;
	addstringpool_f addstringpool;
	writetext_f		writetext;
	writebin_f		writebin;
	readbin_f		readbin;
	senddiff_f		senddiff;
	recvdiff_f		recvdiff;
	freepktids_f	freepktids;
	tosimple_f		tosimple;
	fromsimple_f	fromsimple;
	updatecrc_f		updatecrc;
	compare_f		compare;
	memusage_f		memusage;
	calcoffset_f	calcoffset;
	colsize_f		colsize;
	copystruct_f	copystruct;
	copyfield_f		copyfield;
	endianswap_f	endianswap;
	interp_f		interp;
	calcrate_f		calcrate;
	integrate_f		integrate;
	calccyclic_f	calccyclic;
	applydynop_f	applydynop;
} TokenArrayInfo;
extern TokenArrayInfo g_arraytable[3];

// it is always safe to call any handler on any array type using this method, any stubbed
// out base token types will be ignored correctly
#define TOKARRAY_INFO(type) (g_arraytable[(type & TOK_EARRAY)? 2: (type & TOK_FIXED_ARRAY)? 1: 0])

/////////////////////////////////////////////////////////// Token base types
// g_infotable provides handler functions for each type of token,
// generally intended to by used by the array handler functions below
typedef struct TokenTypeInfo 
{
	U32				type;
	U32				storage_compatibility;
	char*			name_direct_single;			// REQUIRED, rest of names are optional
	char*			name_direct_fixedarray;
	char*			name_direct_earray;
	char*			name_indirect_single;
	char*			name_indirect_fixedarray;
	char*			name_indirect_earray;
	interpretfield_f interpretfield;			// REQUIRED, use ignore_interpretfield if not needed
	initstruct_f	initstruct;
	destroystruct_f destroystruct;
	preparse_f		preparse;
	parse_f			parse;
	addstringpool_f addstringpool;
	writetext_f		writetext;
	writebin_f		writebin;
	readbin_f		readbin;
	senddiff_f		senddiff;
	recvdiff_f		recvdiff;
	freepktids_f	freepktids;
	tosimple_f		tosimple;
	fromsimple_f	fromsimple;
	updatecrc_f		updatecrc;
	compare_f		compare;
	memusage_f		memusage;
	calcoffset_f	calcoffset;
	colsize_f		colsize;
	copystruct_f	copystruct;
	copyfield_f		copyfield;
	endianswap_f	endianswap;
	interp_f		interp;
	calcrate_f		calcrate;
	integrate_f		integrate;
	calccyclic_f	calccyclic;
	applydynop_f	applydynop;
} TokenTypeInfo;
extern TokenTypeInfo g_tokentable[NUM_TOK_TYPE_TOKENS];

// there is no guarantee that a particular token type will have a particular handler, generally
// you should use TOKARRAY_INFO instead
#define TYPE_INFO(type) (devassert(TOK_GET_TYPE(type) < NUM_TOK_TYPE_TOKENS), g_tokentable[TOK_GET_TYPE(type)])

// these are some sanity checks on parse tables in case users try to get crazy with token types
void TestParseTable(ParseTable pti[]);
#define TEST_PARSE_TABLE(pti) (isDevelopmentMode()? TestParseTable(pti): (void)0)

////////////////////////////////////////////////////////// FileList
// FileList is used by the parser to keep a record of files and correct
// file dates in the .bin files.  They are maintained as a 
// sorted EArray of file names and dates

#define FILELIST_SIG "Files2"

// how could my work be done without creating another file information structure?
typedef struct FileEntry {
	char path[MAX_PATH];
	__time32_t date;
	int seen;			// just used for compare ops
} FileEntry;

typedef struct StringPool StringPool;
typedef FileEntry** FileList;
typedef void (*FileListCallback)(char* path, __time32_t date);

void FileListCreate(FileList* list);
void FileListDestroy(FileList* list);
void FileListInsert(FileList* list, const char* path, __time32_t date); // ok to pass 0 for date
void FileListAddStrings(FileList *list, StringPool *sp);
int FileListRead(FileList* list, SimpleBufHandle file, StringPool *sp); // returns success
int FileListWrite(FileList* list, SimpleBufHandle file, StringPool *sp); // returns success
int FileListIsBinUpToDate(FileList* binlist, FileList *disklist); // Returns 1 if files in binlist are at least as new as those in disklist.  Returns 0 if files are in one list but not the other.
int FileListLength(FileList *list);
void FileListForEach(FileList *list, FileListCallback callback);
FileEntry* FileListFind(FileList* list, char* path);

// For creating a compact single-instance string pool

StringPool *StringPoolCreate();
int StringPoolAdd(StringPool *pool, const char *str);
bool StringPoolFind(StringPool *pool, const char *str, int *off);
int StringPoolSize(StringPool *pool);
const char *StringPoolGet(StringPool *pool, int off);
void StringPoolFree(StringPool *pool);
int StringPoolWrite(StringPool *pool, SimpleBufHandle file);
int StringPoolRead(StringPool **pool, SimpleBufHandle file);

#endif // STRUCTINTERNALS_H