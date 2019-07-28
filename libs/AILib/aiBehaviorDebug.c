#include "aiStruct.h"
#include "aiBehaviorPublic.h"
#include "aiBehaviorPrivate.h"
#include "aiBehaviorDebug.h"

#include "earray.h"
#include "error.h"
#include "EString.h"
#include "file.h"
#include "SuperAssert.h"
#include "textparser.h"
#include "timing.h"

// void aiBehaviorVerifyBehaviorStructs(Entity* e, AIVars* ai, AIBehavior*** behaviors)
// {
// 	int i;
// 	AITeam* team = ai->teamMemberInfo.team;
// 	int numFlags = 0;
// 
// 	if(isDevelopmentMode())
// 	{
// 		for(i = eaSize(behaviors)-1; i>=0; i--)
// 		{
// 			AIBehavior* behavior = (*behaviors)[i];
// 			devassert(behavior->info);
// 
// 			if(behavior->info->teamDataStructSize)
// 			{
// 				AITeamMemberInfo* member;
// 				int j, count = 0;
// 				bool found = false;
// 
// 				for(member = team->members.list; member; member = member->next)
// 				{
// 					AIVars* memberAI = ENTAI(member->entity);
// 					for(j = eaSize(&memberAI->behaviors)-1; j >= 0; j--)
// 					{
// 						if(memberAI->behaviors[j]->info == (*behaviors)[i]->info)
// 						{
// 							devassert(memberAI->behaviors[j]->teamDataID == (*behaviors)[i]->teamDataID);
// 							count++;
// 						}
// 					}
// 				}
// 
// 				for(j = eaSize(&team->behaviorData)-1; j >= 0 && !found; j--)
// 				{
// 					devassert(team->behaviorData[j]->teamDataID != 0xfafafafa);
// 					if(team->behaviorData[j]->teamDataID == (*behaviors)[i]->teamDataID)
// 					{
// 						devassert(team->behaviorData[j]->behaviorInfo == (*behaviors)[i]->info);
// 						devassert(team->behaviorData[j]->refCount == count);
// 						found = true;
// 					}
// 				}
// 				for(j = eaSize(&team->behaviorData)-1; j >= 0 && !found; j--)
// 				{
// 					if(team->behaviorData[j]->behaviorInfo == (*behaviors)[i]->info)
// 					{
// 						devassertmsg(team->behaviorData[j]->refCount == count, "teamDataID also screwed up?");
// 						devassert(team->behaviorData[j]->teamDataID == (*behaviors)[i]->teamDataID);
// 						found = true;
// 					}
// 				}
// 				devassertmsg(found, "Did not find matching teamdata for behavior");
// 			}
// 		}
// 
// 		for(i = eaSize(behaviors)-1; i >= 0; i--)
// 		{
// 			numFlags += eaSize(&(*behaviors)[i]->flags);
// 		}
// 		devassert(numFlags == ai->behaviorFlagsAllocated);
// 
// 		// check the other team members' team data also
// 		if(team)
// 		{
// 			for(i = eaSize(&team->behaviorData)-1; i >= 0; i--)
// 			{
// 				AITeamMemberInfo* member;
// 				int j, count = 0;
// 				bool found = false;
// 
// 				devassert(team->behaviorData[i]->teamDataID != 0xfafafafa);
// 				devassert(team->behaviorData[i]->behaviorInfo != (AIBehaviorInfo*)0xfafafafa);
// 				devassert(team->behaviorData[i]->refCount != 0xfafafafa && team->behaviorData[i]->refCount);
// 				devassert(team->behaviorData[i]->data != (void*)0xfafafafa);
// 
// 				for(member = team->members.list; member; member = member->next)
// 				{
// 					AIVars* memberAI = ENTAI(member->entity);
// 					for(j = eaSize(&memberAI->behaviors)-1; j >= 0; j--)
// 					{
// 						if(memberAI->behaviors[j]->info == team->behaviorData[i]->behaviorInfo)
// 						{
// 							devassert(memberAI->behaviors[j]->teamDataID == team->behaviorData[i]->teamDataID);
// 							count++;
// 						}
// 					}
// 				}
// 				devassert(team->behaviorData[i]->refCount == count);
// 			}
// 		}
// 	}
// }


