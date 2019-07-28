/***************************************************************************
 *     Copyright (c) 2000-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef ENTITYREF_H__
#define ENTITYREF_H__

typedef struct Entity Entity;

typedef S64 EntityRef;

EntityRef	erGetRef(Entity* ent);
Entity*		erGetEnt(EntityRef ref);
S32			erGetEntID(EntityRef ref);
S32			erGetDbId(EntityRef ref);
char*		erGetRefString(Entity* ent);	// static buffer
Entity*		erGetEntFromString(const char* refstring);

#endif /* #ifndef ENTITYREF_H__ */

/* End of File */

