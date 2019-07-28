/***************************************************************************
 *     Copyright (c) 2000-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#include <assert.h>

#include "netio.h"
#include "netcomp.h"
#include "entity.h"
#include "earray.h"
#include "comm_game.h"
#include "svr_base.h"

#include "powers.h"
#include "character_base.h"
#include "character_mods.h"
#include "character_combat.h"
#include "character_animfx.h"
#include "character_net.h"
#include "character_target.h"

#include "gridcoll.h"

#include "entity_power.h"
#include "mathutil.h"

/**********************************************************************func*
 * entity_ActivatePowerReceive
 *
 * Returns false if there was a data validation problem, true othwerwise.
 *
 * Send function in Game\entity\entity_power_client.c
 *    (entity_ActivatePowerSend)
 *
 */
bool entity_ActivatePowerReceive(Entity *e, Packet *pak)
{
	int ibuild;
	int iset;
	int ipow;
	int iIdxTarget;
	int iIDTarget;
	bool bRet = false;

	assert(e!=NULL);
	assert(pak!=NULL);

	// CLIENTINP_ACTIVATE_POWER
	// powerset idx
	// power idx
	// target idx
	// target id (for verification)

	ibuild = pktGetBitsAuto(pak);
	iset = pktGetBitsPack(pak, 4);
	ipow = pktGetBitsPack(pak, 4);
	iIdxTarget = pktGetBitsPack(pak, 16);
	iIDTarget = pktGetBitsPack(pak, 32);

	if(e->pchar!=NULL)
	{
		if(iIdxTarget<0 || iIdxTarget>=entities_max)
		{
			// Invalid request. Naughty!
			// Ignore this request.
		}
		else
		{
			Power *ppow = SafeGetPowerByIdx(e->pchar, ibuild, iset, ipow, 0);

			if(ppow!=NULL)
			{
				Character *pcharTarget = NULL;

				if(iIdxTarget!=0)
				{
					if(entities[iIdxTarget]->id != iIDTarget)
					{
						// Bad target
						return false;
					}
					pcharTarget = entities[iIdxTarget]->pchar;
				}
				else
				{
					// This power was activated with the assumption that the
					// entity is assisting someone.
				}

				character_ActivatePowerOnCharacter(e->pchar, pcharTarget, ppow);
				bRet = true;
			}
			else
			{
				// The index and the id don't match. The entity may have died before
				// the power could be applied.
				// Ignore this request.
			}
		}
	}

	return bRet;
}

/**********************************************************************func*
 * entity_ActivatePowerAtLocationReceive
 *
 * Returns false if there was a data validation problem, true othwerwise.
 *
 * Send function in Game\entity\entity_power_client.c
 *    (entity_ActivatePowerAtLocationSend)
 *
 */
bool entity_ActivatePowerAtLocationReceive(Entity *e, Packet *pak)
{
	int ibuild;
	int iset;
	int ipow;
	int iIdxTarget;
	int iIDTarget;
	float fPlayerHeight;
	Vec3 vec, camera, playerPos, tempDest;

	bool bRet = false;

	assert(e!=NULL);
	assert(pak!=NULL);

	ibuild = pktGetBitsAuto(pak);
	iset = pktGetBitsPack(pak, 4);
	ipow = pktGetBitsPack(pak, 4);
	iIdxTarget = pktGetBitsPack(pak, 16);
	iIDTarget = pktGetBitsPack(pak, 32);
	fPlayerHeight = pktGetF32(pak);

	pktGetVec3(pak, vec);
	pktGetVec3(pak, camera);

	if( fPlayerHeight > 12.0f || fPlayerHeight < 1.f ) // make sure player height is within tolerances
		return false;


	if(e->pchar!=NULL)
	{
		if(iIdxTarget<=0 || iIdxTarget>=entities_max)
		{
			// Invalid request. Naughty!
			// Ignore this request.
		}
		else
		{
			Power *ppow = SafeGetPowerByIdx(e->pchar, ibuild, iset, ipow, 0);

 			if(ppow!=NULL)
			{
				Entity *ptarget = NULL;

				if(iIdxTarget!=0)
				{
					ptarget = entities[iIdxTarget];
					if(ptarget->id != iIDTarget)
					{
						// Invalid target
						return false;
					}
				}
				else
				{
					// This power was activated with the assumption that the
					// entity is assisting someone.
				}

				if (ppow->ppowBase->eTargetType == kTargetType_Teleport ||
						ppow->ppowBase->eTargetTypeSecondary == kTargetType_Teleport)
				{
					copyVec3(ENTPOS(e), playerPos);
					copyVec3(vec, tempDest);

					if (!IsTeleportTargetValid(tempDest) || !ExtraTeleportChecks(tempDest, playerPos, camera))
					{
						// If someone triggers this a lot, they're probably cheating.
						// This duplicates a test done clientside, therefore for a legitimate player the
						// command will be culled clinetside.
						return false;
					}
				}

				character_ActivatePowerAtLocation(e->pchar, ptarget->pchar, vec, ppow);
				bRet = true;
			}
			else
			{
				// The index and the id don't match. The entity may have died before
				// the power could be applied.
				// Ignore this request.
			}
		}
	}

	return bRet;
}

