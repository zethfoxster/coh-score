/***************************************************************************
 *     Copyright (c) 2003-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#include <string.h>
#include <math.h>

#include "earray.h"
#include "StashTable.h"

#include "entity.h"
#include "entPlayer.h"
#include "character_base.h"
#include "DetailRecipe.h"
#include "character_inventory.h"
#include "powers.h"

#include "store.h"
#include "store_data.h"
#include "auth/authUserData.h"

#ifdef SERVER
#include "dbcomm.h"	// for dblog() messages
#include "dbghelper.h"
#include "character_combat.h"
#include "character_net_server.h"
#include "badges_server.h"
#include "logcomm.h"
#endif

typedef struct StoreIter
{
	Store *pstore;
	StoreIndex *index;

	int iCurDept;
	int iCurIdx;
} StoreIter;

typedef struct MultiStoreIter
{
	MultiStore *pmulti;
	int iIdxStore;
	StoreIter **ppiter;
} MultiStoreIter;


/**********************************************************************func*
 * store_FindItemByName
 *
 */
const StoreItem *store_FindItemByName(const char *pch)
{
	return (const StoreItem*)stashFindPointerReturnPointerConst(g_Items.hashItems, pch);
}

/**********************************************************************func*
 * store_FindItem
 *
 */
const StoreItem *store_FindItem(const BasePower *ppow, int iLevel)
{
//	return (StoreItem *)gHashFindValue(g_StoreHashes.hashItemsByPower, ITEM_HASH_FROM_POWER(ppow, iLevel));
	StoreItem* pResult = NULL;
	if ( stashIntFindPointer(g_StoreHashes.hashItemsByPower, (int)ITEM_HASH_FROM_POWER(ppow, iLevel), &pResult) )
		return pResult;
	return NULL;
}

/**********************************************************************func*
 * store_Find
 *
 */
Store *store_Find(const char *pch)
{
	return (Store *)stashFindPointerReturnPointer(g_hashStores, pch);
}

/**********************************************************************func*
* store_AreStoresLoaded
*
*/
bool store_AreStoresLoaded(void)
{
	return g_Stores.ppStores!=NULL;
}

/**********************************************************************func*
 * storeiter_CreateByName
 *
 */
StoreIter *storeiter_CreateByName(char *pch)
{
	return storeiter_Create(store_Find(pch));
}


/**********************************************************************func*
 * storeiter_Create
 *
 */
StoreIter *storeiter_Create(Store *pstore)
{
	StoreIter *piter;

	if(pstore==NULL)
		return NULL;

	piter = (StoreIter *)calloc(sizeof(StoreIter), 1);

	piter->pstore = pstore;
	if(!g_StoreHashes.StoreIndicies[pstore->iIdx])
	{
		store_IndexByIdx(pstore->iIdx);
	}
	piter->index = g_StoreHashes.StoreIndicies[pstore->iIdx];
	piter->iCurDept = 0;
	piter->iCurIdx = -1;

	return piter;
}

/**********************************************************************func*
 * storeiter_Destroy
 *
 */
void storeiter_Destroy(StoreIter *piter)
{
	if(piter!=NULL)
	{
		free(piter);
	}
}

/**********************************************************************func*
 * storeiter_First
 *
 */
const StoreItem *storeiter_First(StoreIter *piter, float *pfCost)
{
	piter->iCurDept = 0;
	piter->iCurIdx = -1;

	return storeiter_Next(piter, pfCost);
}

/**********************************************************************func*
 * storeiter_Next
 *
 */
