/*\
*
*	arenaref.h - Copyright 2004, 2005 Cryptic Studios
*		All Rights Reserved
*		Confidential property of Cryptic Studios
*
*	Arenaref stuff
*
 */

#ifndef ARENAREF_H
#define ARENAREF_H

#include "stdtypes.h"

typedef struct ArenaRef
{
	U32				eventid;
	U32				uniqueid;
} ArenaRef;

#define ArenaRefMatch(lhs, rhs) ((lhs)->eventid == (rhs)->eventid && (lhs)->uniqueid == (rhs)->uniqueid) // events or refs
#define ArenaRefCopy(dst, src) ((dst)->uniqueid = (src)->uniqueid, (dst)->eventid = (src)->eventid) // events or refs
#define ArenaRefZero(dst) ((dst)->eventid = (dst)->uniqueid = 0)

#endif