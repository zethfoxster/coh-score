
/*\
 *
 *	scriptdebug.h/c - Copyright 2004 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	Debug support for scripts
 *
 */

#include "scriptdebug.h"
#include "scriptengine.h"
#include "comm_game.h"
#include "debugCommon.h"
#include "earray.h"
#include "entity.h"

Packet* sv_pak;
int sv_count;
static int countVars(StashElement el)
{
	sv_count++;
	return 1;
}
static int sendVars(StashElement el)
{
	char* val = 0;
	pktSendString(sv_pak, stashElementGetStringKey(el));
	val = stashElementGetPointer(el);
	if (val == SE_DYNAMIC_LOOKUP)
		pktSendString(sv_pak, "(Dynamic)");
	else
		pktSendString(sv_pak, val);
	return 1;
}

void ScriptSendDebugInfo(ClientLink* client, int debugFlags, int debugSelect)
{
	int i, n, count;
	ScriptEnvironment* selected;
	ScriptFunction* func;

	START_PACKET(pak, client->entity, SERVER_SCRIPT_DEBUGINFO)

	// list of running scripts
	n = eaSize(&g_runningScripts);
	pktSendBitsPack(pak, 1, n);
	for (i = 0; i < n; i++)
	{
		selected = g_runningScripts[i];
		func = selected->function;
		pktSendString(pak, func->name);
		pktSendBitsPack(pak, 1, func->type);
		pktSendBitsPack(pak, 1, selected->timeDelta);
		pktSendBitsPack(pak, 1, selected->pauseTime);
		pktSendBitsPack(pak, 1, selected->runState);
		pktSendBitsPack(pak, 1, selected->id);
		pktSendBitsPack(pak, 1, LuaMemoryUsed(selected));
	}
	pktSendBits(pak, 1, client->scriptDebugFlags & SCRIPTDEBUGFLAG_SHOWVARS? 1: 0);

	// details on selected script
	selected = ScriptFromRunId(debugSelect);
	pktSendBitsPack(pak, 1, selected? selected->id: 0);
	if (selected)
	{
		func = selected->function;
		g_currentScript = selected;	// so we can use Var utils
		
		// buttons
		count = 0;
		for (i = 0; i < MAX_SCRIPT_SIGNALS; i++)
		{
			if (!func->signals[i].display || !func->signals[i].display[0]) break;
			count++;
		}
		pktSendBitsPack(pak, 1, count);
		for (i = 0; i < MAX_SCRIPT_SIGNALS; i++)
		{
			if (!func->signals[i].display || !func->signals[i].display[0]) break;
			pktSendString(pak, func->signals[i].display);
			pktSendString(pak, func->signals[i].signal);
		}

		// variables
		if (debugFlags & SCRIPTDEBUGFLAG_SHOWVARS)
		{
			StashTableIterator iter;
			StashElement	el;
			int flags;
			
			sv_count = 0;
			stashGetIterator(selected->vars, &iter);
			while (stashGetNextElement(&iter, &el))
			{
				if (stashFindInt(selected->varsFlags, stashElementGetStringKey(el), &flags)) {
					if (!(flags & SP_DONTDISPLAYDEBUG))
						sv_count++;
				} else {
					sv_count++;
				}
			}
//			stashForEachElement(selected->vars, countVars);
			pktSendBitsPack(pak, 1, sv_count);
			
			sv_pak = pak;
			stashGetIterator(selected->vars, &iter);
			while (stashGetNextElement(&iter, &el))
			{
				if (stashFindInt(selected->varsFlags, stashElementGetStringKey(el), &flags)) {
					if (!(flags & SP_DONTDISPLAYDEBUG))
						sendVars(el);
				} else {
					sendVars(el);
				}
			}
//			stashForEachElement(selected->vars, sendVars);
		}
		else
			pktSendBitsPack(pak, 1, 0);
	}

	END_PACKET
}

void ScriptDebugSetServer(ClientLink* client, int on, int scriptId)
{
	if (on)
		client->scriptDebugFlags |= SCRIPTDEBUGFLAG_DEBUGON;
	else
		client->scriptDebugFlags &= ~SCRIPTDEBUGFLAG_DEBUGON;
	client->scriptDebugSelect = scriptId;
}

void ScriptDebugShowVars(ClientLink* client, int on)
{
	if (on)
		client->scriptDebugFlags = SCRIPTDEBUGFLAG_DEBUGON | SCRIPTDEBUGFLAG_SHOWVARS;
	else
		client->scriptDebugFlags &= ~SCRIPTDEBUGFLAG_SHOWVARS;
}

void ScriptDebugSetVar(ClientLink* client, int id, char *key, char *value)
{
	ScriptEnvironment* script = ScriptFromRunId(id);

	if (script)
	{
		SetEnvironmentVar(script, key, value);
	}
}