const StoreItem *storeiter_Next(StoreIter *piter, float *pfCost)
{
	int iCntDept = eaSize(&piter->pstore->ppSells);
	int iCnt;

	//
	// I apologize in advance for the goofiness in this code. It could be
	// more efficient, but going through the data structure change to do
	// so is more than I can stomach right now. It works, and isn't too bad. --poz
	//

	if(piter->iCurDept < iCntDept)
		iCnt = eaSize(&g_DepartmentContents.ppDepartments[piter->iCurDept]->ppItems);

	while(piter->iCurDept < iCntDept)
	{
		piter->iCurIdx++;

		if(piter->iCurIdx >= iCnt)
		{
			// Go to next department
			piter->iCurDept++;
			while(piter->iCurDept < iCntDept && !piter->pstore->ppSells[piter->iCurDept])
			{
				piter->iCurDept++;
			}
			if(piter->iCurDept >= iCntDept)
				return NULL;

			iCnt = eaSize(&g_DepartmentContents.ppDepartments[piter->iCurDept]->ppItems);
			// Start at the beginning of the list
			piter->iCurIdx = 0;
		}

		while(piter->iCurIdx<iCnt)
		{
			int iDept;
			DepartmentContent *pcontent = g_DepartmentContents.ppDepartments[piter->iCurDept];
			StoreItem *psi = pcontent->ppItems[piter->iCurIdx];

			if ( stashFindInt(piter->index->hashSell, psi, &iDept))
			{
				if(piter->iCurDept == iDept)
				{
					*pfCost = piter->pstore->ppSells[piter->iCurDept]->fMarkup;
					return psi;
				}
			}

			piter->iCurIdx++;
		}
	}

	return NULL;
}

/**********************************************************************func*
 * storeiter_GetBuyPrice
 *
 */
const StoreItem *storeiter_GetBuyPrice(StoreIter *piter, const StoreItem *psi, float *pfCost)
{
	int iDept;

	if(piter==NULL || psi==NULL || pfCost==NULL)
		return NULL;

	*pfCost = 0;

	if( stashFindInt(piter->index->hashBuy, psi, &iDept))
	{
		if(piter->pstore->ppBuys[iDept])
		{
			*pfCost = piter->pstore->ppBuys[iDept]->fMarkup;
			return psi;
		}
	}

	return NULL;
}

/**********************************************************************func*
 * storeiter_GetBuyItem
 *
 */
const StoreItem *storeiter_GetBuyItem(StoreIter *piter, const BasePower *ppowBase, int iLevel, float *pfCost)
{
	const StoreItem *psi;

	if(piter==NULL || ppowBase==NULL)
		return NULL;

	psi = store_FindItem(ppowBase, iLevel);

	return storeiter_GetBuyPrice(piter, psi, pfCost);
}


/**********************************************************************func*
* storeiter_GetBuySalvageMult
*
*/
float storeiter_GetBuySalvageMult(StoreIter *piter)
{
	if(piter==NULL || piter->pstore==NULL)
		return 0.0f;

	return piter->pstore->fBuySalvage;
}

/**********************************************************************func*
* storeiter_GetBuySalvagePrice
*
*/
float storeiter_GetBuyRecipeMult(StoreIter *piter)
{
	if(piter==NULL || piter->pstore==NULL)
		return 0.0f;

	return piter->pstore->fBuyRecipe;
}

/**********************************************************************func*
 * storeiter_GetSellPrice
 *
 */
const StoreItem *storeiter_GetSellPrice(StoreIter *piter, const StoreItem *psi, float *pfCost)
{
	int iDept;

	if(piter==NULL || psi==NULL)
		return NULL;

	if(stashFindInt(piter->index->hashSell, psi, &iDept))
	{
		if(piter->pstore->ppSells[iDept])
		{
			*pfCost = piter->pstore->ppSells[iDept]->fMarkup;
			return psi;
		}
	}

	return NULL;
}

/**********************************************************************func*
 * storeiter_GetSellItem
 *
 */
const StoreItem *storeiter_GetSellItem(StoreIter *piter, const BasePower *ppowBase, int iLevel, float *pfCost)
{
	const StoreItem *psi;

	if(piter==NULL || ppowBase==NULL)
		return NULL;

	psi = store_FindItem(ppowBase, iLevel);

	return storeiter_GetSellPrice(piter, psi, pfCost);
}

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

//
// I apologize even more for the half-assed MultiStoreIter thing.
// All I have to say is that it works. --poz
//

/**********************************************************************func*
 * multistoreiter_Create
 *
 */
