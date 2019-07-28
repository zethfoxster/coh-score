/***************************************************************************
 *     Copyright (c) 2000-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/

#ifndef POWERINFO_H
#define POWERINFO_H

typedef struct Packet Packet;
typedef struct Character Character;
#include "powers.h"

//-------------------------------------------------------------------------------------------
// PowerRechargeTimer
//-------------------------------------------------------------------------------------------
/* Structure PowerRechargeTimer
 *  Provides information to indicate how much more time a player needs to wait before
 *  a power can be used again.
 *
 */

typedef struct PowerRechargeTimer
{
	PowerRef	ppowRef;              // Which power are we describing?
	float       rechargeCountdown;  // How much time left until the power can be used again?
} PowerRechargeTimer;

PowerRechargeTimer* powerRechargeTimer_Create(void);
void powerRechargeTimer_Destroy(PowerRechargeTimer* timer);

//-------------------------------------------------------------------------------------------
// PowerInfo
//-------------------------------------------------------------------------------------------
/* Structure PowerInfo
 *  Top level structure for the PowerInfo module.
 *
 *  It tracks the following information:
 *      - Which powers are currently active.
 *      - How long until certain powers can be used again.
 */
typedef struct PowerInfo
{
	PowerRef** activePowers;               // What powers do I have turned on?
	PowerRechargeTimer** rechargeTimers;    // Which powers am I waiting on?
} PowerInfo;

PowerInfo* powerInfo_Create(void);
void powerInfo_Destroy(PowerInfo* info);

void powerInfo_ChangeActiveState(PowerInfo* info, PowerRef ppowRef, int active);
void powerInfo_ChangeRechargeTimer(PowerInfo* info, PowerRef ppowRef, float timer);
void powerInfo_UpdateFullRechargeStatus(Character *p);

void powerInfo_UpdateTimers(PowerInfo* info, float elapsedTime);

int powerInfo_PowerIsActive(PowerInfo* info, PowerRef ppowRef);
PowerRechargeTimer* powerInfo_PowerGetRechargeTimer(PowerInfo* info, PowerRef ppowRef);

PowerRef* powerInfo_FindPower(PowerRef ***powerRefArray, PowerRef ppowRef);
int powerInfo_FindPowerIndex(PowerRef ***powerRefArray, PowerRef ppowRef);

//-------------------------------------------------------------------------------------------
// PowerInfo network related stuff
//-------------------------------------------------------------------------------------------
void powerActivation_Send(PowerRef* ppowRef, Packet* pak, int activate);
void powerActivation_Receive(PowerRef* ppowRef, Packet* pak, int* activate);
void powerRechargeTimer_Send(PowerRechargeTimer* timer, Packet* pak);
void powerRechargeTimer_Receive(PowerRechargeTimer* timer, Packet* pak);
void powerInfo_Send(PowerInfo* info, Packet* pak);
void powerInfo_Receive(PowerInfo* info, Packet* pak);

typedef struct Entity Entity;
typedef struct Packet Packet;
#ifdef SERVER
void entity_SendPowerInfoUpdate(Entity* e, Packet* pak);
void entity_SendPowerModeUpdate(Entity* e, Packet* pak);
void power_sendTrayAdd( Entity *e, const BasePower * basePower );
#else
void entity_ReceivePowerInfoUpdate(Entity* e, Packet* pak);
void entity_ReceivePowerModeUpdate(Entity* e, Packet* pak);
#endif

#endif