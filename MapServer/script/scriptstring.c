/*\
 *
 *	scriptstring.c - Copyright 2004 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	Implementation for the basic string manipulation functions for script.
 *	Strings are sort of garbage collected, any strings allocated during a tick
 *	are thrown away at the end of the tick.
 *
 *	We also do the number/string conversion and script var hooks here
 *
 */

#include "script.h"
#include "scriptvars.h"
#include "scriptengine.h"
#include "earray.h"
#include "mathutil.h"
#include "limits.h"
#include "contact.h"
#include "MessageStoreUtil.h"
#include "storyarcutil.h"
#include "hashFunctions.h"

Entity* EntTeamInternal(TEAM team, int index, int* num);

// *********************************************************************************
//  Temp string memory allocator
// *********************************************************************************

typedef struct StringBlock
{
	char* head;
	char* nextavail;
	int size;
	int remain;
} StringBlock;

static StringBlock** blocks = 0;
static StringBlock* curblock = 0;
static int cursize = 1000;

static void AllocBlock(void)
{
	StringBlock* newblock = calloc(sizeof(StringBlock), 1);
	if (!blocks) eaCreate(&blocks);
	eaPush(&blocks, newblock);
	curblock = newblock;

	curblock->nextavail = curblock->head = calloc(cursize, 1);
	curblock->size = curblock->remain = cursize;
}

char* StringAlloc(int len)
{
	char* ret;
	len++;		// assume they weren't including the terminator
	
	// if we can't fit in current block
	if (!curblock || curblock->remain < len)
	{
		if (cursize < len) cursize = len;
		AllocBlock();
	}

	// alloc the string in the current block
	ret = curblock->nextavail;
	curblock->nextavail += len;
	curblock->remain -= len;
	return ret;
}

char* StringDupe(const char* str)
{
	int len = strlen(str);
	char* result = StringAlloc(len);
	strcpy(result, str);
	return result;
}

void FreeAllTempStrings(void)
{
	// if we had multiple blocks, condense them
	if (eaSize(&blocks) >= 2)
	{
		int i, n = eaSize(&blocks);
		cursize = 0;
		for (i = 0; i < n; i++)
		{
			cursize += blocks[i]->size;
			free(blocks[i]->head);
			free(blocks[i]);
		}
		eaSetSize(&blocks, 0);
		AllocBlock();
	}

	// reset the pointers
	if (curblock)
	{
		curblock->nextavail = curblock->head;
		curblock->remain = curblock->size;
	}
}


// *********************************************************************************
//  Script var hooksants pop caps on those to be
// *********************************************************************************

int VarIsEmpty(VARIABLE var)
{
	var = GetEnvironmentVar(g_currentScript, var);
	return !(var && var[0]);
}

void VarSetEmpty(VARIABLE var)
{
	SetEnvironmentVar(g_currentScript, var, NULL);
}

STRING VarGet(VARIABLE var)
{
	var = GetEnvironmentVar(g_currentScript, var);
	return var ? var: "";
}

void VarSet(VARIABLE var, STRING value)
{
	if (value != NULL)
		if (value[0] == 0) 
			value = NULL;
	SetEnvironmentVar(g_currentScript, var, value);
}

void VarSetHidden(VARIABLE var, STRING value)
{
	int flags = 0;
	if (value != NULL)
		if (value[0] == 0) 
			value = NULL;
	SetEnvironmentVar(g_currentScript, var, value);
	if (!stashFindInt(g_currentScript->varsFlags, var, &flags))
	{
		flags = 0;
	}
	flags |= SP_DONTDISPLAYDEBUG;
	stashAddInt(g_currentScript->varsFlags, var, flags, false);
}


// weird semantics.. it's ok if you use either a string literal here or the name of a var
int VarEqual(VARIABLE var1, VARIABLE var2)
{
	const char* hash1 = GetEnvironmentVar(g_currentScript, var1);
	const char* hash2 = GetEnvironmentVar(g_currentScript, var2);
	if (hash1) var1 = hash1;
	if (hash2) var2 = hash2;
	return StringEqual(var1, var2);
}

int VarIsTrue(VARIABLE var)
{
	var = GetEnvironmentVar(g_currentScript, var);
	if (!var) return 0;
	return stricmp(var, "True") == 0;
}

