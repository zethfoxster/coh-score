#include "power_customization.h"
#include "power_customization_server.h"
#include "StashTable.h"
#include "entity.h"
#include "Reward.h"
#include "entPlayer.h"
#include "entGameActions.h"

extern StashTable powerCustReward_HashTable;
PowerCustomizationList * powerCustList_recieveTailor(Packet *pak, Entity *e)
{
	PowerCustomizationList *powerCustList = StructAllocRaw(sizeof(PowerCustomizationList));
	powerCustList_receive(pak, e, powerCustList);
	if (powerCustList_validate( powerCustList, e, 1))
	{
		return powerCustList;
	}
	else
	{
		powerCustList_destroy(powerCustList);
		return NULL;
	}
}
void powerCustList_applyTailorChanges(Entity *e, PowerCustomizationList *powerCustList)
{
	powerCustList_destroy(e->pl->powerCustomizationLists[e->pl->current_powerCust]);
	e->powerCust = e->pl->powerCustomizationLists[e->pl->current_powerCust] = powerCustList;
	e->updated_powerCust = 1;
	e->send_all_powerCust = 1;
}

int powerCust_isReward(char *powerCustTheme)
{
	if( !powerCustTheme || !powerCustReward_HashTable )
		return 0;

	return stashFindElement(powerCustReward_HashTable, powerCustTheme, NULL);
}

void powerCust_Award( Entity *e, char * powerCustTheme, int notifyPlayer)
{
	if( e && e->pl && powerCustTheme && powerCust_isReward(powerCustTheme) )
	{
		if( rewardtoken_IdxFromName( &e->pl->rewardTokens, powerCustTheme ) == -1 )
		{
			rewardtoken_Award( &e->pl->rewardTokens, powerCustTheme, 0 );
			e->updated_powerCust = true;
			if (notifyPlayer)
			{
				//	So far, the only reason not to notify the player is for critter powerset costume pieces
				//	They shouldn't be told that they got some costume piece they don't really have. -EL
				sendInfoBox(e, INFO_REWARD, "PowerCustThemeYouReceived", powerCustTheme);
				serveFloater(e, e, "FloatCostume", 0.0f, kFloaterStyle_Attention, 0);
			}
		}
	}
}

void powerCust_Unaward( Entity *e, char * powerCustTheme)
{
	if( e && powerCustTheme )
		rewardtoken_Unaward(&e->pl->rewardTokens, powerCustTheme);
}