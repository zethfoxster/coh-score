/***************************************************************************
 *     Copyright (c) 2003-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef STORE_H__
#define STORE_H__

#include "PowerNameRef.h"

typedef struct StashTableImp *StashTable;
typedef const struct StashTableImp *cStashTable;
typedef struct BasePower BasePower;
typedef struct Character Character;
typedef struct Entity Entity;

typedef struct StoreItem
{
	const char *pchName;
	PowerNameRef power;
	int iSell;
	int iBuy;
	int iCntPerStore;
	const int *piDepartments;

	const BasePower *ppowBase;
} StoreItem;

typedef struct StoreItems
{
	const StoreItem **ppItems;

	cStashTable hashItems; // Maps item names to StoreItem *s
} StoreItems;

typedef struct Department
{
	int id;
	const char *pchName;
} Department;

typedef struct Departments
{
	const Department **ppDepartments;
} Departments;

typedef struct DepartmentContent
{
	StoreItem **ppItems;
} DepartmentContent;

typedef struct DepartmentContents
{
	DepartmentContent **ppDepartments;
} DepartmentContents;

typedef struct Markup
{
	int iDepartment;
	float fMarkup;
} Markup;

typedef struct Store
{
	char *pchName;
	int iIdx;

	Markup **ppSells;
	Markup **ppBuys;

	const StoreItem **ppItems;

	float fBuySalvage;
	float fBuyRecipe;
} Store;

typedef struct Stores
{
	Store **ppStores;
	StashTable hashStores; // Maps store names to Store *s	
} Stores;

// This structure is both const and non-const in different circumstances
typedef struct MultiStore
{
	int idNPC; // used on the client only
	int iCnt;
	char **ppchStores;
} MultiStore;

typedef struct StoreIter StoreIter;
typedef struct MultiStoreIter MultiStoreIter;

void store_Tick(Entity *e); // server only

void load_Stores(void); // actually in store_data.c
void store_IndexByIdx(int iStore); // actually in store_data.c

Store *store_Find(const char *pch);

bool store_AreStoresLoaded(void);

const StoreItem *store_FindItem(const BasePower *ppow, int iLevel);
const StoreItem *store_FindItemByName(const char *pch);

StoreIter *storeiter_Create(Store *pstore);
StoreIter *storeiter_CreateByName(char *pch);

void storeiter_Destroy(StoreIter *piter);

const StoreItem *storeiter_First(StoreIter *piter, float *pfCost);
const StoreItem *storeiter_Next(StoreIter *piter, float *pfCost);

const StoreItem *storeiter_GetBuyPrice(StoreIter *piter, const StoreItem *psi, float *pfCost);
const StoreItem *storeiter_GetBuyItem(StoreIter *piter, const BasePower *ppowBase, int iLevel, float *pfCost);
const StoreItem *storeiter_GetSellPrice(StoreIter *piter, const StoreItem *psi, float *pfCost);
const StoreItem *storeiter_GetSellItem(StoreIter *piter, const BasePower *ppowBase, int iLevel, float *pfCost);
float storeiter_GetBuySalvageMult(StoreIter *piter);
float storeiter_GetBuyRecipeMult(StoreIter *piter);

bool store_SellItem(Store *pstore, Character *p, const StoreItem *pitem);
bool store_SellItemByName(Store *pstore, Character *p, char *pchItem);
bool store_BuyItem(Store *pstore, Character *p, int eType, int i, int j);
bool store_BuySalvage(Store *pstore, Character *p, int id, int amount);
bool store_BuyRecipe(Store *pstore, Character *p, int id, int amount);

MultiStoreIter *multistoreiter_Create(MultiStore *pmulti);
void multistoreiter_Destroy(MultiStoreIter *piter);
const StoreItem *multistoreiter_First(MultiStoreIter *piter, float *pfCost);
const StoreItem *multistoreiter_Next(MultiStoreIter *piter, float *pfCost);
const StoreItem *multostoreiter_GetBuyPrice(MultiStoreIter *piter, const StoreItem *psi, float *pfCost);
const StoreItem *multistoreiter_GetBuyItem(MultiStoreIter *piter, const BasePower *ppowBase, int iLevel, float *pfCost);
const StoreItem *multostoreiter_GetSellPrice(MultiStoreIter *piter, const StoreItem *psi, float *pfCost);
const StoreItem *multistoreiter_GetSellItem(MultiStoreIter *piter, const BasePower *ppowBase, int iLevel, float *pfCost);
float multistoreiter_GetBuySalvageMult(MultiStoreIter *piter);
float multistoreiter_GetBuyRecipeMult(MultiStoreIter *piter);

bool multistore_SellItemByName(const MultiStore *pstore, int iCntValid, Character *p, const char *pchItem);
bool multistore_BuyItem(const MultiStore *pstore, int iCntValid, Character *p, int eType, int i, int j);
bool multistore_BuyRecipe(const MultiStore *pstore, int iCntValid, Character *p, int id, int amount);
bool multistore_BuySalvage(const MultiStore *pstore, int iCntValid, Character *p, int id, int amount);



#endif /* #ifndef STORE_H__ */

/* End of File */

