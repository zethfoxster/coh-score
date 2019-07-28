// scriptvars.h - provides a random variable system for use with story arc scripts

#ifndef __SCRIPTVARS_H
#define __SCRIPTVARS_H

#include "stdtypes.h"
typedef struct StructParams StructParams;
typedef struct MultiMessageStore MultiMessageStore;
#define TokenizerParams StructParams

////////////////////////////////////////////////////////////// Vars
// ScriptVars is just a method to hook up random variables to scripts.
// Since they have to be randomized at runtime, they are stored as 
// unparsed strings.
//
// VarsScope should be populated by the Tokenizer with TOK_UNPARSED
// statements (unparsed).  Generally, this is just a list of strings that 
// came after the token 'var' or 'vargroup'.   ScriptVarsLookup will 
// try to replace a string with a variable value, looking through higher
// scopes as appropriate.  If a match
// isn't found, the string is returned unmodified.
// 
// The user of the function is expected to set up the higherscope 
// variable within ScriptVarsScope if they want the lookup function 
// to trace through.  ScriptVarsLookup is passed a seed value for 
// the random generator, so the user can set up this value and 
// consistently get the same random variables.  
//
// ScriptVarsVerifySyntax will printf various errors in the formatting 
// of scriptvars.
// 
typedef struct ScriptVarsScope {
	TokenizerParams** randomgroups;		// HACK - storyarcutil relies on the placement of this field
	TokenizerParams** vars;
	struct ScriptVarsScope* higherscope;		// hook up to parent in vars tree, or set to NULL
} ScriptVarsScope;

const char* ScriptVarsLookup(const ScriptVarsScope* currentscope, const char* str, unsigned int rseed); 
	// looks for match through hierarchy, chains variables for further lookups, may return <str> if no matches,
	// uses rseed to choose from random lists

int ScriptVarsLookupInt(const ScriptVarsScope* currentscope, const char* str, unsigned int rseed);
	// as above, but returns atoi result.  0 on error or NULL string


// some functions for debugging
typedef int (*VerifyVarNameFunc)(char* varname);
void ScriptVarsVerifySyntax(ScriptVarsScope* currentscope, VerifyVarNameFunc varnamefunc, MultiMessageStore *mms, const char* filename);
	// gives fatal error messages if there are any problems with the syntax of the vars
	// - varnamefunc function is optional.  It will be passed all variable names and can return 0 to signal an error

void ScriptVarsLookupComplete(const ScriptVarsScope* currentscope, const char* str, char*** resultlist);
void ScriptVarsFreeList(char*** resultlist);
	// returns a complete list of every value str could have given the current scope,
	// call with the address of a char** pointer, use eaSize to get size of list
	// use ScriptVarsFreeList to delete list after done

// optionally you can use these defines to add a .vs field
// to your structure and parse it
#define SCRIPTVARS_STD_FIELD ScriptVarsScope vs;
#define SCRIPTVARS_STD_PARSE(structname) \
	{ "var",		TOK_UNPARSED(structname,vs.vars) }, \
	{ "vargroup",	TOK_UNPARSED(structname,vs.randomgroups) },


////////////////////////////////////////////////////////// ScriptVarTable
// ScriptVarTable let you link together several ScriptVarScope's
// 
// This functionality probably could have been folded into ScriptVarsScope, but 
// I decided against it because the usage model of ScriptVarsScope and ScriptVarsTable
// is different.  ScriptVarsScope is static data, and ScriptVarTable is intended
// to quickly link together a couple of scopes directly before performing lookups.
//
// No heap operations are done when assembling a ScriptVarsTable
// UNLESS you use SV_COPYVALUE - most vars tables that stick around for a while have
// some copied strings now, and need a ScriptVarsTableClear() call
//
// vars placed directly in the table are evaluated first, then each scope in
// reverse order from when they were added.  A VarsTable holds its own random seed.
// Use xxSetSeed before performing a lookup

#define MAX_VARSTABLE_SCOPES	10	// doesn't matter much, just don't want to be resizing tables
#define MAX_VARSTABLE_VARS		40	// has to accommodate textparams right now

typedef struct ScriptVarsTable {
	const ScriptVarsScope* scopes[MAX_VARSTABLE_SCOPES];
	int					numscopes;
	const char*			keys[MAX_VARSTABLE_VARS];
	const char*			values[MAX_VARSTABLE_VARS];
	int					numvars;
	unsigned int		seed;
	char				types[MAX_VARSTABLE_VARS];	// needed for entity* and contactdef* support
	U8					flags[MAX_VARSTABLE_VARS];	// if set, the values are copied
} ScriptVarsTable;

void ScriptVarsTableClear(ScriptVarsTable* table); // you can no longer memset to 0 if you have any copied strings
void ScriptVarsTableCopy(ScriptVarsTable* target, ScriptVarsTable* source);
void ScriptVarsTableScopeCopy(ScriptVarsTable* target, ScriptVarsTable* source);
void ScriptVarsTablePushScope(ScriptVarsTable* table, const ScriptVarsScope* scope);
const ScriptVarsScope* ScriptVarsTablePopScope(ScriptVarsTable* table);
void ScriptVarsTablePushVar(ScriptVarsTable* table, char* key, const char* value);
void ScriptVarsTablePopVar(ScriptVarsTable* table);
void ScriptVarsTableClearVars(ScriptVarsTable* table);
void ScriptVarsTableSetSeed(ScriptVarsTable* table, unsigned int rseed);

char* VarName(TokenizerParams* var);
char* VarGroupName(TokenizerParams* var);
int VarNumValues(TokenizerParams* var);
char* VarValue(TokenizerParams* var, int index);

// be careful - copying values means you'll have to clean up the vars table instead of just clearing it
// you can use ScriptVarsScopeCopy if you don't have to deal with the rest of the table
#define SV_COPYVALUE	1
void ScriptVarsTablePushVarEx(ScriptVarsTable* table, char* key, const void* value, char type, U8 flags);

const char* ScriptVarsTableLookup(ScriptVarsTable* table, const char* str); 
int ScriptVarsTableLookupInt(ScriptVarsTable* table, const char* str);
const void* ScriptVarsTableLookupTyped(ScriptVarsTable* table, const char* str, char* type);

#endif // __SCRIPTVARS_H
