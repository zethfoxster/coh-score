/* File StringTable.h
 *	A string table is basically a buffer space that holds tightly packed string.  The table
 *	grows in large memory chunks whenever it runs out of space to store more strings.
 *
 *	Create and using a string table:
 *		First, call the createStringTable function to get a valid StringTable handle.  Then
 *		call the initStringTable function with the handle and the desired "chunkSize".  This
 *		size tells the string table how much memory to allocate everytime the string table
 *		runs out of space to hold the string being inserted.
 *
 *	Inserting strings into the string table:
 *		Call the strTableAddString function with a the handle of the table to add the string
 *		to and the string to be added.  The index of the string will be returned.
 *
 *	Making a string table indexable:
 *		Call the strTableSetMode function with the "Indexable" mode value before any strings
 *		are inserted.  If there are already some strings present in the table, attempts to
 *		switch into the "indexable" mode will fail.
 *
 */

#ifndef STRINGTABLE_H
#define STRINGTABLE_H

typedef struct StringTableImp StringTableImp;
typedef StringTableImp *StringTable;
typedef int (*StringProcessor)(char*);

/* Enum StringTableMode
 *	Defines several possible StashTable operation modes.
 *	
 *	Default:
 *		Makes the table non-indexable.
 *
 *	Indexable:
 *		Allow all the strings in the table to be accessed via some index.
 *		The index reflects the order of string insertion into the table and
 *		will be static over the lifetime of the StringTable.
 *
 *		Making the strings indexable requires that the string table use 
 *		additional memory to keep track of the index-to-string relationship.  
 *		Currently, this overhead is at 4 bytes per string.
 *
 *	NoRedundance: [This is implemented, but unused and untested -GG]
 *		Keep a stash table of the strings pointing to themselves, to prevent
 *		redundant adds.  Has the overhead of the stash table (currently
 *		no more than 193 bytes + 16 bytes per string).
 *		The flag and the stash table are never copied.
 */
typedef enum
{
	StrTableDefault =		0,
	Indexable =				(1 << 0),
	WideString =			(1 << 1),
//	NoRedundance =			(1 << 2), // this will not be copied
//	InSharedHeap =			(1 << 3), // this is apparently unused
} StringTableMode;


typedef struct _SimpleBufHandle *SimpleBufHandle;
typedef struct StashTableImp *StashTable;
typedef const struct StashTableImp *cStashTable;


// Constructor + Destructor
StringTable createStringTableInSharedHeap();
StringTable createStringTable();
void destroyStringTable(StringTable table);
StringTable strTableCreate(StringTableMode mode);
StringTable strTableCheckForSharedCopy(const char* pcStringSharedHashKey, bool* bFirstCaller);
StringTable strTableCopyIntoSharedHeap(StringTable table, const char* pcStringSharedHashKey, bool bAlreadyAreFirstCaller );
size_t strTableGetCopyTargetAllocSize(StringTable table);
StringTable strTableCopyToAllocatedSpace(StringTable table, void* pAllocatedSpace, size_t uiTotalSize );

void initStringTableEx(StringTable table, unsigned int chunkSize, const char* pFile, int iLine );
#define initStringTable(table, chunkSize) initStringTableEx(table, chunkSize, __FILE__, __LINE__)

void* strTableAddString(StringTable table, const void* str);
int strTableAddStringGetIndex(StringTable table, const void* str);
void strTableClear(StringTable table);

// String enumeration
int strTableGetStringCount(StringTable table);
char* strTableGetString(StringTable table, int i);
StashTable strTableCreateStringToIndexMap(StringTable table);
const char* strTableGetConstString(StringTable table, int i);
size_t strTableMemUsage(StringTable table);

// StringTable mode query/alteration
void strTableForEachString(StringTable table, StringProcessor processor);
StringTableMode strTableGetMode(StringTable table);
int strTableSetMode(StringTable table, StringTableMode mode);

void strTableLock(StringTable table);

// We can't remove strings, but we can record how many we would have removed and track how much memory is wasted because of it.
void strTableLogRemovalRequest(StringTable table, const char* pcStringToRemove);

int WriteStringTable(SimpleBufHandle file, StringTable table);
StringTable ReadStringTable(SimpleBufHandle file);

void printStringTableMemUsage();

#endif