MultiStoreIter *multistoreiter_Create(MultiStore *pmulti)
{
	MultiStoreIter *piter;

	if(pmulti==NULL)
		return NULL;

	piter = (MultiStoreIter *)calloc(sizeof(MultiStoreIter), 1);
	piter->ppiter=(StoreIter **)calloc(sizeof(StoreIter *), pmulti->iCnt);

	piter->iIdxStore = -1;
	piter->pmulti = pmulti;

	return piter;
}

/**********************************************************************func*
 * multistoreiter_Destroy
 *
 */
void multistoreiter_Destroy(MultiStoreIter *piter)
{
	if(piter!=NULL)
	{
		if(piter->ppiter)
		{
			int i;
			for(i=0; i<piter->pmulti->iCnt; i++)
			{
				if(piter->ppiter[i])
					storeiter_Destroy(piter->ppiter[i]);
			}
			free(piter->ppiter);
		}
		free(piter);
	}
}

/**********************************************************************func*
 * multistoreiter_First
 *
 */
const StoreItem *multistoreiter_First(MultiStoreIter *piter, float *pfCost)
{
	piter->iIdxStore = -1;
	return multistoreiter_Next(piter, pfCost);
}

/**********************************************************************func*
 * multistoreiter_Next
 *
 */
const StoreItem *multistoreiter_Next(MultiStoreIter *piter, float *pfCost)
{
	const StoreItem *pRet = NULL;

	if(piter->iIdxStore<0
		|| !piter->ppiter[piter->iIdxStore]
		|| (pRet=storeiter_Next(piter->ppiter[piter->iIdxStore], pfCost))==NULL)
	{
		while(piter->iIdxStore<(piter->pmulti->iCnt-1) && pRet==NULL)
		{
			piter->iIdxStore++;
			if(!piter->ppiter[piter->iIdxStore])
			{
				piter->ppiter[piter->iIdxStore] = storeiter_CreateByName(piter->pmulti->ppchStores[piter->iIdxStore]);
			}

			pRet = storeiter_First(piter->ppiter[piter->iIdxStore], pfCost);
		}
	}

	return pRet;
}

/**********************************************************************func*
* multistoreiter_GetBuySalvageMult
*
*/
float multistoreiter_GetBuySalvageMult(MultiStoreIter *piter)
{
	int i;

	if(piter==NULL)
		return 0.0f;

	for(i=0; i<piter->pmulti->iCnt; i++)
	{
		StoreIter *pstoreiter = storeiter_CreateByName(piter->pmulti->ppchStores[i]);
		float fPrice = storeiter_GetBuySalvageMult(pstoreiter);
		storeiter_Destroy(pstoreiter);
		return fPrice;
	}

	return 0.0f;
}

/**********************************************************************func*
* multistoreiter_GetBuyRecipeMult
*
*/
float multistoreiter_GetBuyRecipeMult(MultiStoreIter *piter)
{
	int i;

	if(piter==NULL)
		return 0.0;

	for(i=0; i<piter->pmulti->iCnt; i++)
	{
		StoreIter *pstoreiter = storeiter_CreateByName(piter->pmulti->ppchStores[i]);
		float fPrice = storeiter_GetBuyRecipeMult(pstoreiter);
		storeiter_Destroy(pstoreiter);
		return fPrice;
	}

	return 0.0f;
}

/**********************************************************************func*
 * multistoreiter_GetBuyPrice
 *
 */
const StoreItem *multistoreiter_GetBuyPrice(MultiStoreIter *piter, const StoreItem *psi, float *pfCost)
{
	int i;

	if(piter==NULL || psi==NULL)
		return NULL;

	for(i=0; i<piter->pmulti->iCnt; i++)
	{
		StoreIter *pstoreiter = storeiter_CreateByName(piter->pmulti->ppchStores[i]);
		const StoreItem *pItem = storeiter_GetBuyPrice(pstoreiter, psi, pfCost);
		storeiter_Destroy(pstoreiter);
		if(pItem)
			return pItem;
	}

	return NULL;
}



/**********************************************************************func*
 * multistoreiter_GetBuyItem
 *
 */