char* aiBehaviorGetDebugString(Entity* e, AIVarsBase* aibase, AIBehavior* behavior, char* indent)
{
	static char* str[5] = {0};
	static U32 recursions = 0;

	char* name;
	char newIndent[200];

	devassertmsg(++recursions < 5, "Too many recursive calls to aiBehaviorGetDebugString");

	if(!str[recursions])
		estrCreate(&str[recursions]);

	estrClear(&str[recursions]);
	sprintf_s(SAFESTR(newIndent), "%s     ", indent);

	if(behavior->running)
		estrConcatStaticCharArray(&str[recursions], "^2");
	else if(!behavior->activated)
		estrConcatStaticCharArray(&str[recursions], "^8");
	else if(behavior->finished)
		estrConcatStaticCharArray(&str[recursions], "^1");
	else
		estrConcatStaticCharArray(&str[recursions], "^5");

	name = behavior->info->name;

	estrConcatf(&str[recursions], "%s%s^0(^sid:^4%i^0,time:^4%.1f^0^n)", indent,
		name ? name : "", behavior->id, behavior->timeRun ? (F32)behavior->timeRun/3000 : 0.0);

	if(behavior->info->debugFunc)
	{
		estrConcatf(&str[recursions], "\n%s", newIndent);

		behavior->info->debugFunc(e, aibase, behavior, &str[recursions], newIndent);
	}

	if(eaSize(&behavior->flags))
	{
		estrConcatf(&str[recursions], "\n%s", newIndent);

		aiBehaviorFlagDebugString(e, behavior, &str[recursions]);
	}


	return str[recursions--];
}

char* aiBehaviorDebugPrint(Entity* e, AIVarsBase* aibase, int stripColors)
{
	int i, bufidx = 0, len;
	static char* str = NULL;
	static char* buf = NULL;
	if(!e || !aibase)
		return NULL;

	estrClear(&buf);
	estrClear(&str);

	for(i = 0; i < eaSize(&aibase->behaviors); i++)
		estrConcatf(&str, "%i %s\n", i, aiBehaviorGetDebugString(e, aibase, aibase->behaviors[i], ""));

	if(!stripColors)
		return str;

	len = (int)strlen(str);
	estrSetLength(&buf, len);

	for(i = 0; i < len; i++)
	{
		if(str[i] == '^')
			i++;
		else
			buf[bufidx++] = str[i];
	}
	buf[bufidx] = '\0';

	// 	printf(buf);
	return buf;
}

char* aiBehaviorDebugStripCrap(const char* input)
{
	// lazy, so don't expect results to stick around for very long, but at least you can compare a few strings
	static char* buf[3] = {0,0,0};
	static int counter = 0;
	char* mybuf = buf[counter++%3];
	const char* c;
	char* out;
	bool quoted = false;
	bool prevWhite = false;

	estrPrintCharString(&mybuf, input);
	out = mybuf;

	for (c=input; *c; c++)
	{
		bool curWhite = (bool)strchr(" \t,", *c);
		if(*c == '\"')
		{
			quoted = !quoted;
			continue;
		}
		if(quoted)
		{
			*out++ = *c;
			continue;
		}

		if(!prevWhite || !curWhite)
		{
			if(curWhite)
			{
				if(*(c-1) != '(' && *(c+1) != ')' && *(c+1) != '(' && *(c-1) != ')' )
					*out++ = ' ';
			}
			else
				*out++ = *c;			
		}
		prevWhite = curWhite;
	}
	*out = 0;
	return mybuf;
}

char* aiBehaviorDebugPrintParseHelper(char* string)
{
	static char* buf = NULL;

	if(strpbrk(string, ",() "))
		estrPrintf(&buf, "\"%s\"", string);
	else
		estrPrintCharString(&buf, string);

	return buf;
}

char* aiBehaviorDebugPrintParse(AIBParsedString* parsed)
{
	static char* estr = NULL;
	int i,j,k,l;
	int baseStrings, iChildren, jChildren, kChildren;

	estrClear(&estr);

	baseStrings = eaSize(&parsed->string);

	for(i = 0; i < baseStrings; i++)
	{
		iChildren = eaSize(&parsed->string[i]->params);
		estrConcatf(&estr, "%s%s",
			aiBehaviorDebugPrintParseHelper(parsed->string[i]->function), iChildren ? "(" : "");
		for(j = 0; j < iChildren; j++)
		{
			jChildren = eaSize(&parsed->string[i]->params[j]->params);
			estrConcatf(&estr, "%s%s",
				aiBehaviorDebugPrintParseHelper(parsed->string[i]->params[j]->function),
				jChildren ? "(" : "");
			for(k = 0; k < jChildren; k++)
			{
				kChildren = eaSize(&parsed->string[i]->params[j]->params[k]->params);
				estrConcatf(&estr, "%s%s",
					aiBehaviorDebugPrintParseHelper(parsed->string[i]->params[j]->params[k]->function),
					kChildren ? "(" : "");
				for(l = 0; l < kChildren; l++)
				{
					estrConcatCharString(&estr,
						aiBehaviorDebugPrintParseHelper(parsed->string[i]->params[j]->params[k]->params[l]->function));
					if(l < kChildren-1)
						estrConcatChar(&estr, ' ');
				}
				if(kChildren)
					estrConcatChar(&estr, ')');
				if(k < jChildren-1)
					estrConcatChar(&estr, ' ');
			}
			if(jChildren)
				estrConcatChar(&estr, ')');
			if(j < iChildren-1)
				estrConcatChar(&estr, ' ');
		}
		if(iChildren)
			estrConcatChar(&estr, ')');
		if(i < baseStrings-1)
			estrConcatChar(&estr, ' ');
	}

	return estr;
}

