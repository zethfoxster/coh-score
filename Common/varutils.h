/***************************************************************************
 *     Copyright (c) 2000-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef VARUTILS_H__
#define VARUTILS_H__

#include "entity_enum.h"

typedef struct Entity Entity;

void zeroEntVars( Entity *e );
void playerVarAlloc( Entity *e, EntType ent_type );
void playerVarFree( Entity *e );
#if SERVER
typedef struct Character *pCharacter;
pCharacter character_Allocate();
void character_Free(pCharacter character);
#endif

#endif