/*\
 *
 *	scriptdebug.h/c - Copyright 2004 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	Debug support for scripts
 *
 */

#ifndef SCRIPTDEBUG_H
#define SCRIPTDEBUG_H

#include "svr_base.h"

void ScriptSendDebugInfo(ClientLink* client, int debugFlags, int debugSelect);
void ScriptDebugSetServer(ClientLink* client, int on, int scriptId);
void ScriptDebugShowVars(ClientLink* client, int on);
void ScriptDebugSetVar(ClientLink* client, int id, char *key, char *value);


#endif // SCRIPTDEBUG_H