bool aiBehaviorDebugVerifyParse(const char* str, AIBParsedString* parsed)
{
	int cmpval;
	char *str1,*str2;
	str1 = aiBehaviorDebugStripCrap(str);
	str2 = aiBehaviorDebugStripCrap(aiBehaviorDebugPrintParse(parsed));
	cmpval = stricmp(str1, str2);
	//devassert(!cmpval);
	if(cmpval)
	{
		Errorf("Failed: %s (%s) turns into %s", str, str1, str2);
	}
	return !cmpval;
}

char* aiBehaviorPrintMod(AIBehaviorMod* mod)
{
	static char* buf;
	char timerstring[128];

	timerMakeOffsetStringFromSeconds(timerstring, mod->time/3000);

	estrPrintf(&buf, "%s %s\n", timerstring, mod->string);

	return buf;
}

void aiBehaviorFlagDebugString(Entity* e, AIBehavior* behavior, char** estr)
{
	int i, j, numflags = eaSize(&behavior->flags);
	int count;
	AIBehaviorFlag* flag;

	estrConcatStaticCharArray(estr, "^0Current Flags: ");
	for(i = 0; i < numflags; i++)
	{
		flag = behavior->flags[i];

		estrConcatf(estr, "^8%s^0(", flag->info->name);

		switch(flag->info->datatype)
		{
		xcase FLAG_DATA_ALLOCSTR:
		case FLAG_DATA_ESTR:
			estrConcatCharString(estr, flag->strptr);
		xcase FLAG_DATA_BOOL:
			estrConcatf(estr, "^4%d", flag->intarray[0]);
		xcase FLAG_DATA_ENTREF:
			estrConcatCharString(estr, AI_LOG_ENT_NAME(erGetEnt(flag->entref)));
		xcase FLAG_DATA_FLOAT:
			estrConcatf(estr, "^4%.1f", flag->floatarray[0]);
		xcase FLAG_DATA_INT:
			estrConcatf(estr, "^4%d", flag->intarray[0]);
		xcase FLAG_DATA_INT3:
			estrConcatf(estr, "^4%d^0, ^4%d^0, ^4%d", flag->intarray[0], flag->intarray[1], flag->intarray[2]);
		xcase FLAG_DATA_MULT_STR:
			count = eaSize(&flag->strarray);
			for(j = 0; j < count; j++)
				estrConcatf(estr, "%s%s", flag->strarray[j], j == count-1 ? "" : ", ");
		xcase FLAG_DATA_PARSE:
			estrConcatCharString(estr, StaticDefineIntRevLookup(flag->info->parseTable, flag->intarray[0]));
		xcase FLAG_DATA_PERCENT:
			estrConcatf(estr, "^4%.1f", flag->floatarray[0]);
		xcase FLAG_DATA_SPECIAL:
			// can't do anything for FLAG_DATA_SPECIAL
		xcase FLAG_DATA_VEC3:
			estrConcatf(estr, "^4%0.1f^0, ^4%0.1f^0, ^4%0.1f^0", flag->floatarray[0], flag->floatarray[1], flag->floatarray[2]);
		}

		estrConcatf(estr, "^0)%s", i < (numflags-1) ? ", " : "");
	}
}

int aiBehaviorDebugGetTableCount()
{
	return eaSize(&BehaviorSystemTableList);
}

const char* aiBehaviorDebugGetTableEntry(int idx)
{
	return BehaviorSystemTableList[idx]->name;
}

int aiBehaviorDebugGetAliasCount()
{
	return eaSize(&BehaviorSystemDebugAliasList);
}

const char* aiBehaviorDebugGetAliasEntry(int idx)
{
	return BehaviorSystemDebugAliasList[idx]->name;
}

char* aiBehaviorDebugPrintPendingMods(Entity* e, AIVarsBase* aibase)
{
	int i;
	static char* buf = NULL;

	if(!e || !aibase)
		return NULL;

	estrClear(&buf);

	for(i = 0; i < eaSize(&aibase->behaviorMods); i++)
	{
		estrConcatCharString(&buf, aiBehaviorPrintMod(aibase->behaviorMods[i]));
	}

	return buf;
}

char* aiBehaviorDebugPrintPrevMods(Entity* e, AIVarsBase* aibase)
{
	int i;
	static char* buf = NULL;

	if(!e || !aibase)
		return NULL;

	estrClear(&buf);

	for(i = 0; i < eaSize(&aibase->behaviorPrevMods); i++)
	{
		estrConcatCharString(&buf, aiBehaviorPrintMod(aibase->behaviorPrevMods[i]));
	}

	return buf;
}