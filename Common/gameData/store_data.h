/***************************************************************************
 *     Copyright (c) 2003-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef STORE_DATA_H__
#define STORE_DATA_H__

#define ITEM_HASH_FROM_ITEM(item)         ((char *)((item)->ppowBase)+((item)->power.level))
#define ITEM_HASH_FROM_POWER(power,level) ((char *)(power)+(level))

typedef struct StashTableImp *StashTable;
typedef const struct StashTableImp *cStashTable;

typedef struct StoreIndex
{
	StashTable hashBuy;
	StashTable hashSell;
} StoreIndex;

typedef struct StoreHashes // Shared, generated store related hashtables
{
	StashTable hashItemsByPower;
		// Maps BasePower*+level  to StoreItem *s

	StoreIndex **StoreIndicies;
		// Indicies for looking up existance and dept of an item in a store. Unshared.
		// This is a parallel EArray to g_Stores.ppStores.
		//    i.e. g_Stores.ppStores[foo] goes with g_StoreIndicies[foo]
} StoreHashes;

extern StoreHashes g_StoreHashes;

extern StashTable g_hashStores;

extern DepartmentContents g_DepartmentContents;
extern SHARED_MEMORY Departments g_Departments;
extern SHARED_MEMORY StoreItems g_Items;
extern Stores g_Stores;

#endif /* #ifndef STORE_DATA_H__ */

/* End of File */