const StoreItem *multistoreiter_GetBuyItem(MultiStoreIter *piter, const BasePower *ppowBase, int iLevel, float *pfCost)
{
	const StoreItem *psi;

	if(piter==NULL || ppowBase==NULL)
		return NULL;

	psi = store_FindItem(ppowBase, iLevel);

	return multistoreiter_GetBuyPrice(piter, psi, pfCost);
}

/**********************************************************************func*
 * multistoreiter_GetSellPrice
 *
 */
static const StoreItem *multistoreiter_GetSellPrice(MultiStoreIter *piter, const StoreItem *psi, float *pfCost)
{
	int i;

	if(piter==NULL || psi==NULL)
		return NULL;

	for(i=0; i<piter->pmulti->iCnt; i++)
	{
		StoreIter *pstoreiter = storeiter_CreateByName(piter->pmulti->ppchStores[i]);
		const StoreItem *pItem = storeiter_GetSellPrice(pstoreiter, psi, pfCost);
		storeiter_Destroy(pstoreiter);
		if(pItem)
			return pItem;
	}

	return NULL;
}

/**********************************************************************func*
 * multistoreiter_GetSellItem
 *
 */
const StoreItem *multistoreiter_GetSellItem(MultiStoreIter *piter, const BasePower *ppowBase, int iLevel, float *pfCost)
{
	const StoreItem *psi;

	if(piter==NULL || ppowBase==NULL)
		return NULL;

	psi = store_FindItem(ppowBase, iLevel);

	return multistoreiter_GetSellPrice(piter, psi, pfCost);
}

/**********************************************************************func*
 * store_SellItem
 *
 */
bool store_SellItem(Store *pstore, Character *p, const StoreItem *pitem)
{
	const StoreItem *pitemStore;
	StoreIter *piter;
	float f;
	int iCost;

	if(!pstore || !p || !pitem)
		return false;

	piter = storeiter_Create(pstore);
	pitemStore = storeiter_GetSellPrice(piter, pitem, &f);
	storeiter_Destroy(piter);

	if(pitemStore)
	{
		iCost = ceil(pitem->iSell*f);
		if(p->iInfluencePoints >= iCost)
		{
			int iRet = -1;

			if(pitem->ppowBase->eType==kPowerType_Inspiration)
			{
				iRet = character_AddInspiration(p, pitem->ppowBase, "store");
			}
			else if(pitem->ppowBase->eType==kPowerType_Boost)
			{
				iRet = character_AddBoost(p, pitem->ppowBase, pitem->power.level, 0, "store");
			}
			else 
			{
				iRet = character_AddRewardPower(p, pitem->ppowBase) ? 1 : -1;
			}

			if(iRet>=0)
			{
				ent_AdjInfluence(p->entParent, -iCost, NULL);
#ifdef SERVER
				badge_StatAdd(p->entParent, "inf.storebuy", iCost);
				LOG_ENT(p->entParent, LOG_ENTITY, LOG_LEVEL_IMPORTANT, 0, "Store:Purchase %s (Cost %d)", dbg_BasePowerStr(pitem->ppowBase), iCost);
#endif
			}

			return true;
		}
	}

	return false;
}

/**********************************************************************func*
 * store_SellItemByName
 *
 */
bool store_SellItemByName(Store *pstore, Character *p, char *pchItem)
{
	return store_SellItem(pstore, p, store_FindItemByName(pchItem));
}


/**********************************************************************func*
 * multistore_SellItemByName
 *
 */
bool multistore_SellItemByName(const MultiStore *pstore, int iCntValid, Character *p, const char *pchItem)
{
	int i;
	const StoreItem *pitem;

	if(!pstore || !p || !pchItem)
		return false;

	pitem = store_FindItemByName(pchItem);
	if(pitem)
	{
		iCntValid = iCntValid > pstore->iCnt ? pstore->iCnt : iCntValid;
		for(i=0; i<iCntValid; i++)
		{
			Store *pstoreSingle = store_Find(pstore->ppchStores[i]);
			if(store_SellItem(pstoreSingle, p, pitem))
			{
				return true;
			}
		}
	}

	return false;
}

