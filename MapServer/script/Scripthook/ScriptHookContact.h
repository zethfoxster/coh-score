/*\
*
*	scripthookcontact.h - Copyright 2004 Cryptic Studios
*		All Rights Reserved
*		Confidential property of Cryptic Studios
*
*	Provides all the functions CoH script files should be referencing -
*		essentially a list of hooks into game systems.
*
*	This file contains all functions related to locations
*
 */

#ifndef SCRIPTHOOKCONTACT_H
#define SCRIPTHOOKCONTACT_H

int DoesPlayerHaveContact(ENTITY player, ContactHandle contact);
void ScriptGivePlayerNewContact(ENTITY player, ContactHandle contact);
ContactHandle ScriptGetContactHandle(STRING contact);
void ScriptRegisterNewContact(ContactHandle contact, LOCATION location);

void OpenStore(ENTITY npc, ENTITY player, STRING name);
void OpenAuction(ENTITY npc, ENTITY player);

int ContactHandleDialogDef(ENTITY player, ENTITY target, STRING dialog, STRING startPage, STRING filename, NUMBER link, ContactResponse* response);

#endif