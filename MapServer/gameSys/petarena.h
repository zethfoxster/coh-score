/*\
 *
 *	arenamap.h/c - Copyright 2004, 2005 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	Code to run an instanced arena map.
 *
 */

#ifndef PETARENA_H
#define PETARENA_H

typedef struct Entity Entity;
typedef struct Packet Packet;

int OnPetArenaMap();
void ArenaCreateArenaPets( Entity * e );
void LoadPetBattleCreatureInfos();

void sendArenaManagePets( Entity * e );
void PetArenaCommitPetChanges( Entity * e,  Packet* pak );

void updatePetState( Entity * pet );

#endif // ARENAMAP_H