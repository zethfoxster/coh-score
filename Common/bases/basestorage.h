/***************************************************************************
 *     Copyright (c) 2005-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef BASESTORAGE_H
#define BASESTORAGE_H

#include "stdtypes.h"

typedef struct GenericHashTableImp *GenericHashTable;
typedef struct HashTableImp *HashTable;
typedef struct Character Character;
typedef struct	Packet Packet;
typedef struct Entity Entity;
typedef struct RoomDetail RoomDetail;
typedef struct SalvageItem SalvageItem;
typedef struct StoredSalvage StoredSalvage;
typedef struct BasePower BasePower;

// *********************************************************************************
// general methods
// *********************************************************************************

bool roomdetail_CanAdjSalvage(RoomDetail *detail, const char *nameItem, int amount);
bool roomdetail_CanAdjInspiration(RoomDetail *detail, const char *pathInsp, int amount);
bool roomdetail_CanAdjEnhancement(RoomDetail *detail, const char *pathEnhancement, int iLevel, int iNumCombines, int amount);

StoredSalvage *roomdetail_FindStoredSalvage(RoomDetail *detail, const char *nameSalvage, int *resI);

bool entity_CanAdjStoredSalvage(Entity *e, RoomDetail *detail, const SalvageItem *salvage, int amount);
bool entity_CanAdjStoredInspiration(Entity *e, RoomDetail *detail, const BasePower *ppow, const char *pathInsp, int amount);
bool entity_CanAdjStoredEnhancement(Entity *e, RoomDetail *detail, const BasePower *ppow, const char *pathInsp, int iLevel, int iNumCombines, int amount);

int roomdetail_SetSalvage(RoomDetail *detail, const char *nameSalvage, int amt);
int roomdetail_AdjSalvage(RoomDetail *detail, const char *nameSalvage, int amount);

int roomdetail_SetInspiration(RoomDetail *detail, char *nameInspiration, int amt);
int roomdetail_AdjInspiration(RoomDetail *detail, char *nameInspiration, int amount);

int roomdetail_SetEnhancement(RoomDetail *detail, char *nameEnhancement, int iLevel, int iNumCombines, int amt);
int roomdetail_AdjEnhancement(RoomDetail *detail, char *pathBasePower, int iLevel, int iNumCombines, int amount);

int roomdetail_MaxCount( RoomDetail *detail);			// capacity of storage container
int roomdetail_CurrentCount( RoomDetail *detail );		// current # of items in container
bool roomdetail_CanDeleteStorage(RoomDetail *detail);

// *********************************************************************************
// client/server specific
// *********************************************************************************


#if SERVER
void ReceiveStoredSalvageAdjustFromInv(Character *p, Packet *pak);
void ReceiveStoredInspirationAdjustFromInv(Character *p, Packet *pak);
void ReceiveStoredEnhancementAdjustFromInv(Character *p, Packet *pak);
void roomdetail_AddLogMsgv(RoomDetail *d, char *msg, va_list ap);
void roomdetail_AddLogMsg(RoomDetail *d, char *msg, ...);
#endif // SERVER

#if CLIENT 
#endif // CLIENT
#endif //BASESTORAGE_H
