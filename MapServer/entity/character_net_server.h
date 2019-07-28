/***************************************************************************
 *     Copyright (c) 2003-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef CHARACTER_NET_SERVER_H__
#define CHARACTER_NET_SERVER_H__

// forward decls
typedef struct Entity Entity;
typedef struct Packet Packet;
typedef struct Character Character;

bool character_IsLeveling(Character *pchar);

void character_SendInspirations(Packet *pak, Character *pchar);
void character_Send(Packet* pak, Character *pchar);
void character_SendBuffs(Packet *pak, Entity * recvEnt, Character *p);
// void character_sendBuffRemove(Packet *pak, Character *p);
void character_SendStatusEffects(Packet *pak, Character *p);
void character_UpdateBuffStash(Character * p);

void inspiration_Send(Packet *pak, Character *pchar, int iCol, int iRow);

bool character_ReceiveCreate(Packet* pak, Character *pchar, Entity *e);
bool character_ReceiveCreateClassOrigin(Packet* pak, Character *pchar, Entity *e, const CharacterClass *oldClass, const CharacterOrigin *oldOrigin);
bool character_ReceiveCreatePowers(Packet* pak, Character *pchar, Entity *e);

bool character_ReceiveLevelAndBuyPower(Packet *pak, Character *pchar);
bool power_ReceiveLevelAndAssignBoostSlot(Packet *pak, Character *pchar);
bool character_ReceiveLevel(Packet *pak, Character *pchar);

bool character_ReceiveSetBoost( Packet *pak, Character *pchar);
bool character_ReceiveRemoveBoost( Packet *pak, Character *pchar);
bool character_ReceiveRemoveBoostFromPower( Packet *pak, Character *pchar);
bool character_ReceiveUnslotBoostFromPower(Packet *pak, Character *pchar);
bool power_ReceiveCombineBoosts(Packet *pak, Character *pchar);
bool power_ReceiveBoosterBoost(Packet *pak, Character *pchar);
bool power_ReceiveCatalystBoost(Packet *pak, Character *pchar);
bool power_ReceiveSetBoost( Packet *pak, Character *pchar);
void character_SendCombineResponse( Entity *e, int iSuccess, int iSlot );
void character_SendBoosterResponse( Entity *e, int iSuccess, int iSlot );
void character_SendCatalystResponse( Entity *e, int iSuccess, int iSlot );

void character_toggleAuctionWindow(Entity *e, int windowUp, int npcId);

int player_RepairCorruption(Entity *e, int justChecking);

bool character_IsValidAT(Entity *e, Character *pchar, CharacterClass *pclass, int bCanBePraetorian);

#endif /* #ifndef CHARACTER_NET_SERVER_H__ */

/* End of File */