/**********************************************************************func*
 * entity_SetDefaultPowerReceive
 *
 * Send function in Game\entity\entity_power_client.c
 *    (entity_SetDefaultPowerSend)
 *
 */
bool entity_SetDefaultPowerReceive(Entity *e, Packet *pak)
{
	int ibuild;
	int iset;
	int ipow;
	bool bRet = false;

	assert(e!=NULL);
	assert(pak!=NULL);

	// CLIENTINP_SET_DEFAULT_POWER
	// powerset idx
	// power idx

	ibuild = pktGetBitsAuto(pak);
	iset = pktGetBitsPack(pak, 4);
	ipow = pktGetBitsPack(pak, 4);

	if(e->pchar!=NULL)
	{
		Power *ppow = SafeGetPowerByIdx(e->pchar, ibuild, iset, ipow, 0);

		if(ppow!=NULL && ppow->ppowBase->eType==kPowerType_Click)
		{
			e->pchar->ppowDefault = ppow;
			bRet = true;
		}
	}

	return bRet;
}


/**********************************************************************func*
 * entity_ActivateInspirationReceive
 *
 * Send function in Game\entity\entity_power_client.c
 *    (entity_ActivateInspirationSend)
 *
 */
bool entity_ActivateInspirationReceive(Entity *e, Packet *pak)
{
	int iCol, iRow;
	bool bRet = false;

	assert(e!=NULL);
	assert(pak!=NULL);

	// CLIENTINP_ACTIVATE_INSPIRATION
	// inspriation col
	// inspriation row

	iCol = pktGetBitsPack(pak, 3);
	iRow = pktGetBitsPack(pak, 3);

	if(e->pchar!=NULL)
	{

		if(iCol<e->pchar->iNumInspirationCols && iRow<e->pchar->iNumInspirationRows)
		{
			character_ActivateInspiration(e->pchar, e->pchar, iCol, iRow);

			bRet = true;
		}
	}

	return bRet;
}

/**********************************************************************func*
* entity_ActivateInspirationReceive
*
* Send function in Game\entity\entity_power_client.c
*    (entity_ActivateInspirationSend)
*
*/
bool entity_MoveInspirationReceive(Entity *e, Packet *pak)
{
	int iSrcCol, iSrcRow, iDestCol, iDestRow;
	bool bRet = false;

	assert(e!=NULL);
	assert(pak!=NULL);

	iSrcCol  = pktGetBitsPack(pak, 3);
	iSrcRow  = pktGetBitsPack(pak, 3);
	iDestCol = pktGetBitsPack(pak, 3);
	iDestRow = pktGetBitsPack(pak, 3);

	if(e->pchar!=NULL && (iSrcCol != iDestCol || iSrcRow != iDestRow))
	{
		bRet = character_MoveInspiration( e->pchar, iSrcCol, iSrcRow, iDestCol, iDestRow );
		character_CompactInspirations(e->pchar);
	}

	return bRet;
}


/**********************************************************************func*
 * entity_StanceReceive
 *
 * Returns false if there was a data validation problem, true othwerwise.
 *
 * Send function in Game\entity\entity_power_client.c
 *    (entity_StanceSend)
 *
 */
bool entity_StanceReceive(Entity *e, Packet *pak)
{
	bool bEnterStance;
	int ibuild;
	int iset;
	int ipow;
	Power *ppow = NULL;

	assert(e!=NULL);
	assert(pak!=NULL);

	bEnterStance = pktGetBits(pak, 1);
	if(bEnterStance)
	{
		ibuild = pktGetBitsAuto(pak);
		iset = pktGetBitsPack(pak, 4);
		ipow = pktGetBitsPack(pak, 4);

		if((ppow = SafeGetPowerByIdx(e->pchar, ibuild, iset, ipow, 0))!=NULL)
		{
			character_EnterStance(e->pchar, ppow, true);
			return true;
		}
	}
	else
	{
		if(e->pchar->ppowCurrent==e->pchar->ppowStance
			|| e->pchar->ppowQueued==e->pchar->ppowStance)
		{
			// They tried to cancel a stance that they should stay in.
			//    Remind the client of the stance.
			entity_StanceSend(e, e->pchar->ppowStance);
		}
		else
		{
			character_EnterStance(e->pchar, NULL, true);
		}
		return true;
	}

	return false;
}

/**********************************************************************func*
 * entity_StanceSend
 *
 */
void entity_StanceSend(Entity *e, Power *ppow)
{
	if (ENTTYPE(e)==ENTTYPE_PLAYER)
	{
		START_PACKET(pak, e, SERVER_SET_STANCE);
		if(ppow!=NULL && e->pchar)
		{
			pktSendBits(pak, 1, 1);
			pktSendBitsAuto(pak, e->pchar->iCurBuild);
			pktSendBitsPack(pak, 4, ppow->psetParent->idx);
			pktSendBitsPack(pak, 4, ppow->idx);
		}
		else
		{
			pktSendBits(pak, 1, 0);
		}
		END_PACKET
	}
}

/* End of File */