int VarIsFalse(VARIABLE var)
{
	var = GetEnvironmentVar(g_currentScript, var);
	if (!var) return 1;
	return stricmp(var, "True") != 0;
}

void VarSetTrue(VARIABLE var)
{
	SetEnvironmentVar(g_currentScript, var, "True");
}

void VarSetFalse(VARIABLE var)
{
	SetEnvironmentVar(g_currentScript, var, "False");
}

NUMBER VarGetNumber(VARIABLE var)
{
	var = GetEnvironmentVar(g_currentScript, var);
	if (!var) return 0;
	return atoi(var);
}

void VarSetNumber(VARIABLE var, NUMBER value)
{
	char buf[100];
	sprintf(buf, "%i", value);
	SetEnvironmentVar(g_currentScript, var, buf);
}

void VarSetNumberHidden(VARIABLE var, NUMBER value)
{
	int flags = 0;
	char buf[100];

	sprintf(buf, "%i", value);
	SetEnvironmentVar(g_currentScript, var, buf);

	if (!stashFindInt(g_currentScript->varsFlags, var, &flags))
	{
		flags = 0;
	}
	flags |= SP_DONTDISPLAYDEBUG;
	stashAddInt(g_currentScript->varsFlags, var, flags, false);
}


FRACTION VarGetFraction(VARIABLE var)
{
	var = GetEnvironmentVar(g_currentScript, var);
	if (!var) return 0.0;
	return atof(var);
}

void VarSetFraction(VARIABLE var, FRACTION value)
{
	char buf[100];
	sprintf(buf, "%f", value);
	SetEnvironmentVar(g_currentScript, var, buf);
}

void VarSetFractionHidden(VARIABLE var, FRACTION value)
{
	char buf[100];
	int flags = 0;

	sprintf(buf, "%f", value);
	SetEnvironmentVar(g_currentScript, var, buf);

	if (!stashFindInt(g_currentScript->varsFlags, var, &flags))
	{
		flags = 0;
	}
	flags |= SP_DONTDISPLAYDEBUG;
	stashAddInt(g_currentScript->varsFlags, var, flags, false);
}

NUMBER VarGetArrayCount(VARIABLE var)
{ 
	int count = 0;
	char *pTok = NULL;
	char *sCopy;
 
	if (VarIsEmpty(var))
		return 0;

	var = GetEnvironmentVar(g_currentScript, var);
	if (!var) 
		return 0;
 	
	sCopy = StringDupe(var);
	pTok = strtok(sCopy, ";"); 
	while (pTok != NULL)
	{
		count++;
		pTok = strtok(NULL, ";");
	}
	return count;
}

STRING VarGetArrayElement(VARIABLE var, NUMBER index)
{
	int count = 0;
	char *pTok = NULL;
	char *sCopy;
 
	var = GetEnvironmentVar(g_currentScript, var);
	if (!var) return 0;
 	
	sCopy = StringDupe(var);
	pTok = strtok(sCopy, ";"); 
	while (pTok != NULL)
	{
		if (count == index)
			return pTok;
		count++;
		pTok = strtok(NULL, ";");
	}
	return NULL;
}

NUMBER VarGetArrayElementNumber(VARIABLE var, NUMBER index)
{
	return StringToNumber(VarGetArrayElement(var, index));
}

FRACTION VarGetArrayElementFraction(VARIABLE var, NUMBER index)
{
	return StringToFraction(VarGetArrayElement(var, index));
}

void VarSetArrayElement(VARIABLE var, NUMBER index, STRING value)
{
	int count = 0;
	const char *pTok = NULL;
	STRING orig;
	char *copy;
	char *result;
	char *target;
 
	orig = GetEnvironmentVar(g_currentScript, var);
	if (orig == NULL)
	{
		orig = "";
	}
 	
	copy = StringDupe(orig);
	// This is probably excessive, but it does guarantee we'll have enough space.
	result = StringAlloc(strlen(copy) + strlen(value) + 4 + index);
	result[0] = 0;
	target = result;
	pTok = strtok(copy, ";"); 
	while (pTok != NULL)
	{
		if (count != 0)
		{
			*target++ = ';';
		}
		if (count == index)
		{
			pTok = value;
		}
		while (*target = *pTok)
		{
			target++;
			pTok++;
		}
		count++;
		pTok = strtok(NULL, ";");
	}
	while (count <= index)
	{
		if (count != 0)
		{
			*target++ = ';';
		}
		pTok = (count == index) ? value : "";
		while (*target = *pTok)
		{
			target++;
			pTok++;
		}
		count++;
	}
	SetEnvironmentVar(g_currentScript, var, result);
}

