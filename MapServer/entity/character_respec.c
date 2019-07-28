/***************************************************************************
 *     Copyright (c) 2004-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#include "netio.h"
#include "earray.h"

#include "classes.h"
#include "origins.h"
#include "character_base.h"
#include "character_level.h"
#include "character_combat.h"
#include "powers.h"
#include "character_net.h"
#include "character_net_server.h"
#include "entity.h" // Just to get at e->auth_name for printing cheater info.  Better solution is to make a cheaterLog function that takes an entity
#include "storyarcprivate.h" // For contact stuff
#include "PowerInfo.h"	// for character respec
#include "dbcomm.h"  // for logging
#include "dbghelper.h" // for logging
#include "comm_game.h"
#include "entPlayer.h"
#include "Reward.h"
#include "svr_player.h"
#include "costume.h" // for respecs that change powersets
#include "varutils.h"

#include "TeamReward.h"
#include "badges_server.h"
#include "logcomm.h"

#define MAX_BOOST_SLOTS 250

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

#define PATRON_COUNT			4

typedef struct respecPatronData {
	char			*badgeName;
} respecPatronData;

respecPatronData respec_Patrons[PATRON_COUNT] =
{
	{ "SpidersKissPatron" },
	{ "MiragePatron" },
	{ "BloodInTheWaterPatron" },
	{ "TheStingerPatron" },
};

static int respecFindPatron(Entity *pEntity)
{
	int i, idx;

	if (!pEntity)
		return -1;

	for (i = 0; i < PATRON_COUNT; i++)
	{
		badge_GetIdxFromName(respec_Patrons[i].badgeName, &idx);

		if (idx && BADGE_IS_OWNED(pEntity->pl->aiBadges[idx]))
			return i;
	}

	return -1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
typedef struct RespecBoost
{
	// where its going to
	bool bToInventory;
	bool bToPile;

	int	iToIdx;
	const BasePower *pToPow;

	// where its from
	bool bFromInventory;
	int iset;
	int ipow;
	int ispec;

} RespecBoost;

// returns the idx of the found boost (-1 for no boost found)
//
static int character_FindRespecBoost( RespecBoost *findme, RespecBoost **ppList )
{
	int i;

	for( i = eaSize( &ppList )-1; i >= 0; i-- )
	{
		if( findme->bFromInventory && ppList[i]->bFromInventory )
		{
			if( findme->ispec == ppList[i]->ispec )
				return i;
		}
		else if( !findme->bFromInventory && !ppList[i]->bFromInventory )
		{
			if( findme->iset == ppList[i]->iset &&
				findme->ipow == ppList[i]->ipow &&
				findme->ispec == ppList[i]->ispec )
			{
				return i;
			}
		}
	}

	return -1;
}

static Boost *character_getBoostFromIndex( Character *pchar, RespecBoost **ppList, int idx )
{
	if( idx < 0 )
		return NULL;

	if( ppList[idx]->bFromInventory )
	{
		return pchar->aBoosts[ppList[idx]->ispec];
	}
	else
	{
		return pchar->ppPowerSets[ppList[idx]->iset]->ppPowers[ppList[idx]->ipow]->ppBoosts[ppList[idx]->ispec];
	}

}


static bool respec_VerifyPowers(Entity * e, Character * tchar, BasePower ***pppPowers, int iExpLevel, Character *oldCharacter)
{
	int i,k;
	int iPowerCount = eaSize(pppPowers);


	/* Log list of initial powers *********************************************/
	for( i = 0; i < eaSize( &tchar->ppPowerSets ); i++ )
	{
		PowerSet * pset = tchar->ppPowerSets[i];
		if(pset)
		{
			for( k = 0; k < eaSize( &pset->ppPowers ); k++)
			{
				if(pset->ppPowers[k])
				{
					LOG_ENT(e, LOG_ENTITY, LOG_LEVEL_DEBUG, 0, "Respec Initial Power: %s %s",
						pset->psetBase->pchName,
						pset->ppPowers[k]->ppowBase->pchName);
				}
			}
		}
	}


	// loop through all powers in order that they were bought, leveling the character up as we go.
	tchar->iLevel = 0;
	for( i = 0; i < iPowerCount && tchar->iLevel < iExpLevel; i++ )
	{
		BasePower * ppowBase = (*pppPowers)[i];

		if( ! ppowBase)
		{
			LOG_ENT( e, LOG_ENTITY, LOG_LEVEL_DEBUG, 0, "Respec Null/bad power sent to server! (index %d)", i);
			return false;
		}

		// increase characters level until the next level that receives a power
		while( tchar->iLevel < iExpLevel
			&& 0 >= character_CountPowersForLevel(tchar, tchar->iLevel))
		{
			tchar->iLevel++;
		}

		// try to buy the power

		if(    ppowBase!=NULL
			&& character_CanBuyPower(tchar)
			&& character_IsAllowedToHavePowerAtSpecializedLevel(tchar, iExpLevel, ppowBase))
		{
			PowerSet *pset;

			if(character_OwnsPower(tchar, ppowBase)==NULL)
			{
				if((pset=character_OwnsPowerSet(tchar, ppowBase->psetParent))==NULL)
				{
					bool bBoughtPower = false;
					// The character doesn't own the set yet.
					if(baseset_IsSpecialization(ppowBase->psetParent)
						&& character_IsAllowedToHavePowerSetHypotheticallyAtSpecializedLevel(tchar, iExpLevel, ppowBase->psetParent, NULL))
					{
						// Specialization sets are only supposed to be added to primaries or secondaries by design.
						if(ppowBase->psetParent->pcatParent==tchar->pclass->pcat[kCategory_Primary]
							|| ppowBase->psetParent->pcatParent==tchar->pclass->pcat[kCategory_Secondary])
						{
							pset = character_BuyPowerSet(tchar, ppowBase->psetParent);
							LOG_ENT( e, LOG_ENTITY, LOG_LEVEL_DEBUG, 0, "Respec Bought power set %s\n", ppowBase->psetParent->pchName);
							bBoughtPower = true;
						}
					}

					if(character_CanBuyPoolPowerSet(tchar)
						&& character_IsAllowedToHavePowerSetHypothetically(tchar, ppowBase->psetParent))
					{
						// You can get new power sets only from the pool.
						if(ppowBase->psetParent->pcatParent==tchar->pclass->pcat[kCategory_Pool])
						{
							pset = character_BuyPowerSet(tchar, ppowBase->psetParent);
							LOG_ENT( e, LOG_ENTITY, LOG_LEVEL_DEBUG, 0, "Respec Bought power set %s\n", ppowBase->psetParent->pchName);
							bBoughtPower = true;
						}
					}

					if(!bBoughtPower
						&& character_CanBuyEpicPowerSet(tchar)
						&& character_IsAllowedToHavePowerSetHypothetically(tchar, ppowBase->psetParent))
					{
						// You can get new power sets only from the epic pool.
						if(ppowBase->psetParent->pcatParent==tchar->pclass->pcat[kCategory_Epic])
						{
							pset = character_BuyPowerSet(tchar, ppowBase->psetParent);
							LOG_ENT( e, LOG_ENTITY, LOG_LEVEL_DEBUG, 0, "Respec Bought power set %s\n", ppowBase->psetParent->pchName);
							bBoughtPower = true;
						}
					}

					if(!bBoughtPower)
					{
						LOG_ENT( e, LOG_ENTITY, LOG_LEVEL_DEBUG, 0, "Respec Buying power set %s disallowed (owns %d, can get %d)", ppowBase->psetParent->pchName, character_CountPoolPowerSetsBought(tchar), CountForLevel(tchar->iLevel, g_Schedules.aSchedules.piPoolPowerSet));
					}
				}

				if(pset!=NULL)
				{
					// Respecs assume that all powers referenced in a respec are NumAllowed 1.
					// If this assumption isn't true, the code breaks down here.
					Power *oldPower = character_OwnsPower(oldCharacter, ppowBase);

					// BuyPower is a command; it doesn't do any level checks.
					if (character_BuyPowerEx(tchar, pset, ppowBase, NULL, 0, oldPower ? oldPower->iUniqueID : 0, oldCharacter)!=NULL)
					{
						tchar->entParent->general_update = 1;
						character_LevelFinalize(tchar, true);

						LOG_ENT(e, LOG_ENTITY, LOG_LEVEL_DEBUG, 0, "Respec Bought power %s\n", dbg_BasePowerStr(ppowBase));
					}
					else
					{
						LOG_ENT(e, LOG_ENTITY, LOG_LEVEL_DEBUG, 0, "Respec Error: Unable to buy power %s", dbg_BasePowerStr(ppowBase));
						return false;
					}
				}
				else
				{
					LOG_ENT(e, LOG_ENTITY, LOG_LEVEL_DEBUG, 0, "Respec Error: Unable to buy power set %s\n", ppowBase->psetParent->pchName);
					return false;
				}
			}
			else
			{
				LOG_ENT(e, LOG_ENTITY, LOG_LEVEL_DEBUG, 0, "Respec Error: Character attempted to buy power that they already own: %s", ppowBase->pchName);
				return false;
			}
		}
	} // end of buy-powers for-loop


	tchar->iLevel = iExpLevel;

	character_LevelFinalize(tchar, true);

	return true;
}


