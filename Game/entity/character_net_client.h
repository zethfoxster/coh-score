/***************************************************************************
 *     Copyright (c) 2003-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef CHARACTER_NET_CLIENT_H__
#define CHARACTER_NET_CLIENT_H__

typedef struct Entity Entity;
typedef struct Packet Packet;
typedef struct Character Character;
typedef struct BasePower BasePower;

void character_SendCreate(Packet* pak, Character *pchar);
void character_SendBuyPower(Packet *pak, Character *pchar, const Power *ppow, PowerSystem ps);

void character_ReceiveInspirations(Packet *pak, Character *pchar);
void character_Receive(Packet* pak, Character *pchar, Entity *e);
void character_ReceiveBuffs(Packet *pak, Entity *e, bool oo_data);
void character_ReceiveStatusEffects(Packet *pak, Character *p, bool oo_data);
// void character_ReceiveBuffRemove(Packet *pak, Entity *e, bool oo_data);

void inspiration_Receive(Packet *pak, Character *pchar);

void power_SendBuyBoostSlot(Packet *pak, Power *ppow);
void power_SendCombineBoosts(Packet *pak, Boost * pbA, Boost *pbB );

#endif /* #ifndef CHARACTER_NET_CLIENT_H__ */

/* End of File */

