/*
 *
 *	structDefines.h - Copyright 2004 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	Provides DefineXxx and StaticDefineXxx systems for
 *  use with textparser, or just independently.  Both
 *  systems allow string substitution.  DefineXxx is based 
 *  on a hash table, StaticDefineXxx is just a simple
 *  static table that can be defined as a constant in code.
 *
 */

#ifndef STRUCTDEFINES_H
#define STRUCTDEFINES_H

C_DECLARATIONS_BEGIN

//////////////////////////////////////////////////////// Defines
// DefineXxx is based on a hash table.  Just use the 
// interface functions to do everything.
typedef struct DefineContext DefineContext;
typedef struct DefineList {
	char* key;
	char* value;
	char* display;
} DefineList;
typedef struct DefineIntList {
	char* key;
	intptr_t value;
	char* display;
} DefineIntList;

DefineContext* DefineCreate();
DefineContext* DefineCreateFromList(DefineList defs[]);
DefineContext* DefineCreateFromIntList(DefineIntList defs[]);
void DefineDestroy(DefineContext* context);
void DefineAdd(DefineContext* context, const char* key, const char* value); // strings copied
void DefineAddList(DefineContext* lhs, DefineList defs[]);
void DefineAddIntList(DefineContext* lhs, DefineIntList defs[]);
const char* DefineLookup(DefineContext* context, const char* key);
void DefineSetHigherLevel(DefineContext* lower, DefineContext* higher); // lookups on lower will default to higher if not found


///////////////////////////////////////////////////// StaticDefine
// New defines system for per-token defines.  Only uses a static definition.
// Lists with different types may be used interchangeably, but
// must be declared with DEFINE_Xxx type macros, as in:
//
//	StaticDefineInt mydefines = {
//		DEFINE_INT
//		{ "fFlag",	1 },
//		...
//		DEFINE_END
//	};

// private stuff..
#define DM_END 0
#define DM_INT 1
#define DM_STRING 2
#define DM_DYNLIST 3
#define DM_NOTYPE 4
#define DM_TAILLIST 5 
#define DM_INTDEFINESTACK 6

// structs to hold defines
typedef struct StaticDefine {
	char* key;
	char* value;
	char* display;
} StaticDefine;

typedef struct StaticDefineInt {
	char* key;
	intptr_t value;
	char* display;
} StaticDefineInt;

// use these to start and end a list
#define DEFINE_STRING	{ (char*)U32_TO_PTR(DM_STRING), 0 },
#define DEFINE_INT		{ (char*)U32_TO_PTR(DM_INT), 0 },
#define DEFINE_END		{ (char*)U32_TO_PTR(DM_END), 0 },

// use this to embed a dynamic define list in a StaticDefine or StaticDefineInt
#define DEFINE_EMBEDDYNAMIC(pDefineContext) { U32_TO_PTR(DM_DYNLIST), (char*)(&pDefineContext) }, 
#define DEFINE_EMBEDDYNAMIC_INT(pDefineContext) { U32_TO_PTR(DM_DYNLIST), (intptr_t)(&pDefineContext) }, 

// use this to just wrap a dynamic define
#define STATIC_DEFINE_WRAPPER(StaticDefineName, pDefineContext) \
	StaticDefine StaticDefineName[] = { \
		DEFINE_EMBEDDYNAMIC(pDefineContext) \
		DEFINE_END \
	}

// all lookups are made directly from static data
const char* StaticDefineLookup(StaticDefine* list, const char* key);
const char* StaticDefineRevLookup(StaticDefine* list, char* value);
const char* StaticDefineIntRevLookup(StaticDefineInt* list, int value);
const char* StaticDefineIntRevLookupDisplay(StaticDefineInt* list, int value);

int StaticDefineIntLookupInt(StaticDefineInt* list, const char *key); // sorry for the naming, i've come too late to this battle, ugh
// returns value if found, otherwise atoi(key)

static INLINEDBG const char* StaticDefineIntLookup(StaticDefineInt* list, const char* key) 
{
	return StaticDefineLookup((StaticDefine*)list, key);
}

static INLINEDBG int StaticDefineIntGetInt(StaticDefineInt* list, const char* key)
{
	const char* res = StaticDefineIntLookup(list, key);
	if (!res) return -1;
	return atoi(res);
}

void UpdateCrcFromDefineList(DefineContext* defines);

C_DECLARATIONS_END

#endif	// STRUCTDEFINES_H
