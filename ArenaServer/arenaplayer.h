/*\
 *
 *	ArenaPlayer.h/c - Copyright 2004, 2005 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	Arena server handling of all ArenaPlayer structures -
 *  keeps track of each player's rating, etc.
 *
 */

#ifndef ARENAPLAYER_H
#define ARENAPLAYER_H

#include "arenastruct.h"

ArenaPlayer* PlayerGet(U32 dbid, int req_new);	        // get existing record
ArenaPlayer* PlayerGetAdd(U32 dbid, int initialLoad);	// return existing record or create new one
ArenaPlayer** PlayerGetAll(void);						// returns the ArenaPlayer EArray (for building the rating list
void PlayerSave(U32 dbid);
int PlayerTotal(void);							// how many player records do we have?

void PlayerInit(ArenaPlayer* ap, U32 dbid);		// do opening initialization on a new player
void PlayerListInit(void);	                    // all players just got loaded
void PlayerListTick(void);						// timeout players as needed

// deal with player connection state
void PlayerSetConnected(U32 dbid, int connected);	// mapserver connection update

#endif // ARENAPLAYER_H