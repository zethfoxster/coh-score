#ifndef NPCSERVER_H
#define NPCSERVER_H

#include "character_base.h"	
#include "entity_enum.h"

typedef struct Entity Entity;
typedef struct PowerNameRef PowerNameRef;

Entity* npcCreate(const char *name, EntType ent_type);
int npcInitPowers(Entity* e, const PowerNameRef** powers, int autopower_only);
int npcAddPower(Character* character, const char* categoryName, const char* powerSetName, const char* powerName, int dontSetStance, int autopower_only);

void npcInteract(Entity *npc, Entity *e);

#endif