/**********************************************************************func*
* store_BuySalvage
*
*/
bool store_BuySalvage(Store *pstore, Character *p, int id, int amount)
{
#ifdef SERVER
	int iCost;
	const SalvageItem *pSal = salvage_GetItemById(id);
	int iLevel = 0;

	if(pstore==NULL || p==NULL || pSal==NULL || amount==0)
		return false;

	if (character_CanAdjustSalvage(p, pSal, -amount))
	{
		StoreIter *piter = storeiter_Create(pstore);
		float mult = storeiter_GetBuySalvageMult(piter);
		storeiter_Destroy(piter);

		iCost = ceil(pSal->sellAmount*mult)*amount;
		
		if (iCost <= 0)
			return false;

		ent_AdjInfluence(p->entParent, iCost, NULL);

		character_AdjustSalvage(p, pSal, -amount, "store", false);

		LOG_ENT(p->entParent, LOG_ENTITY, LOG_LEVEL_IMPORTANT, 0, "Store:Sell %s (Paid %d)", pSal->pchName, iCost);
		badge_StatAdd(p->entParent, "inf.storesell.salvage", iCost);

		return true;
	}
#endif

	return false;
}

/**********************************************************************func*
* store_BuySalvage
*
*/
bool store_BuyRecipe(Store *pstore, Character *p, int id, int amount)
{
#ifdef SERVER
	int iCost;
	const DetailRecipe *pRec = recipe_GetItemById(id);
	int iLevel = 0;

	if(pstore==NULL || p==NULL || pRec==NULL || amount==0)
		return false;

	if (character_CanRecipeBeChanged(p, pRec, -amount))
	{
		StoreIter *piter = storeiter_Create(pstore);
		float mult = storeiter_GetBuySalvageMult(piter);
		storeiter_Destroy(piter);

		iCost = ceil(pRec->SellToVendor*mult)*amount;

		if (iCost <= 0)
			return false;

		ent_AdjInfluence(p->entParent, iCost, NULL);

		character_AdjustRecipe(p, pRec, -amount, "store");

		badge_StatAdd(p->entParent, "inf.storesell.recipe", iCost);
		LOG_ENT(p->entParent, LOG_ENTITY, LOG_LEVEL_IMPORTANT, 0, "Store:Sell %s (Paid %d)", pRec->pchName, iCost);
		return true;
	}
#endif

	return false;
}

/**********************************************************************func*
 * store_BuyItem
 *
 */
bool store_BuyItem(Store *pstore, Character *p, int eType, int i, int j)
{
	float f;
	int iCost;
	const BasePower *ppowBase = NULL;
	int iLevel = 0;

	if(pstore==NULL || p==NULL)
		return false;

	if(eType==kPowerType_Inspiration)
	{
		if(i>=0 && i<p->iNumInspirationRows
			&& j>=0 && j<p->iNumInspirationCols )
		{
			ppowBase = p->aInspirations[j][i];
		}
	}
	else if(eType==kPowerType_Boost)
	{
		if(i>=0 && i<CHAR_BOOST_MAX && p->aBoosts[i]!=NULL)
		{
			ppowBase = p->aBoosts[i]->ppowBase;
			iLevel = p->aBoosts[i]->iLevel;
		}
	}

	if(ppowBase)
	{
		const StoreItem *pitem = store_FindItem(ppowBase, iLevel);

		if(pitem)
		{
			StoreIter *piter = storeiter_Create(pstore);
			const StoreItem *pitemPrice = storeiter_GetBuyPrice(piter, pitem, &f);
			storeiter_Destroy(piter);

			if(pitemPrice!=NULL)
			{
				iCost = ceil(pitem->iBuy*f);

				ent_AdjInfluence(p->entParent, iCost, NULL);

				if(eType==kPowerType_Inspiration)
				{
					character_RemoveInspiration(p, j, i, "store");
				}
				else if(eType==kPowerType_Boost)
				{
					character_RemoveBoost(p, i, "store");
				}
#ifdef SERVER
				badge_StatAdd(p->entParent, "inf.storesell.enh", iCost);
				LOG_ENT(p->entParent, LOG_ENTITY, LOG_LEVEL_IMPORTANT, 0, "Store:Sell %s (Paid %d)", dbg_BasePowerStr(ppowBase), iCost);
#endif
				return true;
			}
		}
	}

	return false;
}

