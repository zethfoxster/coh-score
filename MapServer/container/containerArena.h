/*\
 *
 *	containerArena.h/c - Copyright 2004, 2005 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	Structure defs and container pack/unpack for arena stuff
 *
 */

#ifndef _CONTAINERARENA_H
#define _CONTAINERARENA_H

#include "arenastruct.h"

char *arenaEventTemplate();
char *arenaEventSchema();
char *packageArenaEvent(ArenaEvent* ae);
void unpackArenaEvent(ArenaEvent* ae,char *mem,U32 id);

char *arenaPlayerTemplate();
char *arenaPlayerSchema();
char *packageArenaPlayer(ArenaPlayer* ap);
void unpackArenaPlayer(ArenaPlayer* ap, char* mem, U32 id);

#endif // _CONTAINERARENA_H