void VarSetArrayElementNumber(VARIABLE var, NUMBER index, NUMBER value)
{
	VarSetArrayElement(var, index, NumberToString(value));
}

// Wrap this around any var to create an Ent specific Var
// Ex: SetVar(EntVar("Statesman", "EnemyName"), "Lord Recluse");
// Statesman now has a var attached to him called "EnemyName" with the value "Lord Recluse"
// ----------------------------------------------------------------------------------------
// Function assumes the entity is described in the form seen in EntityNameFromEnt()
// Returns a string in the form "ent:var" with ent = "ent:ref" or "player:id"
mSTRING EntVar(ENTITY entName, STRING var)
{
	char buf[256];
	sprintf(buf, "%s:%s", entName, var);
	return StringDupe(buf);
}


STRING InstanceVarGet(VARIABLE var)
{
	var = GetInstanceVar(var);
	return var? var: "";
}

void InstanceVarSet(VARIABLE var, STRING value)
{
	if (value != NULL)
		if (value[0] == 0) 
			value = NULL;
	SetInstanceVar(var, value);
}

void InstanceVarRemove(VARIABLE var)
{
	RemoveInstanceVar(var);
}

int InstanceVarIsEmpty(VARIABLE var)
{
	var = GetInstanceVar(var);
	return !(var && var[0]);
}

// *********************************************************************************
//  Numbers and strings
// *********************************************************************************

FRACTION RandomFraction(FRACTION min, FRACTION max)
{
	U32 rand = rule30Rand();
	F32 dif = max - min;
	F32 at = dif * rand / UINT_MAX;
	return min + at;
}

NUMBER RandomNumber(NUMBER min, NUMBER max)
{
	U32 rand = rule30Rand();
	int dif;
	if( min > max )
	{
		NUMBER t=min;
		min=max;
		max=t;
	}
	dif = max - min + 1;
	return min + rand % dif;
}

int StringEqual(STRING lhs, STRING rhs)
{
	if ((!lhs || !lhs[0]) && (!rhs || !rhs[0])) return 1;
	if (!lhs || !lhs[0]) return 0;
	if (!rhs || !rhs[0]) return 0;
	return stricmp(lhs, rhs) == 0;
}

STRING StringAdd(STRING lhs, STRING rhs)
{
	int len;
	char* result;

	if (!lhs || !lhs[0]) return rhs;
	if (!rhs || !rhs[0]) return lhs;
	len = strlen(lhs) + strlen(rhs);
	result = StringAlloc(len);
	strcpy(result, lhs);
	strcat(result, rhs);
	return result;
}

STRING StringCopy(STRING str)
{
	int len;
	char* result;

	if (!str || !str[0]) return str;
	len = strlen(str);
	result = StringAlloc(len);
	strcpy(result, str);
	return result;
}

mSTRING StringCopySafe(STRING str)
{
	int len = 0;
	char* result;

	if (!str) str = "";
	len = strlen(str);
	result = StringAlloc(len);
	strcpy(result, str);
	return result;
}

int StringEmpty(STRING str)
{
	return !str || !str[0];
}

mSTRING NumberToString(NUMBER i)
{
	char buf[100];
	sprintf(buf, "%i", i);
	return StringDupe(buf);
}

mSTRING FractionToString(FRACTION f)
{
	char buf[100];
	sprintf(buf, "%0.3f", f);
	return StringDupe(buf);
}

mSTRING TimeToString(NUMBER seconds)
{
	static char timer[9];
	timer[0] = 0;
	// Put a limit on what we display xx:xx:xx
	seconds = MAX(seconds, 0);
	seconds = MIN(seconds, 359999);
	if (seconds >= 3600)
	{
		strcatf(timer, "%i:", seconds/3600);
		seconds %= 3600;
		strcatf(timer, "%d" , seconds/600);
		seconds %= 600;
		strcatf(timer, "%d", seconds/60);
	} else {
		seconds %= 3600;
		strcatf(timer, "%d" , seconds/600);
		seconds %= 600;
		strcatf(timer, "%d:", seconds/60);
		seconds %= 60;
		strcatf(timer, "%d", seconds/10);
		seconds %= 10;
		strcatf(timer, "%d", seconds);
	}
	return StringDupe(timer);
}