static bool respec_VerifyBoosts(Entity * e, Character * tchar, Character * pCharOld, RespecBoost*** pppCurrentBoosts, RespecBoost*** pppRespecBoosts, int iExpLevel, BasePower* aBoostSlots[MAX_BOOST_SLOTS], int iSlotCount, int * piPileValue)
{
	int i,j,k;
	int iSize;

	(*piPileValue) = 0;

	// add empty boost slots to new character's powers

	if(iSlotCount != character_CountBuyBoostForLevel(tchar, iExpLevel) )
	{
		LOG_ENT( e, LOG_ENTITY, LOG_LEVEL_VERBOSE, 0, "Respec Incorrect # of boost slots! (attempted %d, should be %d)", iSlotCount, character_CountBuyBoostForLevel(tchar, iExpLevel));
		return false;
	}

	tchar->iLevel = 0;
	for( i = 0; i < iSlotCount; i++ )
	{
		bool bFoundPower = false;

		// find our level -- skip forward through levels that don't add boost slots
		while(   tchar->iLevel < iExpLevel
			&& 0 >= character_CountBuyBoostForLevel(tchar, tchar->iLevel))
		{
			tchar->iLevel++;
		}

		// first find the power, assuming that temporary powers are in index 0 and cannot have boosts assigned to them (at least here)
		for( j = eaSize(&tchar->ppPowerSets)-1; j > 0; j-- ) // intentionally > as opposed to >=
		{
			for( k = eaSize(&tchar->ppPowerSets[j]->ppPowers)-1; k >= 0; k-- )
			{
				if( !bFoundPower && aBoostSlots[i] == tchar->ppPowerSets[j]->ppPowers[k]->ppowBase )
				{
					// we have a valid target power
					Power *ppow = tchar->ppPowerSets[j]->ppPowers[k];

					if( !character_BuyBoostSlot( tchar, ppow ) )
					{
						LOG_ENT( e, LOG_ENTITY, LOG_LEVEL_VERBOSE, 0, "Respec Character is not allowed to buy boost slot for power %s", ppow->ppowBase->pchName);
						return false;
					}

					bFoundPower = true;
				}
			}
		}

		if(!bFoundPower)
		{
			LOG_ENT( e, LOG_ENTITY, LOG_LEVEL_VERBOSE, 0, "Respec Could not find power for boost slot");
			return false;
		}
	}

	// set to current level
	tchar->iLevel = iExpLevel;

	// create list of old boosts -- this is server data, so is assumed to be correct

	// first search inventory
	for( i = 0; i < CHAR_BOOST_MAX; i++ )
	{
		if( pCharOld->aBoosts[i] && pCharOld->aBoosts[i]->ppowBase )
		{
			RespecBoost *prb = calloc( 1, sizeof(RespecBoost) );
			prb->bFromInventory = true;
			prb->ispec = i;

			LOG_ENT( e, LOG_ENTITY, LOG_LEVEL_VERBOSE, 0, "Respec Inventory Slot %d, Boost %s\n", prb->ispec, pCharOld->aBoosts[i]->ppowBase->pchName);
			eaPush( pppCurrentBoosts, prb );
		}
	}

	//now search powers -- this is server data, so is assumed to be correct
	for( i = eaSize(&pCharOld->ppPowerSets)-1; i >= 0; i-- )
	{
		for( j = eaSize(&pCharOld->ppPowerSets[i]->ppPowers)-1; j >= 0; j-- )
		{
			Power * ppow = pCharOld->ppPowerSets[i]->ppPowers[j];
			if(ppow)
			{
				for( k = eaSize(&ppow->ppBoosts)-1; k >= 0; k-- )
				{
					if( ppow->ppBoosts[k] && ppow->ppBoosts[k]->ppowBase )
					{
						RespecBoost *prb = calloc( 1, sizeof(RespecBoost) );

						prb->ispec = k;
						prb->ipow = j;
						prb->iset = i;

						LOG_ENT( e, LOG_ENTITY, LOG_LEVEL_VERBOSE, 0, "Respec Power %s, Boost %s,  (ispec %d, ipow %d, iset %d)\n", ppow->ppowBase->pchName, ppow->ppBoosts[k]->ppowBase->pchName, prb->ispec, prb->ipow, prb->iset);
						eaPush( pppCurrentBoosts, prb );
					}
				}
			}
		}
	}

	LOG_ENT( e, LOG_ENTITY, LOG_LEVEL_VERBOSE, 0, "Respec Current boost count: %d\n", eaSize(pppCurrentBoosts));
	LOG_ENT( e, LOG_ENTITY, LOG_LEVEL_VERBOSE, 0, "Respec New respec boost count: %d\n", eaSize(pppRespecBoosts));

	// debug -- print # of slots per power
	//now search powers
	for( i = eaSize(&tchar->ppPowerSets)-1; i >= 0; i-- )
	{
		for( j = eaSize(&tchar->ppPowerSets[i]->ppPowers)-1; j >= 0; j-- )
		{
			Power * pow = tchar->ppPowerSets[i]->ppPowers[j];
			LOG_ENT( e, LOG_ENTITY, LOG_LEVEL_DEBUG, 0, "Respec Power %s has %d slots available (%d slots bought)\n",  pow->ppowBase->pchName, eaSize(&pow->ppBoosts), pow->iNumBoostsBought);
		}
	}

	if(eaSize(pppCurrentBoosts) != eaSize(pppRespecBoosts))
	{
		LOG_ENT( e, LOG_ENTITY, LOG_LEVEL_DEBUG, 0, "Respec Sent boost count (%d) does not match current count (%d)", eaSize(pppRespecBoosts), eaSize(pppCurrentBoosts));
		return false;
	}

	iSize = eaSize(pppRespecBoosts);
	for( i = 0; i < iSize; i++ )
	{
		RespecBoost * prb = (*pppRespecBoosts)[i];

		int index = character_FindRespecBoost( prb, (*pppCurrentBoosts) );
		Boost * pBoost = character_getBoostFromIndex(pCharOld, (*pppCurrentBoosts), index );

		if( index < 0 || !pBoost )
		{
			LOG_ENT( e, LOG_ENTITY, LOG_LEVEL_DEBUG, 0, "Respec Invalid boost -- respec boost was not found on current character");
			return false;
		}

		if( prb->bToPile ) // if its going to pile find it in original list and delete it.
		{
			LOG_ENT( e, LOG_ENTITY, LOG_LEVEL_IMPORTANT, 0, "Respec Deleted boost %s\n", pBoost->ppowBase->pchName);
		}
		else if ( prb->bToInventory ) // find it in original list, if its there add it to new char
		{
			if( character_AddBoostIgnoreLimit( tchar, pBoost->ppowBase, pBoost->iLevel, pBoost->iNumCombines, NULL ) >= 0 )
			{
				LOG_ENT( e, LOG_ENTITY, LOG_LEVEL_DEBUG, 0, "Respec Placed boost %s into inventory\n", pBoost->ppowBase->pchName);
			}
			else
			{
				LOG_ENT( e, LOG_ENTITY, LOG_LEVEL_DEBUG, 0, "Respec Unable to add boost %s", pBoost->ppowBase->pchName);
				return false;
			}
		}
		else if ( prb->pToPow ) // ok its going to a power
		{
			bool bFoundPower = false;
			// first find the power
			for( j = eaSize(&tchar->ppPowerSets)-1; j >= 0; j-- )
			{
				for( k = eaSize(&tchar->ppPowerSets[j]->ppPowers)-1; k >= 0; k-- )
				{
					if( !bFoundPower && prb->pToPow == tchar->ppPowerSets[j]->ppPowers[k]->ppowBase )
					{
						// found the power

						if(   prb->iToIdx >= eaSize(&tchar->ppPowerSets[j]->ppPowers[k]->ppBoosts)
							|| prb->iToIdx < 0)
						{
							LOG_ENT( e, LOG_ENTITY, LOG_LEVEL_DEBUG, 0, "Respec Bad boost slot specified (%s, wants to go in slot %d, only %d slots available", pBoost->ppowBase->pchName, prb->iToIdx, eaSize(&tchar->ppPowerSets[j]->ppPowers[k]->ppBoosts));
							return false;
						}

						if(!power_IsValidBoost(tchar->ppPowerSets[j]->ppPowers[k],pBoost))
						{
							LOG_ENT( e, LOG_ENTITY, LOG_LEVEL_DEBUG, 0, "Respec Bad boost for power or character (%s in power %s)\n", pBoost->ppowBase->pchName, tchar->ppPowerSets[j]->ppPowers[k]->ppowBase->pchName);
							return false;
						}

						{
							int iValidToIdx = power_ValidBoostSlotRequired(tchar->ppPowerSets[j]->ppPowers[k],pBoost);
							if(iValidToIdx>0 && iValidToIdx != prb->iToIdx)
							{
								LOG_ENT( e, LOG_ENTITY, LOG_LEVEL_DEBUG, 0, "Respec Invalid slot %d for boost %s in power %s (valid at %d)\n", prb->iToIdx, pBoost->ppowBase->pchName, tchar->ppPowerSets[j]->ppPowers[k]->ppowBase->pchName, iValidToIdx);
								return false;
							}
						}

						bFoundPower = true;
						power_ReplaceBoostFromBoost( tchar->ppPowerSets[j]->ppPowers[k], 
													 prb->iToIdx,
													 pBoost, NULL );

						LOG_ENT( e, LOG_ENTITY, LOG_LEVEL_DEBUG, 0, "Respec Placed boost %s into power %s\n", pBoost->ppowBase->pchName, tchar->ppPowerSets[j]->ppPowers[k]->ppowBase->pchName);
					}
				}
			}

			if(!bFoundPower)
			{
				LOG_ENT( e, LOG_ENTITY, LOG_LEVEL_DEBUG, 0, "Respec Could not find power");
				return false;
			}

		}
		else
		{
			LOG_ENT( e, LOG_ENTITY, LOG_LEVEL_DEBUG, 0, "Respec Boost %s does not have a destination location type!", pBoost->ppowBase->pchName);
			return false;
		}

		// if we fall through, we must have placed a power somewhere, so we'll mark it off as done...
		prb = eaRemove(pppCurrentBoosts, index);
		assert(prb);
		if(prb)
			free(prb);
	}

	// if we finished for-loop without error, make sure all boosts were accounted for
	if(eaSize(pppCurrentBoosts))
	{
		LOG_ENT( e, LOG_ENTITY, LOG_LEVEL_DEBUG, 0, "Respec Did not account for all boosts on current character (%d left)", eaSize(pppCurrentBoosts));
		return false;
	}


	// we made it!
	return true;
}




