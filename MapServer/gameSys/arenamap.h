/*\
 *
 *	arenamap.h/c - Copyright 2004, 2005 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	Code to run an instanced arena map.
 *
 */

#ifndef ARENAMAP_H
#define ARENAMAP_H

#ifndef ARENA_GLOBAL_RESPAWN_PERIOD
#define	ARENA_GLOBAL_RESPAWN_PERIOD	15
#endif

#include "arenastruct.h"

typedef struct Character Character;

typedef enum
{
	ARENAMAP_INIT,
	ARENAMAP_HOLD, // waiting for players
	ARENAMAP_BUFF,
	ARENAMAP_FIGHT,
	ARENAMAP_SUDDEN_DEATH,
	ARENAMAP_FINISH,
} ArenaMapPhase;

extern ArenaEvent* g_arenamap_event;
extern U32 g_arenamap_eventid;
extern int g_arenamap_phase;
extern U32 g_matchstarttime; // time you enter ARENAMAP_FIGHT phase

void ArenaSetInfoString(char* info);
int OnArenaMap(void);
int OnArenaMapNoChat(void);
void ArenaMapSetPosition(Entity* e);
void ArenaMapInitPlayer(Entity* e);
void ArenaMapReceivedCharacter(Entity* e);
void ArenaMapProcess(void);
int ArenaMapHandleEntityRespawn(Entity* e);
void ArenaMapHandleDeath(Entity* e);
void ArenaMapHandleKill(Entity* killer, Entity* victim);
void ArenaGiveStdInspirations(Entity* e);
void ArenaGiveTempPower(Entity* e, char* powerstr); // should really make this not an arena function...
void ArenaRemoveTempPower(Entity* e, char* powerstr); // should really make this not an arena function...

ArenaEvent* GetMyEvent(void);

void ArenaSwitchCamera(Entity* e, int on);
int ArenaMapLevelOverride(Character* p);
int ArenaMapIsParticipant(Entity* e);

// kind of a general method for putting several entities around a spawn point.
// you should pass in two U32's that are used by RotateSpawn and changed for each player
void RotateSpawn(Entity* e, U32 rotate[2], int* result);

#endif // ARENAMAP_H