mSTRING FractionTimeToString(FRACTION seconds)
{
	static char timer[15];
	int remainder;
	timer[0] = 0;
	// Put a limit on what we display xx:xx:xx
	seconds = MAX(seconds, 0.0f);
	seconds = MIN(seconds, 359999.0f);
	if (seconds >= 3600.0f)
	{
		remainder = (int) (seconds/3600);
		strcatf(timer, "%i:", remainder);
		seconds -= (FRACTION) (3600.0f * remainder);

		remainder = (int) (seconds/600);
		strcatf(timer, "%d" , remainder);
		seconds -= (FRACTION) (600.0f * remainder);

		remainder = (int) (seconds/60);
		strcatf(timer, "%d", remainder);
	} else {
		remainder = (int) (seconds/600);
		strcatf(timer, "%d" , remainder);
		seconds -= (FRACTION) (600.0f * remainder);

		remainder = (int) (seconds/60);
		strcatf(timer, "%d:", remainder);
		seconds -= (FRACTION) (60.0f * remainder);

		remainder = (int) (seconds/10);
		strcatf(timer, "%d", remainder);
		seconds -= (FRACTION) (10.0f * remainder);

		strcatf(timer, "%.2f", seconds);
	}
	return StringDupe(timer);
}


NUMBER StringToNumber(STRING str)
{
	if (!str || !str[0]) return 0;
	return atoi(str);
}

FRACTION StringToFraction(STRING str)
{
	if (!str || !str[0]) return 0.0;
	return atof(str);
}

NUMBER StringHash(STRING str)
{
	return hashString(str, false);
}

mSTRING StringLocalize(STRING str)
{
	return StringDupe(saUtilScriptLocalize(str));
}

mSTRING StringLocalizeStatic(STRING str)
{
	return StringDupe(saUtilLocalize(0, str));
}

mSTRING StringLocalizeMenuMessages(STRING str)
{
	return StringDupe(textStd(str));
}

mSTRING StringLocalizeWithVars(STRING str, ENTITY player)
{
	ScriptVarsTable vars = {0};
	mSTRING retval = NULL;
	Entity* e = EntTeamInternal(player, 0, NULL);

	if (e) 
	{
		ScriptVarsTableScopeCopy(&vars, &g_currentScript->initialvars);
		saBuildScriptVarsTable(&vars, e, e->db_id, 0, NULL, NULL, NULL, NULL, NULL, 0, 0, false);

		retval = StringDupe(saUtilTableAndEntityLocalize(str, &vars, e));
	}

	return retval;
}

mSTRING StringLocalizeWithDBID(STRING str, NUMBER playerDBID)
{
	ScriptVarsTable vars = {0};
	mSTRING retval = NULL;

	ScriptVarsTableScopeCopy(&vars, &g_currentScript->initialvars);
	saBuildScriptVarsTable(&vars, NULL, playerDBID, 0, NULL, NULL, NULL, NULL, NULL, 0, 0, false);

	retval = StringDupe(saUtilTableLocalize(str, &vars));

	return retval;
}

mSTRING StringLocalizeWithReplacements(STRING str, NUMBER nParams, ...) 
{
	int i;
	va_list	arg;
	ScriptVarsTable vars = {0};
	mSTRING retval = NULL;

	ScriptVarsTableScopeCopy(&vars, &g_currentScript->initialvars);

	va_start(arg, nParams);

	for (i = 0; i < nParams; i++)
	{
		char* tokenName = va_arg(arg, char*);
		char* tokenVal = va_arg(arg, char*);

		ScriptVarsTablePushVarEx(&vars, tokenName, tokenVal, 's', SV_COPYVALUE);
	}

	retval = StringDupe(saUtilTableLocalize(str, &vars));

	ScriptVarsTableClear(&vars); // Needed due to SV_COPYVALUE

	return retval;

	va_end(arg);
}



