 //
// tray.c 1/2/03
//
// Handles powers tray info
//
//----------------------------------------------------------------

#include "trayServer.h"
#include "varutils.h"
#include "netio.h"
#include "trayCommon.h"
#include "entity.h"
#include "entPlayer.h"
#include "powers.h"
#include "character_base.h"
#include "earray.h"
#include "strings_opt.h"

//-------------------------------------------------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//-------------------------------------------------------------------------------------------------------------------
//
//
static void tray_setIndexFromString( Entity *e, TrayObj * to, int build )
{
	int i, j;

	// Get the power set for the specified build
	PowerSet ** ppPowerSets = e->pchar->ppBuildPowerSets[build];

	// If this TrayObj has iset and ipow already set up right, don't bother trying to fix them
	//  Otherwise when you have multiple copies of one temporary power every tray icon will map to the
	//  same instance of the power
	if (to->pchPowerSetName
		&& to->pchPowerName
		&& to->iset >= 0
		&& to->ipow >= 0
		&& to->iset < eaSize( &ppPowerSets )
		&& to->ipow < eaSize( &ppPowerSets[to->iset]->ppPowers )
		&& ppPowerSets[to->iset]->ppPowers[to->ipow]
		&& stricmp( to->pchPowerSetName, ppPowerSets[to->iset]->psetBase->pchName ) == 0
		&& stricmp( to->pchPowerName, ppPowerSets[to->iset]->ppPowers[to->ipow]->ppowBase->pchName ) == 0)
	{
		return;
	}


	for( i = eaSize( &ppPowerSets )-1; i >= 0; i-- )
	{
		if ( to->pchPowerSetName && ppPowerSets[i] && 
			ppPowerSets[i]->psetBase && 
			stricmp( to->pchPowerSetName, ppPowerSets[i]->psetBase->pchName ) == 0 )
		{
			to->iset = i;

			for( j = eaSize( &ppPowerSets[i]->ppPowers )-1; j >= 0; j-- )
			{
				if ( to->pchPowerName && ppPowerSets[i]->ppPowers[j] &&
					ppPowerSets[i]->ppPowers[j]->ppowBase && 
					stricmp( to->pchPowerName, ppPowerSets[i]->ppPowers[j]->ppowBase->pchName ) == 0 )
				{
					to->ipow = j;
					return;
				}
			}
		}
	}

	to->type = kTrayItemType_None;
}


void tray_unpack( Entity *e )
{
	int build;
	int maxBuilds;
	TrayObj *to;

	if (!verify(e && e->pl && e->pl->tray && e->pchar))
		return;

	maxBuilds = character_MaxBuildsAvailable(e);

	for (build = 0; build < maxBuilds; build++)
	{
		resetTrayIterator(e->pl->tray, build, kTrayCategory_PlayerControlled);
		while (getNextTrayObjFromIterator(&to))
		{	
			if (!to)
				continue;

			if (to->type == kTrayItemType_Power)
			{
				tray_setIndexFromString(e, to, build);
			}
			if (to->type == kTrayItemType_Inspiration)
			{
				if (to->iset < 0 || to->ipow < 0
					|| to->iset >= e->pchar->iNumInspirationCols
					|| to->ipow >= e->pchar->iNumInspirationRows
					|| e->pchar->aInspirations[to->iset][to->ipow] == NULL)
				{
					destroyCurrentTrayObjViaIterator();
				}
			}
		}
	}

	trimExcessTrayObjs(e->pl->tray, maxBuilds);
}