//
// Get info from client & perform basic validation. More in depth validation will come later
//
static bool respec_receiveInfo(Packet * pak, Entity *e, int iExpLevel, Character *pcharOrig, const BasePower ***pppPowers, RespecBoost ***pppRespecBoosts, 
							   RespecBoost ***pppCurrentBoosts, const BasePower* aBoostSlots[MAX_BOOST_SLOTS], int * piSlotCount, int *patronID)
{
	int iPowerCount, iPileBoosts, iPowerBoosts;
	int i;
	bool bought_primary = false;
	bool bought_secondary = false;
	int bPatron, iPatronID;

	/* Receive Character ****************************************************/

	// this gets the new class and origin
	if( !character_ReceiveCreateClassOrigin(pak, e->pchar, e, pcharOrig->pclass, pcharOrig->porigin) )
	{
		//	If this fails, we are going to bail out entirely
		//	Ignore the rest of the packet and just let it drop, since we don't know what type of character to make
		//	Since this character would not pass validation anyways
		//	There really isn't any point in going further
		LOG_ENT( e, LOG_ENTITY, LOG_LEVEL_DEBUG, 0, "Respec Failed Character Creation");
		return false;	
	}

	character_SelectBuildLightweight(e, pcharOrig->iCurBuild);
	/* Copy over the base powersets from the original character *************/

	for (i = 0; (!bought_primary || !bought_secondary) && i < eaSize(&(pcharOrig->ppPowerSets)); i++)
	{
		// The first primary and secondary on a character will always be
		// their original primary and secondary. Specializations and other
		// such sets will get added later. So, we look for the first ones in
		// each category and add them to the new character to keep that
		// relationship and stop them from disappearing if the respec happens
		// not to pick any powers from the original powersets.
		if (!bought_primary && pcharOrig->ppPowerSets[i]->psetBase->pcatParent == pcharOrig->pclass->pcat[kCategory_Primary])
		{
			character_BuyPowerSet(e->pchar, pcharOrig->ppPowerSets[i]->psetBase);
			bought_primary = true;
		}
		else if (!bought_secondary && pcharOrig->ppPowerSets[i]->psetBase->pcatParent == pcharOrig->pclass->pcat[kCategory_Secondary])
		{
			character_BuyPowerSet(e->pchar, pcharOrig->ppPowerSets[i]->psetBase);
			bought_secondary = true;
		}
	}

	/* Receive Patron *********************************************************/
	bPatron = pktGetBits(pak, 1);
	if (bPatron)
	{
		iPatronID = pktGetBitsPack(pak, 4);
		if (iPatronID < 0 || iPatronID >= PATRON_COUNT)
		{
			LOG_ENT( e, LOG_ENTITY, LOG_LEVEL_DEBUG, 0, "Respec Bad patron sent to server (%d)", iPatronID);
			//	If this fails, we are going to bail out entirely
			//	Ignore the rest of the packet and just let it drop, since we don't know what type of character to make
			//	Since this character would not pass validation anyways
			//	There really isn't any point in going further
			return false;
		}
	} else {
		iPatronID = -1;
	}

	/* Receive Powers *********************************************************/


	iPowerCount = pktGetBitsPack( pak, 32 );

	assert(CountForLevel(iExpLevel, g_Schedules.aSchedules.piPower) >= 2);	// this should always be true (unless very low level character respecs, which is prevented on the client)

	if(iPowerCount != CountForLevel(iExpLevel, g_Schedules.aSchedules.piPower))
	{
		LOG_ENT( e, LOG_ENTITY, LOG_LEVEL_DEBUG, 0, "Respec Incorrect number of powers sent to server (was %d, should be %d)", iPowerCount, CountForLevel(iExpLevel, g_Schedules.aSchedules.piPower));
		//	If this fails, we are going to bail out entirely
		//	Ignore the rest of the packet and just let it drop, since we don't know what type of character to make
		//	Since this character would not pass validation anyways
		//	There really isn't any point in going further
		return false;
	}

	for( i = 0; i < iPowerCount; i++ )
	{
		const BasePower * p = basepower_Receive( pak, &g_PowerDictionary, e );
		if(p)
		{
			eaPushConst( pppPowers, p );
			LOG_ENT( e, LOG_ENTITY, LOG_LEVEL_DEBUG, 0, "Respec Received power %s from client\n", p->pchName);
		}
		else
		{
			LOG_ENT( e, LOG_ENTITY, LOG_LEVEL_DEBUG, 0, "Respec Bad power passed to server (index %d)", i);
			//	If this fails, we are going to bail out entirely
			//	Ignore the rest of the packet and just let it drop, since we don't know what type of character to make
			//	Since this character would not pass validation anyways
			//	There really isn't any point in going further
			return false;
		}
	}

	/* Receive Boosts ***************************************************/

	// boost slots
	(*piSlotCount) = pktGetBitsPack( pak, 32 );
	if((*piSlotCount) != CountForLevel(iExpLevel, g_Schedules.aSchedules.piAssignableBoost))
	{
		LOG_ENT( e, LOG_ENTITY, LOG_LEVEL_DEBUG, 0, "Respec Incorrect number of slots sent to server (was %d, should be %d)", (*piSlotCount), CountForLevel(iExpLevel, g_Schedules.aSchedules.piAssignableBoost));
		//	If this fails, we are going to bail out entirely
		//	Ignore the rest of the packet and just let it drop, since we don't know what type of character to make
		//	Since this character would not pass validation anyways
		//	There really isn't any point in going further
		return false;
	}

	for( i = 0; i < (*piSlotCount); i++ )
		aBoostSlots[i] = basepower_Receive(pak, &g_PowerDictionary, e);

	// power boosts
	iPowerBoosts = pktGetBitsPack( pak, 32 );
	if(    iPowerBoosts < 0
		|| iPowerBoosts > MAX_BOOST_SLOTS)
	{
		LOG_ENT( e, LOG_ENTITY, LOG_LEVEL_DEBUG, 0, "Respec Bad number of power boosts sent to server (%d)", iPowerBoosts);
		//	If this fails, we are going to bail out entirely
		//	Ignore the rest of the packet and just let it drop, since we don't know what type of character to make
		//	Since this character would not pass validation anyways
		//	There really isn't any point in going further
		return false;
	}

	for( i = 0; i < iPowerBoosts; i++ )
	{
		RespecBoost *prb = calloc( 1, sizeof(RespecBoost) );
		prb->pToPow = basepower_Receive( pak, &g_PowerDictionary, e );
		prb->iToIdx = pktGetBitsPack( pak, 32 );
		prb->ispec = pktGetBitsPack( pak, 32 );
		prb->bFromInventory = pktGetBitsPack( pak, 1 );

		if( !prb->bFromInventory )
		{
			prb->ipow  = pktGetBitsPack( pak, 32 );
			prb->iset  = pktGetBitsPack( pak, 32 );
		}
		eaPush( pppRespecBoosts, prb );
	}


	// inventory boosts
	for( i = 0; i < CHAR_BOOST_MAX; i++ )
	{
		if( pktGetBitsPack( pak, 1 ) )
		{
			RespecBoost *prb = calloc( 1, sizeof(RespecBoost) );

			prb->iToIdx = i;
			prb->bToInventory = true;
			prb->ispec = pktGetBitsPack( pak, 32 );
			prb->bFromInventory = pktGetBitsPack( pak, 1 );

			if( !prb->bFromInventory )
			{
				prb->ipow = pktGetBitsPack( pak, 32 );
				prb->iset = pktGetBitsPack( pak, 32 );
			}
			eaPush( pppRespecBoosts, prb );
		}
	}

	// pile boosts (will be converted to influence)
	iPileBoosts = pktGetBitsPack( pak, 32 );
	if(    iPileBoosts < 0
		|| iPileBoosts > MAX_BOOST_SLOTS)
	{
		LOG_ENT( e, LOG_ENTITY, LOG_LEVEL_DEBUG, 0, "Respec Bad number of pile boosts sent to server (%d)", iPileBoosts);
		//	If this fails, we are going to bail out entirely
		//	Ignore the rest of the packet and just let it drop, since we don't know what type of character to make
		//	Since this character would not pass validation anyways
		//	There really isn't any point in going further
		return false;
	}

	for( i = 0; i < iPileBoosts; i++ )
	{
		RespecBoost *prb = calloc( 1, sizeof(RespecBoost) );

		prb->bToPile = true;
		prb->ispec = pktGetBitsPack( pak, 32 );
		prb->bFromInventory = pktGetBitsPack( pak, 1 );

		if( !prb->bFromInventory )
		{
			prb->ipow = pktGetBitsPack( pak, 32 );
			prb->iset = pktGetBitsPack( pak, 32 );
		}
		eaPush( pppRespecBoosts, prb );
	}

	return true;
}

