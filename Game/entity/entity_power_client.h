/***************************************************************************
 *     Copyright (c) 2003-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef ENTITY_POWER_CLIENT_H__
#define ENTITY_POWER_CLIENT_H__

typedef struct Packet Packet;
typedef struct Entity Entity;

void entity_ActivatePowerSend(Entity *e, Packet *pak, int ibuild, int iset, int ipow, Entity *ptarget);
void entity_ActivatePowerAtLocationSend(Entity *e, Packet *pak, int ibuild, int iset, int ipow, Entity *ptarget, Vec3 loc);
void entity_SetDefaultPowerSend(Entity *e, Packet *pak, int build, int iset, int ipow);
void entity_ActivateInspirationSend(Entity *e, Packet *pak, int icol, int irow);
void entity_EnterStanceSend(Entity *e, Packet *pak, int build, int iset, int ipow);
void entity_ExitStanceSend(Entity *e, Packet *pak);

void entity_StanceReceive(Packet *pak);

#endif /* #ifndef ENTITY_POWER_CLIENT_H__ */

/* End of File */