/**********************************************************************func*
 * multistore_BuyItem
 *
 */
bool multistore_BuyItem(const MultiStore *pstore, int iCntValid, Character *p, int eType, int i, int j)
{
	int k;

	if(!pstore || !p)
		return false;

	iCntValid = iCntValid > pstore->iCnt ? pstore->iCnt : iCntValid;
	for(k=0; k<iCntValid; k++)
	{
		Store *pstoreSingle = store_Find(pstore->ppchStores[k]);
		if(store_BuyItem(pstoreSingle, p, eType, i, j))
		{
			return true;
		}
	}

	return false;
}


/**********************************************************************func*
* multistore_BuySalvage
*
*/
bool multistore_BuySalvage(const MultiStore *pstore, int iCntValid, Character *p, int id, int amount)
{
	int k;

	if(!pstore || !p)
		return false;

	iCntValid = iCntValid > pstore->iCnt ? pstore->iCnt : iCntValid;
	for(k=0; k<iCntValid; k++)
	{
		Store *pstoreSingle = store_Find(pstore->ppchStores[k]);
		if(store_BuySalvage(pstoreSingle, p, id, amount))
		{
			return true;
		}
	}

	return false;
}


/**********************************************************************func*
* multistore_BuyRecipe
*
*/
bool multistore_BuyRecipe(const MultiStore *pstore, int iCntValid, Character *p, int id, int amount)
{
	int k;

	if(!pstore || !p)
		return false;

	iCntValid = iCntValid > pstore->iCnt ? pstore->iCnt : iCntValid;
	for(k=0; k<iCntValid; k++)
	{
		Store *pstoreSingle = store_Find(pstore->ppchStores[k]);
		if(store_BuyRecipe(pstoreSingle, p, id, amount))
		{
			return true;
		}
	}

	return false;
}

#ifdef SERVER
#include "entserver.h"
#include "svr_base.h"
#include "comm_game.h"
#include "storyarcprivate.h"

/**********************************************************************func*
 * store_Tick
 *
 */
void store_Tick(Entity *e)
{
	Entity *npc = NULL;
	bool bCloseStore = false;

	// Are we using an auction NPC as opposed to a regular NPC?
	if (e->pl && e->pl->at_auction)
	{
		// We have to know which NPC the player clicked on in order to
		// determine their current distance.
		verify(e->pl->npcStore > 0);

		npc = entFromId(e->pl->npcStore);

		// The auction window follows the same auto-close proximity
		// rule as the regular NPC/kiosk store windows.
		if ((!npc || distance3Squared(ENTPOS(npc), ENTPOS(e)) > SQR(20)) && !accountLoyaltyRewardHasNode(e->pl->loyaltyStatus, "RemoteAuction") )
		{
			character_toggleAuctionWindow(e, 0, 0);
		}
	}
	else
	{
		if(e->pl->npcStore>0)
		{
			npc = entFromId(e->pl->npcStore);

			if (!npc) 
				bCloseStore = true;
			else
				bCloseStore = (distance3Squared(ENTPOS(npc), ENTPOS(e))>SQR(20));
		}
		else if(e->pl->npcStore<0)
		{
			bCloseStore = (e->storyInfo->interactTarget != -e->pl->npcStore);
		}

		if(bCloseStore)
		{
			e->pl->npcStore = 0;
			START_PACKET(pak, e, SERVER_STORE_CLOSE);
			END_PACKET
		}
	}
}
#endif

#ifdef DEBUG_TEST
/**********************************************************************func*
 * dbg_TestIter
 *
 */
void dbg_TestIter(void)
{
	StoreIter *piter;
	StoreItem *psi;
	float f;

	piter = storeiter_CreateByName("Inspirations");
	psi = storeiter_First(piter, &f);
	while(psi)
	{
		verbose_printf("%s: %f\n", psi->pchName, f);
		psi = storeiter_Next(piter, &f);
	}
	storeiter_Destroy(piter);
}
#endif

/* End of File */