static bool respec_validateInfo(Entity *e, int iExpLevel, Character * tchar, Character * pCharOld, BasePower*** pppPowers, RespecBoost ***pppRespecBoosts, RespecBoost ***pppCurrentBoosts, BasePower* aBoostSlots[MAX_BOOST_SLOTS], int iSlotCount, int * piPileValue)
{
	int i,k;
	boolean bRet = TRUE;

	/* Are they allowed to respec even? Level-up respecs don't use a token *************/
	/* or similar, but must pass level validation elsewhere ****************************/
	if( ( iExpLevel <= pCharOld->iLevel ) && !playerCanRespec(e) )
	{
		LOG_ENT( e, LOG_ENTITY, LOG_LEVEL_DEBUG, 0, "Respec Player is not allowed to respec");
		return false;
	}

	/* Origin/Class Validation ********************************************************/

	if(stricmp(pCharOld->porigin->pchName, tchar->porigin->pchName))
	{
		LOG_ENT( e, LOG_ENTITY, LOG_LEVEL_DEBUG, 0, "Respec New origin (%s) does not match original (%s)", tchar->porigin->pchName, e->pchar->porigin->pchName);
		return false;
	}

	if(stricmp(pCharOld->pclass->pchName, tchar->pclass->pchName))
	{
		LOG_ENT( e, LOG_ENTITY, LOG_LEVEL_DEBUG, 0, "Respec New archetype does not match original");
		return false;
	}

	/* Powerset Validation ************************************************************/

	if(pCharOld->pclass->pcat[kCategory_Primary] != tchar->pclass->pcat[kCategory_Primary])
	{
		LOG_ENT( e, LOG_ENTITY, LOG_LEVEL_DEBUG, 0, "Respec New primary powerset (%s) does not match original (%s)", tchar->pclass->pcat[kCategory_Primary]->pchName, e->pchar->pclass->pcat[kCategory_Primary]->pchName);
		return false;
	}

	if(pCharOld->pclass->pcat[kCategory_Secondary] != tchar->pclass->pcat[kCategory_Secondary])
	{
		LOG_ENT( e, LOG_ENTITY, LOG_LEVEL_DEBUG, 0, "Respec New secondary powerset (%s) does not match original (%s)", tchar->pclass->pcat[kCategory_Secondary]->pchName, e->pchar->pclass->pcat[kCategory_Secondary]->pchName);
		return false;
	}

	/* Log old list of powers *********************************************/

	for( i = 0; i < eaSize( &pCharOld->ppPowerSets ); i++ )
	{
		PowerSet * pset = pCharOld->ppPowerSets[i];
		if(pset)
		{
			for( k = 0; k < eaSize( &pset->ppPowers ); k++)
			{
				if(pset->ppPowers[k])
				{
					LOG_ENT( e, LOG_ENTITY, LOG_LEVEL_DEBUG, 0, "Respec Prev Power: %s %s",
							pset->psetBase->pchName,
							pset->ppPowers[k]->ppowBase->pchName);
				}
			}
		}
	}

	/* Power Validation ***************************************************/

	bRet = TRUE;

	if(!respec_VerifyPowers(e, tchar, pppPowers, iExpLevel, pCharOld))
	{
		LOG_ENT( e, LOG_ENTITY, LOG_LEVEL_DEBUG, 0, "Respec Error validating powers");
		bRet = FALSE;
	}


	/* Boost Validation ***************************************************/

	if(bRet && !respec_VerifyBoosts(e, tchar, pCharOld, pppCurrentBoosts, pppRespecBoosts, iExpLevel, aBoostSlots, iSlotCount, piPileValue))
	{
		LOG_ENT( e, LOG_ENTITY, LOG_LEVEL_DEBUG, 0, "Respec Error validating boosts");
		bRet = FALSE;
	}

	// done!
	return bRet;
}

