/***************************************************************************
 *     Copyright (c) 2003-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef ENTITY_POWER_H__
#define ENTITY_POWER_H__

typedef struct Packet Packet;
typedef struct Entity Entity;
typedef struct Character Character;
typedef struct Power Power;

bool entity_ActivateInspirationReceive(Entity *e, Packet *pak);
bool entity_MoveInspirationReceive(Entity *e, Packet *pak);
bool entity_ActivatePowerReceive(Entity *e, Packet *pak);
bool entity_ActivatePowerAtLocationReceive(Entity *e, Packet *pak);
bool entity_SetDefaultPowerReceive(Entity *e, Packet *pak);
bool entity_StanceReceive(Entity *e, Packet *pak);

void entity_StanceSend(Entity *e, Power *ppow);

#endif /* #ifndef ENTITY_POWER_H__ */

/* End of File */