/**********************************************************************func*
 * character_ReceiveRespec
 *
 */
bool character_ReceiveRespec( Packet * pak, Entity * e )
{
	Character *pcharNew = character_Allocate();
	Character *pcharOrig = e->pchar;

	BasePower **ppPowers = 0;
	RespecBoost **ppRespecBoosts = 0;
	RespecBoost **ppCurrentBoosts = 0;

	BasePower* aBoostSlots[MAX_BOOST_SLOTS] = {0};

	int iSlotCount;
	int pileValue = 0;
	int level = 0;
	bool bLevelUp = false;

	int oldPatronID = respecFindPatron(e);
	int newPatronID = -1;

	bool bSuccess = false;

	// Make sure we process onDeactivate and onDisable mods before processing
	//   new onEnable and onActivate mods!
	character_CancelPowers(pcharOrig);

	eaCreate( &ppCurrentBoosts );
	eaCreate( &ppRespecBoosts );
	eaCreate( &ppPowers );

	// set flags for full update when we're done, regardless of success
	e->level_update = TRUE; // resend character always
	eaPush(&e->powerDefChange, (void *)-1);

	// Swap in the new empty pchar for loading.
	e->pchar = pcharNew;
	e->pchar->iNumBoostSlots = pcharOrig->iNumBoostSlots;
	e->pchar->iLevel = pcharOrig->iLevel;
	e->pchar->iCurBuild = pcharOrig->iCurBuild;
	e->pchar->iActiveBuild = pcharOrig->iActiveBuild;
	memcpy(e->pchar->iBuildLevels, pcharOrig->iBuildLevels, sizeof(pcharOrig->iBuildLevels));
	e->pchar->playerTypeByLocation = pcharOrig->playerTypeByLocation;

	level = pcharOrig->iLevel;

	// If this respec is part of a level-up we'll be receiving data for one
	// level ahead of the character's current level.
	if(character_IsLeveling(pcharOrig))
	{
		bLevelUp = true;
		level++;
	}

	// get respec info from client & validate it

	LOG_ENT( e, LOG_ENTITY, LOG_LEVEL_DEBUG, 0, "Respec *********\nBegan verifying respec for player player %s (authname %s)\n", e->name, e->auth_name);

	if(respec_receiveInfo(pak, e, level, pcharOrig, &ppPowers, &ppRespecBoosts, &ppCurrentBoosts, aBoostSlots, &iSlotCount, &newPatronID))
	{
		if(respec_validateInfo(e, level, pcharNew, pcharOrig, &ppPowers, &ppRespecBoosts, &ppCurrentBoosts, aBoostSlots, iSlotCount, &pileValue))
		{
			if(bLevelUp)
			{
				if(character_CanLevel(pcharOrig))
				{
					bSuccess = true;
				}
				else
				{
					LOG_ENT( e, LOG_ENTITY, LOG_LEVEL_VERBOSE, 0, "RespecLevelUp:Error Attempted to level, but can't.");
				}
			}
			else
			{
				// success !!!
				bSuccess = true;
			}
		}
	}
	else
	{
		bSuccess = false;
	}

	// Copy these again, because they get damaged in respec_receiveInfo();
	e->pchar->iCurBuild = pcharOrig->iCurBuild;
	e->pchar->iActiveBuild = pcharOrig->iActiveBuild;
	memcpy(e->pchar->iBuildLevels, pcharOrig->iBuildLevels, sizeof(pcharOrig->iBuildLevels));
	e->pchar->playerTypeByLocation = pcharOrig->playerTypeByLocation;

	if( bSuccess )
	{
		/* great! everything worked, now switch pchars ****************************************************************/

		character_ClearBoostsetAutoPowers(pcharOrig);
		costumeUnawardPowersetParts(e, 0);

		// old character will be destroyed & replaced by new one
		character_RespecSwap(e, pcharOrig, pcharNew, pileValue); // This function must be handed the real Entity.

		character_UncancelPowers(e->pchar);

		// reactivate invention set powers correctly.
		character_ResetBoostsets(e->pchar);

		if( bLevelUp )
		{
			character_LevelUp(e->pchar);
			character_LevelFinalize(e->pchar, true);

			svrSendEntListToDb(&e, 1);

			LOG_ENT( e, LOG_ENTITY, LOG_LEVEL_VERBOSE, 0, "RespecLevelUp Success");
		}
		else
		{
			// Only use a token if this wasn't a freebie respec as part of
			// the level-up.
			playerUseRespecToken(e);

			LOG_ENT( e, LOG_ENTITY, LOG_LEVEL_VERBOSE, 0, "Respec Success");
		}

		costumeAwardPowersetParts(e, 1, 0);
		costumeFillPowersetDefaults(e);

		e->pchar->ppBuildPowerSets[e->pchar->iCurBuild] = e->pchar->ppPowerSets;

		LOG_ENT( e, LOG_ENTITY, LOG_LEVEL_VERBOSE, 0, "Respec succeeded (authname %s)\n\n\n", e->auth_name);

		character_Destroy(pcharOrig,NULL);
		character_Free(pcharOrig);
	}
	else
	{
		// Undo swap
		e->pchar = pcharOrig;
		character_UncancelPowers(e->pchar);

		LOG_ENT( e, LOG_ENTITY, LOG_LEVEL_VERBOSE, 0, "Respec Respec FAILED for player %s (authname %s)\n\n\n", e->name, e->auth_name);
		character_Destroy(pcharNew,NULL);
		character_Free(pcharNew);
	}

	// clean up

	eaDestroyEx(&ppRespecBoosts, 0);
	eaDestroyEx(&ppCurrentBoosts, 0);
	eaDestroy( &ppPowers );

	// unset respeccing and levelling flags (in case this is a level-up
	// respec)
	e->pl->isRespeccing = 0;
	e->pl->levelling = 0;

	return bSuccess;
}

/* End of File */
